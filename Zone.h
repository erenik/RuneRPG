/// Emil Hedemalm
/// 2016-07-09
/// A zone. Load/Pathfinding/Simulation functions.

#ifndef ZONE_H
#define ZONE_H

#include "RRTileProperty.h"

class Zone 
{
public:
	Zone()
	{
		loading = false;
	}
	/// Evaluates text in a tile, what it shall signify.
	void EvaluateText(String text, RTP * rtp);
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

	Vector3f playerSpawnPos; // Pos to spawn after all tiles have been loaded.

		/// Biome for the zone.
	String biome;

private:
	/// Generates a navmesh based on the given walkable tiles.
	void GenerateNavMesh();
	/// Called iteratively while loading the map.
	void Load();

	/// While true, is loading - spawning - entities. This to reduce lag in window maneuverability. Spawns 10 tiles per frame until done.
	int loading;
	enum {
		LOADING_DONE = 0,
		SPAWN_TILES,
		GENERATE_NAVMESH,

	};
	int tilesLoaded; // Related to 'loading' above. from 0 to rtps.Size()
	/// o-o
	List<RRTileProperty*> zoneTiles, rtps;
};

#endif
