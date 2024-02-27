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
#include "Graphics/Messages/GMSetEntity.h"

#include "File/FileUtil.h"
#include "File/LogFile.h"
#include "Pathfinding/PathMessage.h"
#include "Random/Random.h"
#include "RRMovingProperty.h"

#include "Physics/PhysicsManager.h"
#include "RRDialogueProperty.h"

Camera * Zone::editCamera = 0, * playerCamera = 0;

// Time lastZone = ;
AABB aabb;
String currentZone, previousZone;
Time zoneReqTime = Time::Now();
Vector3f previousPosition; // Saved every second? two seconds?
Vector2i oldDir;

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

void Zone::EvaluateText(String text, RTP * rtp)
{
	if (text == "Arrival")
		rtp->arrival = true;
	if (text == "e")
		rtp->enemySpawnArea = true;
	if (text.Contains("Biome:"))
	{
		biome = text.Tokenize(":")[1];
		biome.RemoveSurroundingWhitespaces();
	}
	else 
	{
		std::cout<<"Text found: "+text;
		rtp->text = text;
	}
}

#include "Pathfinding/PathManager.h"
#include "OS/Sleep.h"
#include "RRPathingProperty.h"

/// Name of zone, default dir "data/Zones/*.html"
bool Zone::Load(String zone)
{
	if (previousZone != currentZone)
		previousZone = currentZone;
	rtps.Clear();
	currentZone = zone;
	zoneTiles.Clear();
	styles.Clear();
	npcs.ClearAndDelete();

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
	List<RRTileProperty*> zoneCenters, zoneReceivers; // Zone tiles
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
					else { type = NOT_ADDED_YET; LogMain("HexColor undocumented: "+s->hexColor, LogLevel::INFO); };
				}
			}	
			Vector3f offset;
			bool walkable = true;
			switch(type)
			{
				case UNWALKABLE: walkable = false; offset.y += 0.5f; 
					break;
				case NOT_ADDED_YET: 
				case WALL:
				case BUILDING:
				case BLACK_VOID:
				case WATER:
				case ZONE_INTO_BUILDING:
					walkable = false; 
					break;
			}
			// Do different stuff depending on the style?
			/// Skip walkables for now? Add 'em later when we know where we need 'em?
			/// - Just don't create entities to represent them then. Or create appropriate borders with other tiles just.
//			if (type == WALKABLE)
	//			continue;
			RRTileProperty * rtp = new RRTileProperty(0);
			rtp->position = Vector3f((float)j, 0, (float)i) + offset;
			rtp->walkable = walkable;
			rtp->type = type;
			aabb.Expand(rtp->position);
			rtps.AddItem(rtp);

			if (xe->children.Size())
			{
				XMLElement * div = xe->GetElement("div");
				if (div)
				{
					String text = div->data;
					EvaluateText(text, rtp);
				}
			}

			/// Assign it.
			if (type == ZONE)
			{
				zoneTiles.AddItem(rtp);
				// Look for the target zone within, if possible.
				rtp->isZone = true;
				if (rtp->text.Length() == 0){
					zoneReceivers.AddItem(rtp);
					rtp->text = "CheckNeighbours";
				}
				else
					zoneCenters.AddItem(rtp);
			}
			else if (type == HEALING_FOUNTAIN)
			{
				rtp->healingFountain = true;
			}
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
			float dist = (rtp2->position - rtp->position).LengthSquared();
			if (dist < minDist)
			{
				minDist = dist;
				closest = rtp2;
			}
		}
		if (closest)
			rtp->text = closest->text;
	}
	/// Calc aabb. center etc.
	aabb.UpdatePositionScaleUsingMinMax();
	std::cout<<"\nAABB: "<<aabb.position<<" scale: "<<aabb.scale;
	aabb.position.y = 0;
	// Add plane under-neath.
	String ts = "0x07F00";
	if (biome == "Mountain")
		ts = "0xAD8349";
	else if (biome == "Forest")
		ts = "0x007F00";
	Entity * basePlane = MapMan.CreateEntity("Baseplane", ModelMan.GetModel(("plane")),
		TexMan.GetTexture(nullptr, TextureCategory::Asset, ts),  aabb.position - Vector3f(0,0.01f,0));
	QueuePhysics(new PMSetEntity(basePlane, ShapeType::MESH));
	QueuePhysics(new PMSetEntity(basePlane, PT_SET_SCALE, 200.f));
	QueueGraphics(new GMSetEntitys(basePlane, GT_ENTITY_GROUP, "Tiles"));
	
	/// Start spawning entities.
	loading = true;
	tilesLoaded = 0;

	extern List<String> biomes;
	biomes.Clear();
	QueuePhysics(new PhysicsMessage(PM_RESUME_SIMULATION));

}

#include "RuneEntity.h"

