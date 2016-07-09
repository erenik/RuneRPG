/// Emil Hedemalm
/// 2016-07-09
/// A zone. Load/Pathfinding/Simulation functions.

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

#include "File/FileUtil.h"
#include "File/LogFile.h"

Camera * Zone::editCamera = 0, * playerCamera = 0;

// Time lastZone = ;
AABB aabb;
String currentZone, previousZone;
Time zoneReqTime = Time::Now();
Vector3f previousPosition; // Saved every second? two seconds?

struct Style 
{
	Style(){ index = -1;}
	void Print(){ std::cout<<"\nStyle "<<index<<" "<<hexColor;}
	String hexColor;
	int index; // index as specified in the file.
};

List<Style> styles;
Style * GetStyle(int byIndex)
{
	for (int i = 0; i < styles.Size(); ++i)
	{
		Style * s = &styles[i];
		if (s->index == byIndex)
			return s;
	}
	return 0;
}

void Zone::SetupEditCamera()
{
	editCamera = CameraMan.DefaultCamera();
	editCamera->trackingMode = TrackingMode::NONE;
	QueueGraphics(new GMSetCamera(editCamera));

}

bool ZoneExists(String zone)
{
	return FileExists("data/Zones/"+zone+".html");
}

/// Name of zone, default dir "data/Zones/*.html"
bool Zone::Load(String zone)
{
	if (previousZone != currentZone)
		previousZone = currentZone;
	currentZone = zone;
	zoneTiles.Clear();
	styles.Clear();
	XMLParser parser;
	bool ok = parser.Read("data/Zones/"+zone+".html");
	if (!ok)
	{
		std::cout<<"Failed to read file";
		return false;
	}
	ok = parser.Parse();
	XMLElement * e = parser.GetElement("style");
	assert(e && "No style element?");
	String style = e->data;
	int place = 0;
	Vector2i min, max;
	while((place = style.Find(" .s")) != -1)
	{
		Style st;
		String subString = style.Part(place, place + 100);
		List<String> args = subString.Tokenize("{;}");
		for (int i = 0; i < args.Size(); ++i)
		{
			String arg = args[i];
			if (arg.StartsWith("background-color"))
			{
				st.index = subString.ParseInt();
				st.hexColor = arg.Tokenize(":")[1];
				break;
			}
		}
		style = style.Part(place+2); // Keep remaining non-parsed string?
		if (st.index < 0)
			continue;
		styles.AddItem(st);
	}
	std::cout<<"\n"<<styles.Size()<<" styles found";
	/// Parse the table.
	aabb = AABB();
	List<RRTileProperty*> zoneCenters, zoneReceivers;
	XMLElement * body = parser.GetElement("tbody"); // Body
	for (int i = 0; i < body->children.Size(); ++i) // Rows
	{
		XMLElement * row = body->children[i];
		for (int j = 0; j < row->children.Size(); ++j) // Cell in row (column)
		{
			XMLElement * xe = row->children[j];
			if (xe->name != "td") // Table-data, only relevant.
				continue; // Skip header-skills in the edges.
			assert(xe->name == "td");
			/// Skip first 2 columns for the creation?
			if (j < 2)
				continue;
			
			int type = WALKABLE;
			/// Arguments?
			if (xe->args.Size())
			{
				XMLArgument * arg = xe->GetArgument("class");
				int st = (arg->value - "s").ParseInt();
				Style * s = GetStyle(st);
				if (s)
				{
					if (s->hexColor == "#b7b7b7")		type = UNWALKABLE;
					else if (s->hexColor == "#ffffff")  type = WALKABLE;
					else if (s->hexColor == "#d9d9d9")	type = ROAD;
					else if (s->hexColor == "#d9ead3")  type = ZONE;
					else if (s->hexColor == "#85200c")  type = WALL;
					else if (s->hexColor == "#b45f06")  type = BUILDING;
					else if (s->hexColor == "#a2c4c9")  type = SHALLOW_WATER;
					else if (s->hexColor == "#dd7e6b")  type = ENEMY_SPAWN_AREA;
					else if (s->hexColor == "#3d85c6")  type = WATER;
					else if (s->hexColor == "#a4c2f4") type = NPC;
					else if (s->hexColor == "#93c47d") type = HEALING_FOUNTAIN;
					else if (s->hexColor == "#b4a7d6") type = EVENT;
					else type = NOT_ADDED_YET;
				}
			}	
			Vector3f offset;
			String texture = "0x77"; // White default?
			bool walkable = true;
			bool hasPhysics = true, hasGraphics = true;
			switch(type)
			{
				case UNWALKABLE: walkable = false; offset.y += 0.5f; texture = "0xFF"; break;
				case ENEMY_SPAWN_AREA: 
				case WALKABLE: 
				case NPC:
				case EVENT:
					texture = "0x007F00"; 
					break;
				case ROAD: texture = "0xd9d9d9"; break;
				case ZONE: texture = "0x447FAA"; break; // 0xE5E5FF
				case NOT_ADDED_YET: texture = "0xAA88"; walkable = false; break;
				case WALL: texture = "0x85200c"; walkable = false; break;
				case BUILDING: texture = "0xb45f06"; walkable = false; break;
				case SHALLOW_WATER: texture = "0xa2c4c9"; break;
				case BLACK_VOID: texture = "0x00"; walkable = false; break;
				case WATER: texture = "0x0000FF"; walkable = false; break;
				case ZONE_INTO_BUILDING: texture = "0xFFAAAA"; walkable = false; break;

			}
			// Do different stuff depending on the style?
			/// Skip walkables for now? Add 'em later when we know where we need 'em?
			if (type == WALKABLE)
				continue;
			Vector3f pos = Vector3f((float)j, 0, (float)i) + offset;
			Entity * entity = EntityMan.CreateEntity("ij", ModelMan.GetModel((walkable? "plane" : "cube")), TexMan.GetTexture(texture));
			entity->SetPosition(pos);
			MapMan.AddEntity(entity, hasGraphics, hasPhysics);
			QueuePhysics(new PMSetEntity(entity, PT_PHYSICS_SHAPE, ShapeType::MESH));
			RRTileProperty * rtp = 0;
			switch(type)
			{
				case ZONE:
				case HEALING_FOUNTAIN:
					QueuePhysics(new PMSetEntity(entity, PT_COLLISION_CALLBACK, true));
					rtp = new RRTileProperty(entity);
					entity->properties.AddItem(rtp);
					break;
			}
			if (rtp)
			{
				if (type == ZONE)
				{
					zoneTiles.AddItem(rtp);
					// Look for the target zone within, if possible.
					if (xe->children.Size())
					{
						XMLElement * div = xe->children[0]; // div
						rtp->zone = div->data;
					}
					if (rtp->zone.Length() < 1)
					{
						zoneReceivers.AddItem(rtp);
						rtp->zone = "CheckNeighbours";
					}
					else
						zoneCenters.AddItem(rtp);
				}
				else if (type == HEALING_FOUNTAIN)
				{
					rtp->healingFountain = true;
				}
			}
			aabb.Expand(pos);
			/// Assign it.
		}
	}
	/// Spread zone data as needed.
	for (int i = 0; i < zoneReceivers.Size(); ++i)
	{
		RRTileProperty * rtp = zoneReceivers[i], * closest = 0;
		float minDist = 1000000;
		for (int j = 0; j < zoneCenters.Size(); ++j)
		{
			RRTileProperty * rtp2 = zoneCenters[j];
			float dist = (rtp2->owner->worldPosition - rtp->owner->worldPosition).LengthSquared();
			if (dist < minDist)
			{
				minDist = dist;
				closest = rtp2;
			}
		}
		if (closest)
			rtp->zone = closest->zone;
	}
	/// Calc aabb. center etc.
	aabb.UpdatePositionScaleUsingMinMax();
	std::cout<<"\nAABB: "<<aabb.position<<" scale: "<<aabb.scale;
	aabb.position.y = 0;
	// Add plane under-neath.
	Entity * basePlane = MapMan.CreateEntity("Baseplane", ModelMan.GetModel(("plane")), TexMan.GetTexture("0x007F00"),  aabb.position - Vector3f(0,0.01f,0));
	QueuePhysics(new PMSetEntity(basePlane, PT_PHYSICS_SHAPE, ShapeType::MESH));
	QueuePhysics(new PMSetEntity(basePlane, PT_SET_SCALE, 200.f));
}

