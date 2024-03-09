/// Evergreen IT AB
/// 2024-03-07

#pragma once

#include "Entity/EntityProperty.h"

class SpriteAnimationProperty : public EntityProperty {
public:
	SpriteAnimationProperty(int durationMs, Entity* owner);

	void Process(int timeInMs) override;

	int durationMilliseconds;

private:
	int millisProcessed = 0;
};

