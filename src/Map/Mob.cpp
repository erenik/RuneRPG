/// Evergreen IT AB
/// 2024-03-07

#include "Mob.h"

Mob::Mob() {}

Mob::Mob(MobType type)
	: type(type)
{
	Init(type);
}

void Mob::Init(MobType type) {
	fatigue = 100;
	fatigueRegenPerSecond = 5;
	level = 0;
	exp = -1;
	hpRegenPer3Seconds = 1;
	switch (type) {
	case MobType::Player:
		maxHp = hp = 100;
		maxFatigueMillis = milliFatigue = 100000;
		damage = 10;
		attack = defense = 5;
		fatigueRegenPerSecond = 5;
		hpRegenPer3Seconds = 1;
		level = 1;
		exp = 0;
		UpdateStats();
		break;
	case MobType::Default:
		hp = 5;
		damage = 1;
		attack = defense = 1;
	}
}

void Mob::Damage(int damage) {
	timeOutsideOfCombatMs = 0;

	if (isBlocking) // no damage!
		return;

	hp -= damage;
	if (hp <= 0)
		KO = true;
}

void Mob::Process(int timeInMs) {
	if (milliFatigue == 0)
		milliFatigue = fatigue * 1000;

	float regenFactor = 1.f;
	if (timeOutsideOfCombatMs < 10000) {
		regenFactor = 0.1f;
	}

	if (isBlocking) {
		timeBlockingMs += timeInMs;
		if (timeBlockingMs > 1000) {
			isBlocking = false;
		}
	}

	milliFatigue += timeInMs * fatigueRegenPerSecond * regenFactor;
	if (milliFatigue > maxFatigueMillis)
		milliFatigue = maxFatigueMillis;

	hpRegenTicker += timeInMs;
	if (hpRegenTicker > 3000) {
		hpRegenTicker -= 3000;
		hp += hpRegenPer3Seconds * regenFactor;
		if (hp > maxHp)
			hp = maxHp;
	}

	timeOutsideOfCombatMs += timeInMs;

}

void Mob::ConsumeFatigue(int amount) {
	milliFatigue -= amount * 1000;
	if (milliFatigue < 0)
		milliFatigue = 0;

	if (amount > 3)
		timeOutsideOfCombatMs = 0;
}

int Mob::Fatigue() const {
	return milliFatigue / 1000;
}

void Mob::GetExp(int amount) {
	exp += amount;
	if (exp >= toNextLevel) {
		++level;
		exp = exp - toNextLevel;
		UpdateStats();
	}
}

void Mob::StartBlocking() {
	isBlocking = true;
	timeBlockingMs = 0;
	timeOutsideOfCombatMs = 0;
}

void Mob::UpdateStats() {
	int levelFrom1 = level - 1;
	switch (type) {
	case MobType::Player:
		toNextLevel = level * 100;
		maxFatigueMillis = (100 + levelFrom1  * 5) * 1000;
		maxHp = 100 + levelFrom1 * 5;
		fatigueRegenPerSecond = 5 + levelFrom1;
		hpRegenPer3Seconds = 1 + levelFrom1;
		break;
	}
}
