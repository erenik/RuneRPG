/// Emil Hedemalm
/// 2016-07-20
/// Base class for all animatable entities on the map (and in battle too?)

#include "RuneEntity.h"
#include "StateManager.h"
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

#include "File/FileUtil.h"
#include "File/LogFile.h"

#include "Pathfinding/PathableProperty.h"
#include "RRMovingProperty.h"

/// Spawn onto grid.
void RuneEntity::Spawn(ConstVec3fr atPosition)
{
	std::cout<<"\nSpawning at position: "<<atPosition;
	Vector3f pos = atPosition + Vector3f(0, 0.5f,0);
	physicsEntity = EntityMan.CreateEntity("Player", ModelMan.GetModel("plane"), 0);
	physicsEntity->SetPosition(pos);

	/// Add graphical entity as child.
	String texture = "img/Smile.png";
	if (isNpc)
		texture = "img/SmileNPC.png";
	graphicalEntity = EntityMan.CreateEntity("Player", ModelMan.GetModel("plane"), TexMan.GetTexture(texture));
	graphicalEntity->inheritPositionOnly = true;
	physicsEntity->AddChild(graphicalEntity);

	MapMan.AddEntity(physicsEntity, false, true);
	MapMan.AddEntity(graphicalEntity, true, false);
		
	PhysicsQueue.Add(
		new PMSetEntity(physicsEntity, PT_PHYSICS_SHAPE, ShapeType::SPHERE),
		new PMSetEntity(physicsEntity, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC),
		new PMSetEntity(physicsEntity, PT_SET_SCALE, 0.35f),
		new PMSetEntity(physicsEntity, PT_LINEAR_DAMPING, 0.5f),
		new PMSetEntity(physicsEntity, PT_RESTITUTION, 0.0f));
	QueuePhysics(new PMSetEntity(physicsEntity, PT_FRICTION, 0.02f));
	
	QueueGraphics(new GMSetEntityb(graphicalEntity, GT_DEPTH_WRITE, false));
	QueueGraphics(new GMSetEntityf(graphicalEntity, GT_SCALE, 1.f));
	QueueGraphics(new GMSetEntityb(graphicalEntity, GT_IS_ALPHA_ENTITY, true));


	/// Add Pathable property for automated pathfinding.
	PathableProperty * pp = new PathableProperty(physicsEntity);
	pp->proximityThreshold = 1.4f;
	pp->updateIntervalMs = 200;
	physicsEntity->properties.AddItem(pp);

	movingProp = new RRMovingProperty(physicsEntity);
	physicsEntity->properties.AddItem(movingProp);
}



