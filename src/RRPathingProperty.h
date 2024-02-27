/// Emil Hedemalm
/// 2016-07-30
/// Pathing property for RuneRPG, working with the RRMovingProperty, and in the background the PathableProperty in the Pathfinding system.

#ifndef RR_PATH_PROP
#define RR_PATH_PROP

#include "Entity/EntityProperty.h"
#include "Properties.h"

namespace Condition {
enum 
{
	NO_CONDITION,
	TIME_OF_DAY,
};
};

struct PathPattern
{
	PathPattern(){ distance = type = condition = -1; Nullify(); };
	PathPattern(int type):type(type) {distance = 5; Nullify(); };
	void Nullify() { durationMs = 1000; condition = Condition::NO_CONDITION; };
	int type;
	int condition;
	int distance;
	int durationMs; // Delay before action should be repeated. Default 1000ms
	List<int> tod; // Time of day, if applicable.
	enum {
		IDLE, STAY_STILL = IDLE,
		RANDOM, // uses distance, uses repeatWaitMs
		TO_ZONE,
		GO_HOME,
	};
};
/** Reacts to the following messages:
		PauseMovement
		ResumeMovement

*/
class RRPathingProperty : public EntityProperty
{
public:
	RRPathingProperty(Entity * owner, ConstVec3fr startingPosition);
	virtual ~RRPathingProperty(){}
	static int ID() { return RR_PATHING_PROPERTY; }; 

	virtual void ProcessMessage(Message * message);
	virtual void Process(int timeInMs);
	/// Initializes with a Random of 10 distance, for morning day, Home at evening/night.
	void InitRandom();
	/// Should be called on spawn, in order to be mostly message-based.
	void OnSpawn();
	/// Go! Wherever you should go o.o. If random, randomizes new position to go to. Mostly sends message to set new destination for the PathableProperty.
	void Go();
	/// Repeat command to initiate this movement.
	void Do();

	PathPattern * CurrentPattern();

	List<PathPattern> pattern;
	int currentPatternIndex; // start at -1, index of the array.
	Vector3f startingPosition;
	int timeInCurrentPatternMs;
	bool goQueued;

	/// If true, halts any requests.
	static bool pause;
};

#endif
