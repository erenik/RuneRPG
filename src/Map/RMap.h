/// Evergreen IT AB
/// 2024-03-07
/// Map class to store all monsters and player and current effects.

#pragma once

#include "Mob.h"
#include "Entity/Entity.h"

class MobProperty;
class MobSpawner;

class RMap {
public:
	RMap();
	
	void Process(int timeInMs);

	void ClearMobs();
	
	List<MobProperty*> mobs;
	Entity* playerEntity;

	List<MobSpawner*> mobSpawners;

	Mob playerStats;

private:


};