Entity * playerEntity = 0;
void Zone::CreatePlayer(ConstVec3fr  position)
{
	std::cout<<"\nSpawning at position: "<<position;
	playerEntity = MapMan.CreateEntity("Player", ModelMan.GetModel("sphere"), TexMan.GetTexture("0x00FF00FF"), position + Vector3f(0, 0.5f,0));
	QueuePhysics(new PMSetEntity(playerEntity, PT_PHYSICS_SHAPE, ShapeType::SPHERE));
	QueuePhysics(new PMSetEntity(playerEntity, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));
	QueuePhysics(new PMSetEntity(playerEntity, PT_SET_SCALE, 0.25f));
	QueuePhysics(new PMSetEntity(playerEntity, PT_LINEAR_DAMPING, 0.5f));
	QueuePhysics(new PMSetEntity(playerEntity, PT_RESTITUTION, 0.0f));
	QueuePhysics(new PMSetEntity(playerEntity, PT_FRICTION, 0.02f));
	// Give movement capabilities?
	playerCamera = CameraMan.NewCamera("PlayerCamera", true);
	playerCamera->entityToTrack = playerEntity;
	playerCamera->trackingMode = TrackingMode::ADD_POSITION;
	playerCamera->rotation.x = Angle(PI/2);
	playerCamera->rotation.y = 0;
	playerCamera->projectionType = Camera::ORTHOGONAL;
	playerCamera->zoom = 8.f;
	playerCamera->smoothness = 0.01f;
//	c->rotation.y = Angle();
	playerCamera->position = Vector3f(0,2.5f,0);
	QueueGraphics(new GMSetCamera(playerCamera)); // Make active.
}

