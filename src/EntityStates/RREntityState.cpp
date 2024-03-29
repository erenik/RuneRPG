// Emil Hedemalm
// 2014-04-18
// RuneRPG Entity State, attached to all entities when moving about on the maps or in battles

#include "RREntityState.h"

#include "Message/Message.h"

#include "Physics/Messages/CollisionCallback.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GraphicsMessages.h"

#include "Maps/MapManager.h"
#include "Maps/2D/TileMap2D.h"
#include "Maps/2D/EntityStateTile2D.h"

#include "Pathfinding/PathfindingProperty.h"
#include "Script/ScriptManager.h"
#include "StateManager.h"

const float RREntityState::DEFAULT_MOVEMENT_SPEED = 0.25f;

RREntityState::RREntityState(Entity * entity)
: EntityProperty("RREntityState", 0, entity)
{
	name = "RREntityState";
	entityTile2D = NULL;
	movementSpeed = DEFAULT_MOVEMENT_SPEED;
	timePassedSinceLastMovement = 0;
	left = right = up = down = false;
	isMoving = false;
	direction = queuedDirection = Direction::NO_DIRECTION;
	movementProgress = 0.0f;
	movementEnabled = true;
};

RREntityState::~RREntityState()
{

}

/// Function when entering this state.
void RREntityState::OnEnter(){
}

/// Main processing function
void RREntityState::Process(int timePassedInMs){
	// 
	timePassedSinceLastMovement += timePassedInMs;
	
	/// Only process movement if enabled!
	if (movementEnabled)
		HandleMovement(timePassedInMs);

}
	/// Function when leaving this state
void RREntityState::OnExit(){
}

void RREntityState::ProcessMessage(Message * message)
{
	std::cout<<"\nRREntityPropertyState::ProcessMessage: ";
	switch(message->type){
		case MessageType::STRING: {
			message->msg;
			String & s = message->msg;
			s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			
			/// Disable any input for movement when specified.

			if (!movementEnabled)
				return;
			if (s == "walkLeft")
				left = true;
			else if (s == "walkRight")
				right = true;
			else if (s == "walkUp")
				up = true;
			else if (s == "walkDown")
				down = true;
			else if (s == "stopWalkLeft")
				left = false;
			else if (s == "stopWalkRight")
				right = false;
			else if (s == "stopWalkUp")
				up = false;
			else if (s == "stopWalkDown")
				down = false;
			else if (s == "Interact")
			{
				/// Check if there's any stuff on the map at looking location and trigger their OnInteract events if so!
				TileMap2D * tmap = (TileMap2D*)MapMan.ActiveMap();
				if (!tmap){
					std::cout<<"\nERROR: RREntityState::ProcessMessage: No Active map to interact on!";
					return;
				}
				Vector2i intTile = GetInteractionTile();
				/// Send along our entity as reference.
				tmap->Interact(intTile, owner);
			}
			break;						  
		}					  
		case MessageType::COLLISSION_CALLBACK: {
					
			break;
		}
	}	
	UpdateQueuedMovement();
}

/// Sets queued movement/direction to 0.
void RREntityState::StopMoving()
{
	left = right = up = down = false;
	UpdateQueuedMovement();
}


/// Wosh.
void RREntityState::DisableMovement()
{
	movementEnabled = false;
}
void RREntityState::EnableMovement()
{
	movementEnabled = true;
}

/// One step in the looking-direction.
Vector2i RREntityState::GetInteractionTile()
{		
	// Check if tile is vacant and walkable, if so move to it.
	Vector3f pos = owner->worldPosition;
	pos.Round();
	int tileX = pos.x;
	int tileY = pos.y;

	Direction dir = lastQueuedDirection;
	if (dir == Direction::None)
		dir = previousDirection;
	switch(dir){
		case Direction::LEFT:
			tileX--;
			break;
		case Direction::RIGHT:
			tileX++;
			break;
		case Direction::UP:
			tileY++;
			break;
		case Direction::DOWN:
			tileY--;
			break;
	}
	Vector2i tile(tileX, tileY);
	return tile;
}

/// Based on the requested direction..!
void RREntityState::UpdateQueuedMovement()
{
	if (up)
		queuedDirection = Direction::UP;
	else if (left)
		queuedDirection = Direction::LEFT;
	else if (right)
		queuedDirection = Direction::RIGHT;
	else if (down)
		queuedDirection = Direction::DOWN;
	else
		queuedDirection = Direction::None;
	/// If we have moment?
	if (queuedDirection != Direction::None){
		lastQueuedDirection = queuedDirection;
		// Inform the Physics/Pathfinder of it!
		QueueGraphics(new GMSetEntity(owner, GT_ANIMATION, "Walk"));
	}
	/// And not?
	else {
		PathfindingProperty * path = owner->pathfindingProperty;
		path->desiredVelocity = Vector3f();
		std::cout<<"\nStopping.";
	}
}

