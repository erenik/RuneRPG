/// Evergreen IT AB
/// 2024-03-07

#include "MapIntegrator.h"

#include "Physics/PhysicsManager.h"

// All entities sent here should be fully dynamic!
void MapIntegrator::IntegrateDynamicEntities(List< Entity* >& dynamicEntities, float timeInSeconds) {
	Timer timer;
	timer.Start();
	List< Entity* >& forceBasedEntities = PhysicsMan.forceBasedEntities;
	Timer timer2;
	timer2.Start();
	if (forceBasedEntities.Size())
	{
		/// Provides default "scientific" rigid-body based simulation handling of forces, torques, etc.
		CalculateForces(forceBasedEntities);
		UpdateMomentum(forceBasedEntities, timeInSeconds);
		DeriveVelocity(forceBasedEntities);
	}
	timer2.Stop();
	int forcesMomenumVelocity = timer2.GetMs();

	timer2.Start();
	IntegrateVelocity(dynamicEntities, timeInSeconds);
	timer2.Stop();
	int velocityIntegration = timer2.GetMs();
	timer2.Start();
	IntegratePosition(dynamicEntities, timeInSeconds);
	timer2.Stop();
	int positionIntegration = timer2.GetMs();
	timer.Stop();
	this->integrationTimeMs = timer.GetMs();
}
// All entities sent here should be fully kinematic! 
void MapIntegrator::IntegrateKinematicEntities(List< Entity* >& kinematicEntities, float timeInSeconds) {
	Timer timer;
	timer.Start();
	IntegratePosition(kinematicEntities, timeInSeconds);
	integrationTimeMs += timer.GetMs();

}


void MapIntegrator::IntegratePosition(List< Entity* >& forEntities, float timeInSeconds) {

#ifdef USE_SSE
	SSEVec timeSSE;
	timeSSE.data = _mm_load1_ps(&timeInSeconds);
	timeSSE.v[3] = 0;
#endif
	for (int i = 0; i < forEntities.Size(); ++i)
	{
		Entity* forEntity = forEntities[i];
		PhysicsProperty* pp = forEntity->physics;
		Vector3f& position = forEntity->localPosition;
#ifdef USE_SSE
		position.data = _mm_add_ps(position.data, _mm_mul_ps(pp->currentVelocity.data, timeSSE.data));
#else
		/// First position. Simple enough.
		position += pp->currentVelocity * timeInSeconds;
#endif

		/// Rotation below
		bool rotated = false;
		// Force rot to follow vel.
		if (pp->faceVelocityDirection && (pp->currentVelocity.MaxPart()))
		{
			// From default of 0,0,-1
			Vector3f defaultDir(0, 0, -1);
			Vector3f vec1 = pp->currentVelocity;
			vec1.Normalize();
			Angle a(defaultDir.x, defaultDir.z), b(vec1.x, vec1.z);
			Angle to = b - a;
			if (to.IsGood())
			{
				float angle = to.Radians();
				Quaternion q(Vector3f(0, 1, 0), angle);
				q.Normalize();
				assert(q.x == q.x);
				forEntity->orientation = q;
				forEntity->hasRotated = true;
				//		forEntity->rotationMatrix = q.Matrix();
			}
		}
		else if (pp->angularVelocity.MaxPart())
		{
			/// WOIRK HGER MOARE LATARR@!
			Quaternion& orientation = forEntity->orientation;
			Quaternion rotate(pp->angularVelocity[0], pp->angularVelocity[1], pp->angularVelocity[2]);
			Vector3f axis = pp->angularVelocity.NormalizedCopy();
			Quaternion r2(axis, pp->angularVelocity.Length());
			orientation = orientation * r2;
			orientation.Normalize();
			forEntity->hasRotated = true;
		}
		else if (pp->relativeRotationalVelocity.MaxPart())
		{
			// o.o
			Vector3f up = forEntity->rotationMatrix.GetColumn(1);
			Vector3f forward = -forEntity->rotationMatrix.GetColumn(2);
			Vector3f right = forEntity->rotationMatrix.GetColumn(0);

			Vector3f rotation = pp->relativeRotationalVelocity * timeInSeconds;
			rotation.x *= -1;
			rotation.z *= -1;

			// Alright. So now we have rotations along the 3 main local axises...
			// Now we need to find the 3 quaternions which would apply these rotations (looking from a global perspective)
			Quaternion pitch(right, rotation[0]);
			Quaternion yaw(up, rotation[1]);
			Quaternion roll(forward, rotation[2]);

			// Now multiply these three with the current orientation quaternion one at a time... should be simple enouhg.. maybe :P
			Quaternion& orientation = forEntity->orientation;
			//
			orientation = orientation * pitch;
			orientation = orientation * yaw;
			orientation = orientation * roll;
			forEntity->hasRotated = true;
			forEntity->RecalculateMatrix();
			// Recalculate velocity o.o'
			Vector3f newForward = -forEntity->rotationMatrix.GetColumn(2);
			float velDotForward = pp->velocity.DotProduct(forward);
			Vector3f localZPart = velDotForward * forward;
			float localZPartLen = localZPart.Length() * (velDotForward > 0 ? 1 : -1);
			Vector3f otherPart = pp->velocity - localZPart;
			Vector3f oldVel = pp->velocity;
			float factor = pow(pp->velocityRetainedWhileRotating, timeInSeconds);
			pp->velocity = localZPartLen * factor * newForward +
				(1 - factor) * 0.8f * localZPartLen * forward +
				otherPart;
			float dot = oldVel.NormalizedCopy().DotProduct(pp->velocity.NormalizedCopy());
		}
		if (forEntity->hasRotated)
			forEntity->RecalculateMatrix();
	}
}