void Zone::ZoneTo(String zone, ConstVec3fr dir)
{
	/// Zone!
	if (!ZoneExists(zone))
	{
		LogMain("Zone "+zone+" not present in dir.", INFO);
		return;
	}
	String zoningFrom = currentZone;
	/// Check player velocity, copy it.
	Vector3f playerDir = playerEntity->Velocity().NormalizedCopy();
	// Remove player.
	MapMan.DeleteAllEntities();
	playerEntity = 0;
	Sleep(20);
	Load(zone);
	/// Place correctly?
	Vector3f average; 
	int num = 0;
	for (int i = 0; i < zoneTiles.Size(); ++i)
	{
		RRTileProperty * rrtp = zoneTiles[i];
		if (rrtp->zone == zoningFrom)
		{
			/// Place here?
			average += rrtp->owner->worldPosition;
			++num;
		}
	}
	average /= MaximumFloat(float(num), 1.f);
//	Vector3f dirToCenter = (aabb.position - average).NormalizedCopy();
	Vector3f pos = average + (dir) * 1.25f;
	CreatePlayer(pos);
}

void Zone::Process(int timeInMs)
{
	static int seconder = 0;
	seconder += timeInMs;
	if (seconder > 1000)
	{
		seconder = seconder % 1000;
		previousPosition = (playerEntity? playerEntity->worldPosition : Vector3f());
	}

	/// Evaluate every.. something.
	Vector2i dir(0,0);
	if (InputMan.KeyPressed(KEY::W))
		++dir.y;
	if (InputMan.KeyPressed(KEY::S))
		--dir.y;
	if (InputMan.KeyPressed(KEY::A))
		++dir.x;
	if (InputMan.KeyPressed(KEY::D))
		--dir.x;
	if (InputMan.KeyPressed(KEY::C))
	{
		CameraMan.NextCamera();
		Sleep(100);
	}
	if (InputMan.KeyPressed(KEY::U))
	{
		Camera * c = CameraMan.ActiveCamera();
		c->zoom *= 1.1f;
//		c->SetRatioF(10, 20, true); // rotation.x += Angle(0.1f);
		Sleep(100);
	}
	if (InputMan.KeyPressed(KEY::R))
	{
		/// Respawn.
		MapMan.DeleteEntity(playerEntity);
		CreatePlayer(aabb.position);
		Sleep(100);
		return;
	}
	if (InputMan.KeyPressed(KEY::I))
	{
		std::cout<<"\nPos: "<<playerEntity->worldPosition;
		Sleep(100);
	}
	if (InputMan.KeyPressed(KEY::L))
	{
		MapMan.DeleteAllEntities();
		String zone = currentZone; 
		currentZone = "";
		Load(zone);
		CreatePlayer(aabb.position);
		Sleep(100);
		return;
	}
	float walkSpeed = 15.f;
	static Vector2i oldDir;
	if (oldDir != dir && playerEntity)
	{
		QueuePhysics(new PMSetEntity(playerEntity, PT_ACCELERATION, - walkSpeed * Vector3f(float(dir.x), 0, float(dir.y))));
		if (dir.Length())
			QueuePhysics(new PMSetEntity(playerEntity, PT_LINEAR_DAMPING, 0.5f));
		else 
			QueuePhysics(new PMSetEntity(playerEntity, PT_LINEAR_DAMPING, 0.1f));
		oldDir = dir;
	}
}

void Zone::ProcessMessage(Message * message)
{
	String msg = message->msg;
	Camera * activeCamera = CameraMan.ActiveCamera();
	if (activeCamera == editCamera)
		ProcessCameraMessages(msg, activeCamera);
	switch(message->type)
	{
		case MessageType::COLLISSION_CALLBACK:
		{
			CollisionCallback * cc = (CollisionCallback*) message;
			Entity * object = (cc->one == playerEntity)? object = cc->two : object = cc->one;
			RRTileProperty * rtp = object->GetProperty<RRTileProperty>();
			Time now = Time::Now();
			if (rtp)
			{
				if (rtp->zone.Length() && rtp->zone != currentZone)
				{
					if ((now - zoneReqTime).Seconds() < 1)
					{
						std::cout<<"\nSkipping request, already got one in the last second.";
						return;
					}
					/// Queue up the zoning, ignore next requests for the next second?
					Vector3f offset = (object->worldPosition - previousPosition).NormalizedCopy() * 1.3f;
					ZoneTo(rtp->zone, offset);
					zoneReqTime = Time::Now();
				}
			}
			break;
		};
	}
}
