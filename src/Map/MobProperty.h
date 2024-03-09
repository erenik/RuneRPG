/// Evergreen IT AB
/// 2024-03-07
/// Logic for the mobs

#pragma once

#include "Entity/EntityProperty.h"
#include "Mob.h"

class RMap;

class MobProperty : public EntityProperty {
public:
	MobProperty(RMap* rmap, Entity* owner, MobType type);
	void Process(int timeInMs) override;

	void Die();

	Mob stats;
	RMap* rmap;

	enum class AnimState {
		Idle,
		Walking,
		ReadyingAttack,
		Attack
	};

	AnimState animState;

private:
	void TryAttack();
	void Attack();

	int cooldownMsAttack = 0;
	int readyingMs = 0;

	String defaultTextureSrc;

};
