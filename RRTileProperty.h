/// Emil Hedemalm
/// 2016-07-19
/// Tiles.

#pragma once

enum 
{
	NOT_ADDED_YET,
	JUST_CREATED,
	WALKABLE,
	UNWALKABLE, BIOME = UNWALKABLE,
	ROAD,
	ZONE,
	WALL,
	BUILDING,
	SHALLOW_WATER,
	WATER,
	ENEMY_SPAWN_AREA,
	BLACK_VOID,
	HEALING_FOUNTAIN,
	EVENT,
	NPC, 
	ZONE_INTO_BUILDING,
	DOOR,
};

#define RTP RRTileProperty

#include "Properties.h"
class RRTileProperty : public EntityProperty
{
public:
	RRTileProperty(Entity * owner)
		:EntityProperty("RRMapProp", ID(), owner)
	{
		healingFountain = false;
		walkable = false;
		arrival = false;
		enemySpawnArea = false;
		isZone = false;
		type = JUST_CREATED;
	}
	static int ID() { return RR_TILE_PROPERTY;};
	String ToString();

	/// Spawns representational entities for this property.
	void SpawnEntities();
	/// o-o yao.
	Vector3f position;
	/// Used in all maps?
	bool isZone; // If true, zones to somewhere.  Location stored in text
	bool healingFountain;
	bool walkable;
	bool arrival; /// Had Arrival as text, nearby zone for correct placement.
	bool enemySpawnArea; 
	int type; // See above types.
	String text; /// For some types, holds data identifying further. E.g. number for NPC.
private:

};
