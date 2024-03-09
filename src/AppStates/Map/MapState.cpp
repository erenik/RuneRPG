/// Evergreen IT AB
/// 2024-02-28
/// Map/zone navigation state. 50% of game time in here.

#include "MapState.h"

#include "Battle/SpriteAnimationProperty.h"
#include "EntityStates/RREntityState.h"
#include "Graphics/Animation/AnimationManager.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Input/InputManager.h"
#include "Map/Mob.h"
#include "Map/RMap.h"
#include "Map/MobSpawner.h"
#include "Map/MapIntegrator.h"
#include "Maps/MapManager.h"
#include "Maps/2D/TileMap2D.h"
#include "Message/Message.h"
#include "Message/MathMessage.h"
#include "Map/MobProperty.h"
#include "Physics/Integrator.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsManager.h"
#include "StateManager.h"
#include "TextureManager.h"
#include "Viewport.h"
#include "Window/AppWindowManager.h"

namespace {
	RRPlayer* player = nullptr;
	Entity* playerEntity = nullptr;
	Camera* camera;
	bool left = false, right = false, up = false, down = false;

	float playerSpeed = 5;
	Direction playerDirection;

	List<SpriteAnimationProperty*> attackSpritesToDisappear;

	RMap* rmap;

	Vector3f offsetPlaneToMobs(0, -0.5f, 0);
	Vector3f offsetSpawners(0, -0.1f, 0);


	int prevLevel = 0;
	Time announcementStart = Time::Now();
	String announcementText;
	int announcementDurationMillis = 0;
};


void CenterCamera();
void UpdateExpLevel();
void HideAnnouncement();


MapState::MapState()
: RRGameState()
{
	id = RUNE_GAME_STATE_MAP;
	name = "MapState";
	enterMode = EnterMode::NULL_MODE;
	camera = CameraMan.NewCamera("Map camera");
	mapToLoad = NULL;
	lastModifiedEntity = NULL;
	menuOpen = false;
	keyPressedCallback = true;
	mapTestWindow = NULL;
	stepsTakenAfterZoning = 0;
	stepsSinceLastBattle = 0;
}

MapState::~MapState()
{
}

// Creates the user interface for this state
void MapState::CreateUserInterface(){
	QueueGraphics(new GMPushUI("gui/MapState.gui"));
}

Entity* Placeholder(String color, Vector3f position) {
	CreationSettings cs = CreationSettings(true, "plane.obj", color, position, 1.f);
	cs.rotationX = PI;
	Entity* entity = MapMan.CreateEntity("Placeholder", cs);
	//	flatSpriteCreationSettings
	return entity;
}

void AddMobSpawner(MobType type, Vector3f position) {
	Entity* entity = Placeholder("0xff0000ff", position);
	MobSpawner* ms = new MobSpawner(type, entity);
	rmap->mobSpawners.AddItem(ms);
}

