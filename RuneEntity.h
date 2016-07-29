/// Emil Hedemalm
/// 2016-07-20
/// Base class for all animatable entities on the map (and in battle too?)

#ifndef RUNE_ENTITY_H
#define RUNE_ENTITY_H

#include "Entity/Entity.h"

class RRMovingProperty;

/// A structure for handling entity save/load/creation for the RuneRPG game. Subclass to add further adjustments/statistics. 
class RuneEntity
{
private:
	/// List of all loaded/created types.
	List<RuneEntity*> entityTypes;
public:

	RuneEntity()
	{
		graphicalEntity = physicsEntity = 0;
		movingProp = 0;
	}
	// Static functions handling all these entities!
//	static RuneEntity * 
//	static bool Save(String toFile);
//	static bool Load(String fromFile);
#define RuneEntities RuneEntityTypes
//	static List<RuneEntity> RuneEntityTypes();

	/// Spawn onto grid.
	void Spawn(ConstVec3fr atPosition);

	// Name of the entity, yow
	String name;
	// General type, can be Creature, Object, etc?
	String type;
	
	// General texture used to represent this entity. If no animation set is present, the texture will also be the primary rendering base.
	String texture;
	// Animation set to be used.
	String animSet;

	// Size on the grid.
	Vector2i size;
	// Centered, left, right, etc. Default is centered on X, bottom on Y.
	int alignment;
	
	// Further descriptions for this kind of entity. Can be anything extra?
	String description;

	/// By default the physics entity is the parent of the graphical entity, though this can be disjuncted for animations.
	Entity * graphicalEntity, * physicsEntity;
	/// Properties.
	RRMovingProperty * movingProp;

};


#endif