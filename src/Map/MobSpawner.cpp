/// Evergreen IT AB
/// 2024-03-07

#include "MobSpawner.h"

#include "Entity/Entity.h"
#include "Maps/MapManager.h"
#include "MobProperty.h"
#include "RMap.h"

MobSpawner::MobSpawner(MobType type, Entity* entity)
	: type(type), entity(entity)
{
	switch (type) {
	default:
		spawnCooldown = 3000;
	}
	spawnTimerCounter = 0;
}

void MobSpawner::Process(RMap* map, int timeInMs) {

	Vector3f toPlayer = map->playerEntity->worldPosition - entity->worldPosition;

	float distanceToPlayer = toPlayer.LengthSquared();
	if (distanceToPlayer > 100)
		return;

	spawnTimerCounter += timeInMs;
	if (spawnTimerCounter > spawnCooldown) {
		SpawnMob(map);
		spawnTimerCounter = 0;
	}
}

void MobSpawner::SpawnMob(RMap* map) {
	CreationSettings cs = CreationSettings(true, "plane.obj", "0xffff00ff", this->entity->worldPosition, 1.f);
	cs.rotationX = PI;
	Entity* newMob = MapMan.CreateEntity("Placeholder", cs);

	MobProperty* mp = new MobProperty(map, newMob, type);
	newMob->properties.AddItem(mp);
	map->mobs.AddItem(mp);
}
