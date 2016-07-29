/// Emil Hedemalm
/// 2016-07-29
/// Property for moving! NPCs, Stones, anything :D 
/// Responds to GoToMessages sent by PathableProperty too.

#ifndef RR_MOVE_PROP
#define RR_MOVE_PROP

#include "Entity/EntityProperty.h"
#include "Properties.h"

class RRMovingProperty : public EntityProperty
{
public:
	RRMovingProperty(Entity * owner);
	static float ID(){ return RR_MOVING_PROPERTY; };
	virtual ~RRMovingProperty();

	virtual void ProcessMessage(Message * message);
	virtual void Process(int timeInMs);

	void Walk(ConstVec3fr direction);
	void WalkTo(ConstVec3fr dest);
	void Stop();

	float walkingAcceleration; // default 15.0
	float walkingDamping; // 0.9
	float idleDamping; // 0.1
	float distArrivalThreshold; // 0.5?

	/// If walking or idle.
	int state;
	enum {
		IDLE,
		WALKING,
		WALKING_TO, // destination
	};
private:
	int accumulator; // Update frequency once per second by default?
	Vector3f destination;
};

#endif