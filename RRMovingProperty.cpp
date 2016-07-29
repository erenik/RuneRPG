/// Emil Hedemalm
/// 2016-07-29
/// Property for moving! NPCs, Stones, anything :D 
/// Responds to GoToMessages sent by PathableProperty too.

#include "RRMovingProperty.h"
#include "StateManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Pathfinding/PathMessage.h"

RRMovingProperty::RRMovingProperty(Entity * owner)
	: EntityProperty("RRMoving prop", ID(), owner)
{
	walkingAcceleration = 15.f;
	walkingDamping = 0.5f;
	idleDamping = 0.1f;
	state = IDLE;
	accumulator = 0;
	distArrivalThreshold = 0.5f;
}
RRMovingProperty::~RRMovingProperty(){}

void RRMovingProperty::ProcessMessage(Message * message)
{
	if (message->msg == "OnFinalDestinationSet")
	{
		std::cout<<"\nOnFinalDestinationset!";
	}
	if (message->type == MessageType::MOVE_TO_MESSAGE)
	{
		MoveToMessage * mtm = (MoveToMessage*) message;
		WalkTo(mtm->pos);
	}
}

void RRMovingProperty::Process(int timeInMs)
{
	accumulator += timeInMs;
	if (accumulator < 200)
		return;
	accumulator = accumulator % 200;
	// Stop when reaching position? Adjust if pushed?
	if (state == WALKING_TO)
	{
		float dist = (destination - owner->worldPosition).LengthSquared();
		if (dist < distArrivalThreshold)
		{
			Stop();
			return;
		}
		WalkTo(destination);		
	}
}

void RRMovingProperty::Walk(ConstVec3fr direction)
{
	state = WALKING;
	QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, walkingAcceleration * direction.NormalizedCopy()));
	QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, walkingDamping));
}
void RRMovingProperty::WalkTo(ConstVec3fr dest)
{
	state = WALKING_TO;
//	std::cout<<"\nWalking to: "<<dest;
	destination = dest;
	QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, walkingAcceleration * (dest - owner->worldPosition).NormalizedCopy()));
	QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, walkingDamping));
}
void RRMovingProperty::Stop()
{
	state = IDLE;
	QueuePhysics(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
	QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, idleDamping));
}