void RREntityState::HandleMovement(int timePassedInMs)
{
	float timeRemainingAfterReachingDestination = 0.0f;
	if (isMoving)
	{
		/// Check if we're there yet.
		assert(movementSpeed > 0);
		movementProgress += timePassedInMs / movementSpeed;
		if (movementProgress > 1.0f){
			timeRemainingAfterReachingDestination = (movementProgress - 1.0f) * movementSpeed;
			movementProgress = 1.0f;
			isMoving = false;
			previousDirection = direction;
			direction = Direction::None;
			TileMap2D * map = (TileMap2D*)MapMan.ActiveMap();
			Vector3f pos = owner->worldPosition;
			pos.Round();
			assert(false);
		//	map->OnArrive(entity, pos.x, pos.y);
			/// Update 2D-position.
			entityTile2D->position.x = pos.x;
			entityTile2D->position.y = pos.y;
			std::cout<<"\nUpdating position to: "<<entityTile2D->position.x<<", "<<entityTile2D->position.y;
		}
		/// Process the movement
		Vector3f framePosition = newPosition * movementProgress + previousPosition * (1 - movementProgress);
	//	Graphics.PauseRendering();
		Physics.QueueMessage(new PMSetEntity(owner, PT_POSITION, framePosition));
	//	entity->recalculateMatrix();
	//	Graphics.ResumeRendering();
		
	}
	
	/// If we reached next tile, see if we got a queued direction and occupy that next tile if possible and if so! :)
	if (direction == Direction::None && queuedDirection != Direction::None){
		// Check if tile is vacant and walkable, if so move to it.
		Vector3f pos = owner->worldPosition;
//		std::cout<<"\nPosition: "<<pos;

		TileMap2D * map = (TileMap2D*)MapMan.ActiveMap();
		Tile * closestTile = map->GetTile(pos);
		if (closestTile != lastTile)
		{
			// Check for 
			lastTile = closestTile;
			if (closestTile)
			{
				for (int i = 0; i < closestTile->onEnter.Size(); ++i)
				{
					Script * script = closestTile->onEnter[i];
					ScriptMan.PlayScript(script);
				}
			}
		}

		previousPosition = pos;
		pos.Round();
		tileX = pos.x;
		tileY = pos.y;

		int reqTileX = tileX;
		int reqTileY = tileY;

		if (timePassedSinceLastMovement < movementSpeed)
			return;
		timePassedSinceLastMovement = 0;

		// 
		Vector3f requestedVelocity;
		switch(queuedDirection){
		case Direction::LEFT:
			reqTileX--;
			requestedVelocity.x = -1.0f;
			break;
		case Direction::RIGHT:
			reqTileX++;
			requestedVelocity.x = 1.0f;
			break;
		case Direction::UP:
			reqTileY++;
			requestedVelocity.y = 1.0f;
			break;
		case Direction::DOWN:
			requestedVelocity.y = -1.0f;
			reqTileY--;
			break;
		default:
			requestedVelocity = Vector3f();
			break;
		}
		requestedVelocity *= 5.0f;
		assert(owner->physics);

		/// Send requested velocity to the animation thingy!
		PathfindingProperty * path = owner->pathfindingProperty;
		path->desiredVelocity = requestedVelocity;
		/// Consider setting this somewhere else..
		owner->physics->boundToNavMesh = true;
//		std::cout<<"\nSetting desired velocity to: "<<requestedVelocity;
		/// Update 2D-position.
		if (entityTile2D)
		{
			entityTile2D->position.x = pos.x;
			entityTile2D->position.y = pos.y;

//			std::cout<<"\nUpdating position to: "<<entityTile2D->position.x<<", "<<entityTile2D->position.y;
		}
/*
		/// Check if tile is walkeable and place as there if so.
		TileMap2D * map = (TileMap2D*)MapMan.ActiveMap();
		if (map->IsTileVacant(reqTileX, reqTileY)){
			std::cout<<"\nChecking if requested tile is vacant: "<<reqTileX<<" "<<reqTileY;
			bool success = map->MoveEntity(entity, reqTileX, reqTileY);
			if (success){
				newPosition = Vector3f(reqTileX, reqTileY, 0);
				std::cout<<"\nRREntityPropertyState::HandleMovement Moved to: "<<newPosition;
				direction = queuedDirection;
				movementProgress = 0.0f;
				isMoving = true;
			}
			
		}
*/
	//	else {
	//	}
	}
	// No queued movement and no current movement = stand still!
	else if (direction == Direction::None && queuedDirection == Direction::None){
		QueueGraphics(new GMSetEntity(owner, GT_ANIMATION, "Idle"));
	}
	/// Walk more if we had stuff remaining, yo.
	if (timeRemainingAfterReachingDestination > 0){
		Process(timeRemainingAfterReachingDestination);
	}
}
