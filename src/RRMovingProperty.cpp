/// Emil Hedemalm
/// 2016-07-29
/// Property for moving! NPCs, Stones, anything :D 
/// Responds to GoToMessages sent by PathableProperty too.

#include "RRMovingProperty.h"
#include "StateManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Pathfinding/PathMessage.h"
#include "RuneEntity.h"

RRMovingProperty::RRMovingProperty(Entity * owner)
	: EntityProperty("RRMoving prop", ID(), owner)
{
	walkingAcceleration = 25.f;
	walkingDamping = 0.2f;
	idleDamping = 0.1f;
	state = IDLE;
	accumulator = 0;
	distArrivalThreshold = 0.5f;
	finalDestinationSet = false;
	paused = false;
}
RRMovingProperty::~RRMovingProperty(){}

extern RuneEntity * player;
void RRMovingProperty::ProcessMessage(Message * message)
{
	String msg = message->msg;
	if (msg == "PauseMovement")
	{
		paused = true;
		QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
		QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, idleDamping));
	}
	if (msg == "ResumeMovement")
	{
		paused = false;
		switch(state)
		{
			case WALKING_TO: WalkTo(destination);
			case WALKING: Walk(direction);
			case IDLE: Stop();
		}
	}
	if (message->msg == "OnFinalDestinationSet")
	{
		finalDestinationSet = true;
	}
	if (message->msg == "StopMoving")
		Stop();
	if (message->type == MessageType::MOVE_TO_MESSAGE)
	{
		MoveToMessage * mtm = (MoveToMessage*) message;
		if (player && owner == player->physicsEntity)
		{
			std::cout<<"\nNewDest: "<<mtm->pos<<" current pos: "<<owner->worldPosition;
		}

		WalkTo(mtm->pos);
		if (mtm->pos != lastDestination)
		{
			distanceMoved = 1.f;
		}
		lastDestination = mtm->pos;
		finalDestinationSet = false;
	}
}

void RRMovingProperty::Process(int timeInMs)
{
	if (state == IDLE)
		return;
	accumulator += timeInMs;
	if (accumulator < 200)
		return;
	accumulator = accumulator % 200;
	if (paused)
		return;
	// Stop when reaching position? Adjust if pushed?
	if (state == WALKING_TO)
	{
		float dist = (destination - owner->worldPosition).LengthSquared();
		if (dist < distArrivalThreshold)
		{
			Stop();
			if (finalDestinationSet)
			{
				Message msg("DestinationReached");
				owner->ProcessMessage(&msg);
			}
			return;
		}
		WalkTo(destination);		
	}
	float distMoved = (lastPosition - owner->worldPosition).Length();
	distanceMoved = distanceMoved * 0.5f + 0.5f * distMoved;
	lastPosition = owner->worldPosition;
	if (distanceMoved < 0.05f)
	{
		/// We are stuck, notify other properties.
		Message msg("RRMovingProperty::DistanceMovedZero");
		owner->ProcessMessage(&msg);
	}
}

void RRMovingProperty::Walk(ConstVec3fr dir)
{
	direction = dir;
	state = WALKING;
	distanceMoved = 1.f;
	if (paused)
		return;
	QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, walkingAcceleration * direction.NormalizedCopy()));
	QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, walkingDamping));
}
void RRMovingProperty::WalkTo(ConstVec3fr dest)
{
	state = WALKING_TO;
//	std::cout<<"\nWalking to: "<<dest;
	destination = dest;
	Vector3f vel = walkingAcceleration * (dest - owner->worldPosition).NormalizedCopy();
	if (paused)
		return;
	QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, vel));
	QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, walkingDamping));
}
void RRMovingProperty::Stop()
{
	state = IDLE;
	if (paused)
		return;
	QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
	QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, idleDamping));
}