void MapState::OnEnter(AppState * previousState) {
	std::cout<<"\nMapState::OnEnter";

	rmap = new RMap();

	// Load animation sets for map!
	AnimationMan.LoadAnimationSetsFromDirectory("anim/Map");
	AnimationMan.LoadAnimationSetsFromDirectory("anim/Battle");

	// Set physics integrator to simple!
	Integrator* integrator = new MapIntegrator();
	integrator->gravity = Vector3f();
	QueuePhysics(new PMSet(integrator));

	Viewport * mainViewport = MainWindow()->MainViewport();
	mainViewport->renderAI = false;
	mainViewport->renderNavMesh = false;
	mainViewport->renderGrid = true;
	mainViewport->renderNavMesh = true;
	mainViewport->renderPhysics = true;
	mainViewport->renderFPS = true;
	mainViewport->renderLights = false;

	QueueGraphics(new GMPushUI("gui/MapState.gui"));

	// Create a plane.
	auto plane = Placeholder("0x888888ff", Vector3f());
	plane->Scale(50.f);
	MapMan.CreateEntity("Plane", CreationSettings(true, "plane.obj", "0xff0000ff", Vector3f(), 50.f));

	// add some trees
//	flatSpriteCreationSettings
	Placeholder("0x00ff00ff", Vector3f(5, -0.5f, 0));


	// Create player
	if (player == NULL){
		player = new RRPlayer("Player");
		String texture = "img/Char/Char.png";
		playerEntity = MapMan.CreateEntity("Player", CreationSettings(true, "plane.obj", texture, offsetPlaneToMobs, 1.f));
		rmap->playerEntity = playerEntity;
		QueueGraphics(new GMSetCamera(camera));
	}

	UpdateExpLevel();

	// Create a shop

	// Add some monsters
	AddMobSpawner(MobType::Default, Vector3f(10,0,0)+ offsetSpawners);
	AddMobSpawner(MobType::Default, Vector3f(13, 0, 3)+ offsetSpawners);
	AddMobSpawner(MobType::Default, Vector3f(13, 0, -3)+ offsetSpawners);

	// Play!


	/// For reloading maps, looking at coordinates, etc.
	if (!mapTestWindow)
	{
//		mapTestWindow = WindowMan.NewWindow("Map test");
	//	mapTestWindow->requestedSize = Vector2i(400, 600);
	//	mapTestWindow->requestedRelativePosition = Vector2i(-400,0);
	//	mapTestWindow->Create();
	//	mapTestWindow->ui = new UserInterface();
	//	mapTestWindow->ui->Load("gui/MapTest.gui");
	//	mapTestWindow->CreateGlobalUI();
	//	mapTestWindow->DisableAllRenders();
	//	mapTestWindow->renderUI = true;
	}		
	if (mapTestWindow)
		;//mapTestWindow->Show();
//
//
//	SleepThread(100);
	
	/// Depending on previous state, modify menu.
	if (previousState->GetID() == RUNE_GAME_STATE_MAIN_MENU) {
		/// Hide the menu.
		CloseMenu();
	}
//

	/// Create entities for the players as needed? or later?

	/// Continue should probably be default mode, yo.
	if (enterMode == EnterMode::CONTINUE) {
		std::cout<<"\nMapState::OnEnter Continue";
		// Reload map we were on!
		Map * map = MapMan.GetMap(currentMap);
		assert(map);
		MapMan.MakeActive(map);
//
		/// Stuff. o.o
		/// Check if menu is open, if so set NavigateUI to true!
		if (menuOpen){
			InputMan.SetNavigateUI(true);
		}
//
//		/// Stop all players movement, e.g. after returning from a battle? Could check previous state for this?
		RREntityState * entityState = this->GetMainPlayerState();
		if (entityState) {
			entityState->StopMoving();
		}
	}
	else if (enterMode == EnterMode::TESTING_MAP)
	{
//		assert(false);
//		/*
//		std::cout<<"\nMapState::OnEnter Testing map.";
//		assert(activeMap != NULL && "Test map should have been set before entering!");
//		// Load all maps that were created/added in the editor...!
//		activeMap->ResetEvents();
//		Graphics.QueueMessage(new GMSetUIb("MainMenuButton", GMUI::VISIBILITY, false));
//		Graphics.QueueMessage(new GMSetUIb("ReturnToEditorButton", GMUI::VISIBILITY, true));
//		/// Call it explicitly if we came from the editor.
//		activeMap->OnEnter();
//		*/
	}
//	/// Track camera on ze playah!
	TrackPlayer();
//
//	// And set it as active
	QueueGraphics(new GMSet(GT_MAIN_CAMERA, camera));
	CenterCamera();

////	Graphics.UpdateProjection();


	auto mainWindow = WindowMan.MainWindow();
	/// Toggle debug renders
	mainViewport = mainWindow->MainViewport();
	mainViewport->EnableAllDebugRenders(true);
	mainViewport->renderFPS = true;
//	Graphics.selectionToRender = NULL;
	mainViewport->renderNavMesh = true;
//
//	// And start tha music..
#ifdef USE_AUDIO
	AudioMan.Play(BGM, "2013-02-21 Impacto.ogg", true);
#endif

//	ResetCamera();
}


void MapState::OnExit(AppState *nextState) {
}

float playerRotation = 0;

namespace {
	bool wasBlocking = false;
}

