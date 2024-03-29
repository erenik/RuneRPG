// Emil Hedemalm
// 2014-04-18
// RuneRPG Entity State, attached to all entities when moving about on the maps or in battles

#ifndef RUNE_PLAYER_STATE_H
#define RUNE_PLAYER_STATE_H

#include "MathLib.h"
#include "Entity/EntityProperty.h"
#include "Direction.h"
#include "Maps/Grids/Tile.h"

class EntityStateTile2D;

class RREntityState : public EntityProperty
{
public:
	RREntityState(Entity * owner);
	virtual ~RREntityState();
	/// Function when entering this state.
	void OnEnter();
	/// Main processing function
	virtual void Process(int timePassedInMs);
	/// Function when leaving this state
	void OnExit();
	/// Function for handling messages sent to the entity.
	void ProcessMessage(Message * message);

	/// Sets queued movement/direction to 0.
	void StopMoving();
	/// Wosh.
	void DisableMovement();
	void EnableMovement();

	/// Direction the entity is walking!
	Direction GetDirection();
	/// Seconds per tile 
	float movementSpeed;
private:
	EntityStateTile2D * entityTile2D;
	/// One step in the looking-direction.
	Vector2i GetInteractionTile();
	/// Based on the requested direction..!
	void UpdateQueuedMovement();

	/// Handle movement based on queued movement..!
	void HandleMovement(int timePassedInMs);

	/// Time per tile, in seconds.
	static const float DEFAULT_MOVEMENT_SPEED;
	

	/// To avoid checking every single frame.
	Tile * lastTile;

	/// In seconds
	float timePassedSinceLastMovement;
	int tileX, tileY;
	bool left, right, up, down;
	/// Movement will be queued...!
	bool movementEnabled;
	bool isMoving;
	Direction direction, queuedDirection, previousDirection, lastQueuedDirection;
	/// Should go from 0 to 1.0 upon completion.
	float movementProgress;
	Vector3f previousPosition, 
		newPosition;
};

#endif
