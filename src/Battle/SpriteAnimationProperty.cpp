/// Evergreen IT AB
/// 2024-03-07

#include "SpriteAnimationProperty.h"

#include "Maps/MapManager.h"


SpriteAnimationProperty::SpriteAnimationProperty(int durationMs, Entity* owner)
	: EntityProperty("SpriteAnimProp", EntityPropertyID::CUSTOM_GAME_1, owner)
	, durationMilliseconds(durationMs)
{
}


void SpriteAnimationProperty::Process(int timeInMs) {
	millisProcessed += timeInMs;
	if (millisProcessed > durationMilliseconds) {
		MapMan.DeleteEntity(owner);
	}
}