void UpdateSprite() {
	bool isBlocking = rmap->playerStats.IsBlocking();
	if (isBlocking != wasBlocking) {
		QueueGraphics(new GMSetEntityTexture(rmap->playerEntity, DIFFUSE_MAP,
			isBlocking ? "img/Char/Char_Blocking.png" : "img/Char/Char.png"));
	}

	wasBlocking = isBlocking;
}

void MapState::Process(int timeInMs) {
	Sleep(10);

	rmap->Process(timeInMs);

	if (announcementText.Length()) {
		Time now = Time::Now();
		if ((now - announcementStart).Milliseconds() > announcementDurationMillis) {
			HideAnnouncement();
		}
	}

	UpdateSprite();
	
	// Update HUD
	QueueGraphics(new GMSetUIi("HP", UITarget::IntegerInput, rmap->playerStats.hp));
	QueueGraphics(new GMSetUIi("Fatigue", UITarget::IntegerInput, rmap->playerStats.Fatigue()));

	rmap->playerStats.Process(timeInMs);


	if (rmap->playerStats.hp <= 0) {
		// DEAD!
		QueuePhysics(new PMSetEntity(playerEntity, PT_POSITION, offsetPlaneToMobs));
		// Respawn somewhere? clear mobs?
 		rmap->ClearMobs();
		rmap->playerStats = Mob(MobType::Player);
		UpdateExpLevel();
	}

	Vector3f moveVec;
	if (up)
		moveVec.z += 1;
	else if (down)
		moveVec.z -= 1;
	if (left) moveVec.x -= 1;
	if (right) moveVec.x += 1;

	Vector3f newSpeed = moveVec * playerSpeed;
	if (moveVec.x > 0) {
		playerDirection = Direction::Right;
	}
	else if (moveVec.x < 0) {
		playerDirection = Direction::Left;
	}
	else if (moveVec.z > 0)
		playerDirection = Direction::Up;
	else if (moveVec.z < 0)
		playerDirection = Direction::Down;

	QueuePhysics(new PMSetEntity(playerEntity, PT_VELOCITY, newSpeed));

	playerRotation += timeInMs * 0.01f;
	QueuePhysics(new PMSetEntity(playerEntity, PT_SET_ROTATION, Vector3f(PI, 0, 0)));

//	for (int i = 0; i < attackSpritesToDisappear.Size(); ++i) {
	//	SpriteAnimationProperty* sap = attackSpritesToDisappear[i];
//		sap->durationMilliseconds
};

void MapState::MouseMove(int x, int y, bool lDown, bool rDown, UIElement * elementOver)
{
}

// Callback from the Input-manager, query it for additional information as needed.
void MapState::KeyPressed(int keyCode, bool downBefore)
{
}

void MapState::OpenMenu() {
}

void MapState::CloseMenu() {
}

namespace {
	float cameraRotationX = 1.87f;
	float cameraRotationY = 0;
};

void CenterCamera() {
	Angle angle(-cameraRotationX - PI / 2);
	Vector2f toVec = angle.ToVector2f() * 5.f;
	QueueGraphics(new GMSetCamera(camera, CT_POSITION, Vector3f(0, toVec.x, -toVec.y)));
	QueueGraphics(new GMSetCamera(camera, CT_ROTATION, Vector3f(cameraRotationX, cameraRotationY, 0)));
	QueueGraphics(new GMSetCamera(camera, CT_ENTITY_TO_TRACK, playerEntity));
	QueueGraphics(new GMSetCamera(camera, CT_SMOOTHING, 0.1f));
}

void MapState::ProcessMessage(Message * message) {

	const String& msg = message->msg;
	switch (message->type)
	{
		case MessageType::OnUIReloaded: {
			QueueGraphics(new GMPushUI("gui/MapState.gui"));
			break;
		}
		case MessageType::INTEGER_MESSAGE: {
			IntegerMessage* fm = (IntegerMessage* )message;
			if (msg == "SetSpeed") {
				playerSpeed = fm->value;
			}
			break;
		}
		case MessageType::STRING: {
			if (msg == "CenterCamera") {
				CenterCamera();
			}
			else if (msg == "Attack") {
				Attack();
			}
			else if (msg == "Block") {
				Block();
			}
			else if (msg == "StartWalkLeft") {
				left = true;
				right = false;
			}
			else if (msg == "StartWalkRight") {
				right = true;
				left = false;
			}
			else if (msg == "StartWalkUp") {
				up = true;
				down = false;
			}
			else if (msg == "StartWalkDown") {
				down = true;
				up = false;
			}
			else if (msg == "StopWalkLeft") {
				left = false;
			}
			else if (msg == "StopWalkRight")
				right = false;
			else if (msg == "StopWalkDown")
				down = false;
			else if (msg == "StopWalkUp")
				up = false;
		}
	}
}

