/// Evergreen IT AB
/// 2024-03-07
/// Logic for the mobs

#include "MobProperty.h"

#include "Battle/SpriteAnimationProperty.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Maps/MapManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "RMap.h"
#include "StateManager.h"

MobProperty::MobProperty(RMap* rmap, Entity* owner, MobType type)
	: EntityProperty("MobProp", EntityPropertyID::CUSTOM_GAME_1 + 1, owner)
	, rmap(rmap)
{
	stats = Mob(type);
}

void MobProperty::Process(int timeInMs) {

	Vector3f toPlayer = rmap->playerEntity->worldPosition - owner->worldPosition;
	float distance = toPlayer.MaxPart();


	switch (animState) {
	case AnimState::ReadyingAttack:
		readyingMs += timeInMs;
		if (readyingMs > 500) {
			Attack();
		}
		return;
	}

	cooldownMsAttack -= timeInMs;

	// Close enough?
	if (distance < 1.f) {
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f()));
		TryAttack();
	}
	// Too far? stop chasing
	else if (distance >= 10.f) {
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f()));
	}
	else {
		toPlayer.Normalize();
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, toPlayer));
	}
}

void MobProperty::TryAttack() {
	if (cooldownMsAttack > 0)
		return;

	animState = AnimState::ReadyingAttack;
	readyingMs = 0;
	// Increase scale!
	QueueGraphics(new GMSetEntityf(owner, GT_SCALE, 1.5f));
	defaultTextureSrc = owner->diffuseMap->name;
	QueueGraphics(new GMSetEntityTexture(owner, DIFFUSE_MAP, "0xff8800ff"));
	QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f()));

}

void MobProperty::Attack() {
	QueueGraphics(new GMSetEntityf(owner, GT_SCALE, 1.0f));

	cooldownMsAttack = 1000;

	// Visualize?
	Vector3f damagePoint = (rmap->playerEntity->worldPosition + owner->worldPosition) * 0.5f;
	damagePoint.y -= 0.1f;

	animState = AnimState::Idle;
	QueueGraphics(new GMSetEntityTexture(owner, DIFFUSE_MAP, defaultTextureSrc));

	if (rmap->playerStats.IsBlocking()) {
		// Was blocking! Blue deflect
		CreationSettings cs = CreationSettings(true, "plane.obj", "0x0088ffff", damagePoint, 1.f);
		cs.rotationX = PI;
		Entity* entity = MapMan.CreateEntity("MobAttackSprite", cs);
		SpriteAnimationProperty* sap = new SpriteAnimationProperty(200, entity);
		entity->properties.AddItem(sap);
		return;
	}

	CreationSettings cs = CreationSettings(true, "plane.obj", "0xff0000ff", damagePoint, 1.f);
	cs.rotationX = PI;
	Entity* entity = MapMan.CreateEntity("MobAttackSprite", cs);
	SpriteAnimationProperty* sap = new SpriteAnimationProperty(200, entity);
	entity->properties.AddItem(sap);

	rmap->playerStats.Damage(stats.damage);

}

void MobProperty::Die() {
	rmap->mobs.RemoveItem(this);
	MapMan.DeleteEntity(owner);
}

