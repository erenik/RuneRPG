/// Evergreen IT AB
/// 2024-03-07
/// Map class to store all monsters and player and current effects.

#include "RMap.h"

#include "MobSpawner.h"
#include "MobProperty.h"

RMap::RMap() {
	playerStats = Mob(MobType::Player);
}

void RMap::Process(int timeInMs) {
	for (int i = 0; i < mobSpawners.Size(); ++i) {
		mobSpawners[i]->Process(this, timeInMs);
	}
}

void RMap::ClearMobs() {
	while(mobs.Size())
		mobs[0]->Die();
}