Vector3f GetPlayerFaceDirection() {
	Vector3f dir = GetVector(playerDirection);
	dir.z = dir.y;
	dir.y = 0;
	return dir;
}

void ShowAnnouncement(String text, int durationMs) {
	announcementText = text;
	QueueGraphics(new GMSetUIs("Announcement", UITarget::Text, text));
	QueueGraphics(new GMSetUIb("Announcement", UITarget::Visible, true));
	announcementStart = Time::Now();
	announcementDurationMillis = durationMs;
}

void HideAnnouncement() {
	QueueGraphics(new GMSetUIb("Announcement", UITarget::Visible, false));
}

void UpdateExpLevel() {
	int newLevel = rmap->playerStats.level;
	if (newLevel != prevLevel) {
		ShowAnnouncement("Leveled up!", 3000);
		prevLevel = newLevel;
	}


	QueueGraphics(new GMSetUIi("Level", UITarget::IntegerInput, rmap->playerStats.level));
	QueueGraphics(new GMSetUIi("EXP", UITarget::IntegerInput, rmap->playerStats.exp));
}

void MapState::Attack() {

	if (rmap->playerStats.Fatigue() < 5) {
		ShowAnnouncement("Out of fatigue!", 3000);
		return;
	}

	rmap->playerStats.ConsumeFatigue(5);

	// Add a sprite to animate it.
	Vector3f dir = GetPlayerFaceDirection();
	Vector3f damagePoint = playerEntity->worldPosition + dir;
	String tex = "img/Char/Attack.png";
	Entity* entity = Placeholder(tex, playerEntity->worldPosition + dir);

	SpriteAnimationProperty* sap = new SpriteAnimationProperty(200, entity);
	entity->properties.AddItem(sap);

	attackSpritesToDisappear.AddItem(sap);

	// Find and deal damage.
	auto mobs = rmap->mobs;
	for (int i = 0; i < mobs.Size(); ++i) {
		MobProperty* mob = mobs[i];
		Vector3f vecTo = mob->owner->worldPosition - damagePoint;
		vecTo.y = 0;
		float maxDist = (vecTo).MaxPart();
		if (maxDist > 1.f)
			continue;
		mob->stats.Damage(10);
		if (mob->stats.hp <= 0) {
			mob->Die();
			rmap->playerStats.GetExp(5);
			UpdateExpLevel();
		}
	}
}

void MapState::Block() {
	if (rmap->playerStats.Fatigue() < 5) {
		ShowAnnouncement("Out of fatigue!", 3000);
		return;
	}

	rmap->playerStats.ConsumeFatigue(5);
	rmap->playerStats.StartBlocking();

}

// To look at ze player?
void MapState::ResetCamera()
{
	camera->projectionType = Camera::ORTHOGONAL;
	camera->rotation = Vector3f();
	camera->SetTargetZoom(10.f, true);
	camera->farPlane = -50.0f;
//	
	Entity * mainPlayerEntity = GetMainPlayerEntity();
	if (mainPlayerEntity)
		camera->position = mainPlayerEntity->worldPosition;

//	camera->SetRatio(Graphics.width, Graphics.height);
	camera->distanceFromCentreOfMovement = 10.f;
	// Only update before rendering.
//	camera->Update();
	/// Reset what parts of the map are rendered too..!
	TileMap2D * map = (TileMap2D *)MapMan.ActiveMap();
	if (map)
		map->SetViewRange(20);
}

// Bind camera to ze playah.-
void MapState::TrackPlayer() {
	camera->entityToTrack = GetMainPlayerEntity();
	camera->trackingMode = TrackingMode::ADD_POSITION;
}
