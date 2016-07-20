/// Emil Hedemalm
/// 2016-07-19
/// Tiles.

#include "RRTileProperty.h"
#include "Entity/EntityManager.h"
#include "Zone.h"
#include "XML/XMLParser.h"
#include "XML/XMLElement.h"
#include "StateManager.h"
#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "PhysicsLib.h"
#include "Input/InputManager.h"
#include "Physics/Messages/CollisionCallback.h"

#include "Entity/EntityManager.h"
#include "Graphics/Camera/CameraUtil.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "String/StringUtil.h"
#include "File/FileUtil.h"
#include "File/LogFile.h"
#include "Random/Random.h"

Random tileRand;
List<String> biomes;
extern Zone zone;

/// Spawns representational entities for this property.
void RRTileProperty::SpawnEntities()
{
	if (biomes.Size() == 0)
	{
		String biome = zone.biome;
		int result = GetFilePathsInDirectory("img/Biome/"+biome, biomes);
		result = GetFilePathsInDirectory("img/Biome", biomes);
	}
	/// Move creation of entity/representation into RRTileProperty file? No?

	/// Placeholder texture colors.
	String texture;
	List<String> possibleTextures;
	switch(type)
	{
		case BIOME: 
		{
			possibleTextures;
			texture = "0xFF"; 
			if (biomes.Size())
			{
				int randI = tileRand.Randi(1000);
//				std::cout<<"\nRandI: "<<randI<<" : "<<randI % biomes.Size();
				texture = biomes[randI % biomes.Size()];
			}
			break;
		}
		case ENEMY_SPAWN_AREA: 
			break;
		case WALKABLE: 
			break; // Nothing needed graphically wise... for now.
		case NPC:
		case EVENT:
			texture = "0x007F00"; 
			break;
		case ROAD: texture = "0xd9d9d9"; break;
		case ZONE: texture = "0x447FAA"; break; // 0xE5E5FF
		case NOT_ADDED_YET: texture = "0xAA88"; break;
		case WALL: texture = "0x85200c"; break;
		case BUILDING: texture = "0xb45f06"; break;
		case SHALLOW_WATER: texture = "0xa2c4c9"; break;
		case BLACK_VOID: texture = "0x00"; break;
		case WATER: texture = "0x0000FF"; break;
		case ZONE_INTO_BUILDING: texture = "0xFFAAAA"; break;
		case HEALING_FOUNTAIN: texture = "0x00FFFFFF"; break;
		default: 
			LogMain("Default texture for tile of type "+String(type), INFO);
			texture = "0xFF0000FF"; break;
	}

	/// Physical entity.
	Entity * entity = EntityMan.CreateEntity("ij", ModelMan.GetModel((walkable? "plane" : "cube")), 0);
	entity->SetPosition(position);
	entity->properties.AddItem(this);
	MapMan.AddEntity(entity, false, true);

	/// Graphics entity.
	if (texture.Length())
	{
		Texture * tex = TexMan.GetTexture(texture);
		Vector2f scale = Vector2f(tex->size) / 64.0f;
		scale.Clamp(1.f, 5.f);
		Entity * graphicalEntity = EntityMan.CreateEntity("ij", ModelMan.GetModel("plane"), tex);
		QueueGraphics(new GMSetEntityb(graphicalEntity, GT_DEPTH_WRITE, false));
		graphicalEntity->SetPosition(position + Vector3f(0,0, -(scale.y - 1.f) * 0.5f));
		graphicalEntity->SetScale(Vector3f(scale.x, 1, scale.y));
		graphicalEntity->properties.AddItem(this);
		QueueGraphics(new GMSetEntityb(graphicalEntity, GT_IS_ALPHA_ENTITY, true));
		MapMan.AddEntity(graphicalEntity, true, false);
		/// only create if needed? or rather, skip adding the pr
		this->owners.Add(entity, graphicalEntity);
	}
	else 
		this->owner = entity;

	QueuePhysics(new PMSetEntity(entity, PT_PHYSICS_SHAPE, ShapeType::MESH));
	switch(type)
	{
		case ZONE:
		case HEALING_FOUNTAIN:
		case ZONE_INTO_BUILDING:
		case EVENT:
		case ENEMY_SPAWN_AREA:
		case NPC:
			QueuePhysics(new PMSetEntity(entity, PT_COLLISION_CALLBACK, true));
			break;
	}
}