void MapIntegrator::IntegrateVelocity(List< Entity* >& entities, float timeInSeconds) {
#ifdef USE_SSE
	SSEVec timeSSE;
	timeSSE.data = _mm_load1_ps(&timeInSeconds);
	float zero = 0.f;
	__m128 defaultSSE = _mm_load1_ps(&zero);
#endif

	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity* forEntity = entities[i];

		PhysicsProperty* pp = forEntity->physics;
		Vector3f& relativeVelocity = pp->relativeVelocity;

		// Add regular velocity (from physics and effects)
		Vector3f lookAt = forEntity->LookAt();

		/// First acceleration
		assert(pp->velocity.x == pp->velocity.x);
#ifdef USE_SSE
		__m128 sse = _mm_load1_ps(&pp->gravityMultiplier);
		SSEVec totalAcceleration;
		totalAcceleration.data = defaultSSE;
		/// Apply gravity.
		if (pp->gravityMultiplier && !(pp->state & CollisionState::IN_REST))
			totalAcceleration.data = _mm_add_ps(totalAcceleration.data, _mm_mul_ps(gravity.data, sse));
		/// Accelerate only if requirements met.
		if (!pp->requireGroundForLocalAcceleration || (physicsNowMs - pp->lastGroundCollisionMs) < pp->isOnGroundThresholdMs)
		{
			// Accelerate in the looking-direction
			// Require acceleration, but also that the entity be considered on the ground, if needed.
			if (pp->relativeAcceleration.x || pp->relativeAcceleration.y || pp->relativeAcceleration.z)
			{
				Vector3f relAcc = pp->relativeAcceleration;
				Vector3f worldAcceleration = forEntity->rotationMatrix.Product(relAcc);
				totalAcceleration.data = _mm_add_ps(totalAcceleration.data, worldAcceleration.data);
				assert(totalAcceleration.x == totalAcceleration.x);
			}
			// Regular acceleration.
			if (pp->acceleration.MaxPart())
				totalAcceleration.data = _mm_add_ps(totalAcceleration.data, pp->acceleration.data);
			assert(totalAcceleration.x == totalAcceleration.x);
		}
#else   /// Acceleration, non-SSE
		Vector3f totalAcceleration;
		if (pp->gravityMultiplier && !(pp->state & CollisionState::IN_REST))
			totalAcceleration += gravity * pp->gravityMultiplier;
		if (!pp->requireGroundForLocalAcceleration || (physicsNowMs - pp->lastGroundCollisionMs) < pp->isOnGroundThresholdMs)
		{
			if (pp->relativeAcceleration.x || pp->relativeAcceleration.y || pp->relativeAcceleration.z)
			{
				Vector3f relAcc = pp->relativeAcceleration;
				relAcc.z *= -1;
				Vector3f worldAcceleration = forEntity->rotationMatrix.Product(relAcc);
				totalAcceleration += worldAcceleration;
				assert(totalAcceleration.x == totalAcceleration.x);
			}
			// Regular acceleration.
			totalAcceleration += pp->acceleration;
		}
#endif

		/// Velocity
		float damp = pow(pp->linearDamping, timeInSeconds);
#ifdef USE_SSE
		pp->velocity.data = _mm_add_ps(pp->velocity.data, _mm_mul_ps(totalAcceleration.data, timeSSE.data));
		// Apply linear damping
		sse = _mm_load1_ps(&damp);
		pp->currentVelocity.data = pp->velocity.data = _mm_mul_ps(pp->velocity.data, sse);
		/// Player induced / controlled constant velocity in relative direction?
		Vector3f relVelWorldSpaced;
		if (relativeVelocity.x || relativeVelocity.y || relativeVelocity.z)
		{
			// Add it.
			Vector3f relVel;
			relVel.data = relativeVelocity.data;
			relVel.z *= -1;
			relVelWorldSpaced = forEntity->rotationMatrix * relVel;
		}
		/// Add it up.
		pp->currentVelocity.data = _mm_add_ps(pp->currentVelocity.data, relVelWorldSpaced.data);
		/// De-flag at rest if we got any acceleration or velocity?
		if (pp->currentVelocity.MaxPart())
			pp->state &= ~CollisionState::IN_REST;
#else   /// Velocity, non-SSE
		pp->velocity += totalAcceleration * timeInSeconds;
		pp->velocity *= damp;
		pp->currentVelocity = pp->velocity;
		Vector3f relVelWorldSpaced;
		if (relativeVelocity.x || relativeVelocity.y || relativeVelocity.z)
		{
			Vector3f relVel;
			relVel = relativeVelocity;
			relVel.z *= -1;
			relVelWorldSpaced = forEntity->rotationMatrix * relVel;
		}
		/// Add it up.
		pp->currentVelocity += pp->currentVelocity + relVelWorldSpaced;
		/// De-flag at rest if we got any acceleration or velocity?
		if (pp->currentVelocity.MaxPart())
			pp->state &= ~CollisionState::IN_REST;
#endif
		assert(pp->velocity.x == pp->velocity.x);


		/// Rotation

		// Rotation, non-SSE
		if (pp->angularAcceleration.MaxPart())
		{
			Vector3f accToApply = pp->angularAcceleration * timeInSeconds;

			// Add rotational velocity around local forward, up and right vectors.
			Vector3f rightVec = forEntity->RightVec();
			Vector3f upVec = forEntity->UpVec();
			Vector3f lookAt = forEntity->LookAt();
			Quaternion pitch(forEntity->RightVec(), accToApply.x);
			Quaternion yaw(forEntity->UpVec(), accToApply.y);
			Quaternion roll(forEntity->LookAt(), accToApply.z);

			/// Global ang velocity update.
			pp->angularVelocity += pp->angularAcceleration * timeInSeconds;
			/// Relative ang vel update.
		}
		if (pp->relativeAngularAcceleration.MaxPart())
		{
			// Relative angular acceleration to entity's forward (roll), right(pitch) and up (yaw) vectors.
			Vector3f accToApply = pp->relativeAngularAcceleration * timeInSeconds;

			// Add rotational velocity around local forward, up and right vectors.
			Vector3f rightVec = forEntity->RightVec();
			Vector3f upVec = forEntity->UpVec();
			Vector3f lookAt = forEntity->LookAt();
			Quaternion pitch(forEntity->RightVec(), accToApply.x);
			Quaternion yaw(forEntity->UpVec(), accToApply.y);
			Quaternion roll(forEntity->LookAt(), accToApply.z);

			/// Relative ang vel update.
			pp->angularVelocity += upVec * accToApply.y;
			pp->angularVelocity += rightVec * accToApply.x;
			pp->angularVelocity += lookAt * accToApply.z;

		}
		// Apply damping
		pp->angularVelocity *= pow(pp->angularDamping, timeInSeconds);
	}
}