RuneEntity * player = 0;

void Zone::CreatePlayer(ConstVec3fr  position)
{
	if (player == 0)
		player = new RuneEntity();
	player->Spawn(position);

	// Give movement capabilities?
	playerCamera = CameraMan.NewCamera("PlayerCamera", true);
	playerCamera->entityToTrack = player->graphicalEntity;
	playerCamera->trackingMode = TrackingMode::ADD_POSITION;
	playerCamera->rotation.x = Angle(PI/2);
	playerCamera->rotation.y = 0;
	playerCamera->projectionType = Camera::ORTHOGONAL;
	playerCamera->SetTargetZoom(8.f, true);
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
		LogMain("Zone "+zone+" not present in dir.", LogLevel::INFO);
		return;
	}
	String zoningFrom = currentZone;
	// Remove player.
	MapMan.DeleteAllEntities();
	Sleep(20);
	Load(zone);
	/// Place correctly?
	Vector3f average; 
	int num = 0;
	for (int i = 0; i < zoneTiles.Size(); ++i)
	{
		RRTileProperty * rrtp = zoneTiles[i];
		if (rrtp->text == zoningFrom)
		{
			/// Place here?
			average += rrtp->position;
			++num;
		}
	}
	average /= MaximumFloat(float(num), 1.f);
	/// Check nearest Arrival tile nearby.
	Vector3f closest;
	float closestDist = 10000.f;
	for (int i = 0; i < rtps.Size(); ++i)
	{
		RRTileProperty * rtp = rtps[i];
		if (!rtp->arrival)
			continue;
		float dist = (rtp->position - average).LengthSquared();
		if (dist < closestDist)
		{
			closestDist = dist;
			closest = rtp->position;
		}
	}
	playerSpawnPos = closest;	
}

#include "Pathfinding/WaypointManager.h"
void Zone::GenerateNavMesh()
{
	NavMesh * nm = WaypointMan.GetNavMesh(currentZone);
	if (!nm)
		nm = WaypointMan.CreateNavMesh(currentZone);
	nm->waypoints.ClearAndDelete(); // Cleanse it.

	std::cout<<"\nGenerating "<<rtps.Size()<<" waypoints";
	for (int i = 0; i < rtps.Size(); ++i)
	{
		RRTileProperty * rtp = rtps[i];
		if (!rtp->walkable)
			continue; // Skip walkables? for now.
		nm->AddWaypoint(new Waypoint(rtp->position));
	}
	std::cout<<"\nConnecting waypoints by proximity...";
	nm->ConnectWaypointsByProximity(1.1f);
	std::cout<<"\nNavmesh generated";
}
void Zone::Load()
{
	if (loading == SPAWN_TILES)
	{
		// Spawn some more entities....wtf?
		int toLoad = tilesLoaded + 100;
		for (; tilesLoaded < toLoad && tilesLoaded < rtps.Size(); ++tilesLoaded)
		{
			RRTileProperty * rtp = rtps[tilesLoaded];
			rtp->SpawnEntities();
			if (tilesLoaded >= rtps.Size() - 1)
			{
				++loading;
			}
		}
	}
	else if (loading == GENERATE_NAVMESH)
	{
		GenerateNavMesh();
		++loading;
	}
	else if (loading == SPAWN_NPCS)
	{
		RRPathingProperty::pause = false;
		std::cout<<"\nSpawning NPCs";
		for (int i = 0; i < rtps.Size(); ++i)
		{
//			continue;
			RRTileProperty * rtp = rtps[i];
			if (rtp->type == NPC)
			{
				// Spawn an NPC here too.
				RuneEntity * npc = new RuneEntity();
				npc->name = rtp->text;
				npc->isNpc = true;
				/// Spawn it right away too?
				npc->Spawn(rtp->position);
				npcs.AddItem(npc);
			}
		}
		++loading;
	}
	else
	{
		loading = false;
		CreatePlayer(playerSpawnPos);
		std::cout<<"\nTotal entities: "<<MapMan.GetEntities().Size();
		oldDir = Vector2i();
	}
}

List<Entity*> GetEntities(ConstVec3fr nearPos, float andWithinRadius)
{
	Entities ent = MapMan.GetEntities();
	/// Interact with nearby entities.
	Entities relevant;
	for (int i = 0; i < ent.Size(); ++i)
	{
		Entity * e = ent[i];
		if ((e->worldPosition - nearPos).Length() < andWithinRadius)
			relevant.AddItem(e);
	}
	return relevant;
}

