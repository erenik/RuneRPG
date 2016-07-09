/// Emil Hedemalm
/// 2016-07-09
/// A zone. Load/Pathfinding/Simulation functions.

#ifndef ZONE_H
#define ZONE_H

enum 
{
	NOT_ADDED_YET,
	WALKABLE,
	UNWALKABLE,
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

#include "Entity/EntityProperty.h"
class RRTileProperty : public EntityProperty
{
public:
	RRTileProperty(Entity * owner)
		:EntityProperty("RRMapProp", ID(), owner)
	{
		healingFountain = false;
	}
	static int ID() { return 7;};
	/// Used in all maps?
	String zone; // If filled, zones to somewhere.
	bool healingFountain;
private:

};

class Zone 
{
public:
	/// Name of zone, default dir "data/Zones/*.html"
	void SetupEditCamera();
	/// Makes the zone active by default.
	bool Load(String zone); 
	void CreatePlayer(ConstVec3fr position);
	/// Tries to zone into neighbouring zone, placing player accordingly.
	void ZoneTo(String zone, ConstVec3fr dir);

	void Process(int timeInMs);
	void ProcessMessage(Message * message);

	static Camera * editCamera;
private:
	/// o-o
	List<RRTileProperty*> zoneTiles;
};

#endif
