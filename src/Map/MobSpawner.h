/// Evergreen IT AB
/// 2024-03-07

#pragma once

class Entity;
class RMap;

enum class MobType;

class MobSpawner {
public:
	MobSpawner(MobType type, Entity* entity);
	void Process(RMap* map, int timeInMs);

private:

	void SpawnMob(RMap* map);

	int spawnCooldown;
	int spawnTimerCounter;
	MobType type;
	Entity* entity;
};