void Zone::Process(int timeInMs)
{
	static int seconder = 0;
	seconder += timeInMs;

	if (loading)
	{
		Load();
		return;
	}

	Entity * playerEntity = 0;
	if (player)
	{
		playerEntity = player->physicsEntity;
	}


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
	if (InputMan.KeyPressed(KEY::L))
	{
		Entities ent = MapMan.ActiveMap()->GetEntities();
		std::cout<<"\nListing entities: "<<ent.Size();
		for (int i = 0; i < ent.Size(); ++i)
		{
			std::cout<<"\n"<<ent[i]->name;
		}
	}
	if (InputMan.KeyPressed(KEY::ENTER))
	{
		if (Dialogue::ActiveDialogue())
		{
			Dialogue::ActiveDialogue()->Continue();
			Sleep(200);
			return;
		}
		List<Entity*> relevantEntities = GetEntities(playerEntity->worldPosition + playerEntity->Velocity().NormalizedCopy() * 0.5f, 0.8f);
		Message message("Interact");
		for (int i = 0; i < relevantEntities.Size(); ++i)
		{
			Entity * e = relevantEntities[i];
			e->ProcessMessage(&message);
		}
		Sleep(200);
	}
	if (InputMan.KeyPressed(KEY::C))
	{
		CameraMan.NextCamera();
		Sleep(100);
	}
	if (InputMan.KeyPressed(KEY::U))
	{
		Camera * c = CameraMan.ActiveCamera();
		c->SetTargetZoom(1.1f * c->CurrentZoom(), false);
//		c->SetRatioF(10, 20, true); // rotation.x += Angle(0.1f);
		Sleep(100);
	}
	if (InputMan.KeyPressed(KEY::R) && InputMan.KeyPressed(KEY::S))
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
	if (InputMan.KeyPressed(KEY::G))
	{
		// Go walking!
		Random r;
		RRTileProperty * rtp = 0;
		int index = r.Randi() % rtps.Size();
		while(rtp == 0 || rtp->walkable == false)
		{
			index = r.Randi() % rtps.Size();
			rtp = rtps[index];
		}
		std::cout<<"\nWalking to random tile: "<<index<<" "<<rtp->ToString();
		SetPathDestinationMessage walk(rtp->position); // Get random position?
		playerEntity->ProcessMessage(&walk);
		Sleep(200);
	}
	if (InputMan.KeyPressed(KEY::L) && InputMan.KeyPressed(KEY::R))
	{
		/// Pause physics. - Queue it by bypassing the StateManager, or it will lag extra much.
		PhysicsMan.QueueMessage(new PhysicsMessage(PM_PAUSE_SIMULATION));
		/// Pause pathfinding. Wait until all path threads have returned.
		RRPathingProperty::pause = true;
	//	PathMan.acceptRequests = false;
		while(PathMan.ThreadsActive()) 
			SleepThread(10);

		MapMan.DeleteAllEntities();
		String zone = currentZone; 
		currentZone = "";
		Load(zone);
		CreatePlayer(aabb.position);
		Sleep(100);
		return;
	}

	/// Don't update movement if dialogue is active.
	if (Dialogue::ActiveDialogue())
		return;

	float walkSpeed = 15.f;
	if (oldDir != dir && playerEntity)
	{
		if (dir.Length())
			player->movingProp->Walk(Vector3f(-dir.x, 0, -dir.y));
		else 
			player->movingProp->Stop();
		oldDir = dir;
	}
}

void Zone::ProcessMessage(Message * message)
{
	String msg = message->msg;
	Camera * activeCamera = CameraMan.ActiveCamera();
	if (activeCamera == editCamera)
		ProcessCameraMessages(msg, activeCamera);
	if (msg.StartsWith("OnDialogueBegun"))
	{
		Message msg ("PauseMovement");
		player->physicsEntity->ProcessMessage(&msg);
	}
	else if (msg.StartsWith("OnDialogueEnded"))
	{
		Message msg("ResumeMovement");
		player->physicsEntity->ProcessMessage(&msg);
	}
	switch(message->type)
	{
		case MessageType::COLLISSION_CALLBACK:
		{
			if (!player)
				return;
			CollisionCallback * cc = (CollisionCallback*) message;
			Entity * object = (cc->one == player->physicsEntity)? object = cc->two : object = cc->one;
			Entity * collider = cc->one == object? cc->two : cc->one;
			RRTileProperty * rtp = object->GetProperty<RRTileProperty>();
			Time now = Time::Now();
			if (rtp)
			{
				if (rtp->isZone && rtp->text.Length() && rtp->text != currentZone)
				{
					if (collider != player->physicsEntity)
						return;
					if ((now - zoneReqTime).Seconds() < 1)
					{
						std::cout<<"\nSkipping request, already got one in the last second.";
						return;
					}
					/// Queue up the zoning, ignore next requests for the next second?
					Vector3f offset = (object->worldPosition - previousPosition).NormalizedCopy() * 1.3f;
					ZoneTo(rtp->text, offset);
					zoneReqTime = Time::Now();
				}
			}
			break;
		};
	}
}
