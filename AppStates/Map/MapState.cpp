//// Emil Hedemalm
//// 2013-06-28
//
//#include "MapState.h"
//
//#include "OS/Sleep.h"
//#include "Actions.h"
//#include "Message/Message.h"
//#include "Viewport.h"
//
//#include "UI/UIButtons.h"
//#include "UI/UserInterface.h"
//
//#include "PhysicsLib/Shapes/Ray.h"
//
//#include "Physics/Messages/CollisionCallback.h"
//#include "Physics/Messages/PhysicsMessage.h"
//#include "Physics/PhysicsManager.h"
//#include "Physics/PhysicsProperty.h"
//
//#include "Entity/EntityProperty.h"
//
//#include "Graphics/Messages/GMParticles.h"
//#include "Graphics/Messages/GMUI.h"
//
//#include "EntityStates/RREntityState.h"
//#include "Battle/RuneBattler.h"
//#include "Item/RuneShop.h"
//
//#include "Graphics/Camera/Camera.h"
//#include "Graphics/Messages/GMSet.h"
//#include "Graphics/Messages/GMSetEntity.h"
//#include "Graphics/GraphicsManager.h"
//#include "Graphics/Animation/AnimationManager.h"
//
//#include "Maps/2D/TileMap2D.h"
//#include "../../RRPlayer.h"
//#include "Player/PlayerManager.h"
//#include "Script/Script.h"
//#include "Script/ScriptProperty.h"
//#include "Entity/EntityFlags.h"
//#include "../../RuneDirectories.h"
//#include "../RuneGameStatesEnum.h"
//#include "Window/AppWindowManager.h"
//
//#include "TextureManager.h"
//#include "Maps/MapManager.h"
//#include "StateManager.h"
//#include "Input/InputManager.h"
//#include "Script/ScriptManager.h"
//#include "Model/ModelManager.h"
//#include "Pathfinding/WaypointManager.h"
//#include "File/File.h"
//#include "File/FileUtil.h"
//#include "PopulationManager.h"
//
//extern UserInterface * ui[GameStateID::MAX_GAME_STATES];
//
//MapState::MapState()
//: RRGameState()
//{
//	id = RUNE_GAME_STATE_MAP;
//	name = "MapState";
//	enterMode = EnterMode::NULL_MODE;
//	camera = CameraMan.NewCamera();
//	mapToLoad = NULL;
////	player = NULL;
////	playerEntity = NULL;
////	playerState = NULL;
//	lastModifiedEntity = NULL;
//	menuOpen = false;
//	keyPressedCallback = true;
////	activeShop = NULL;
//	mapTestWindow = NULL;
//	stepsTakenAfterZoning = 0;
//	stepsSinceLastBattle = 0;
//}
//
//MapState::~MapState()
//{
//}
//
//
///// Creates the user interface for this state
//void MapState::CreateUserInterface(){
//	if (ui)
//		delete ui;
//	ui = new UserInterface();
//	ui->Load("gui/MapState.gui");
//}
//
//void MapState::OnEnter(AppState * previousState)
//{
//	std::cout<<"\nMapState::OnEnter";
//	// Load initial texture and set it to render over everything else
//	Graphics.QueueMessage(new GMSet(GT_OVERLAY_TEXTURE, TexMan.GetTexture("img/loadingData.png")));
//	
//	// Load animation sets for map!
//	AnimationMan.LoadFromDirectory("anim/Map");
//	AnimationMan.LoadFromDirectory("anim/Battle");
//	// Set physics integrator to simple!
//	Physics.QueueMessage(new PMSet(PT_INTEGRATOR_TYPE, IntegratorType::SIMPLE_PHYSICS));
//
////	Graphics.render
//
//
//	/// For reloading maps, looking at coordinates, etc.
//	if (!mapTestWindow)
//	{
//		mapTestWindow = WindowMan.NewWindow("Map test");
//		mapTestWindow->requestedSize = Vector2i(400, 600);
//		mapTestWindow->requestedRelativePosition = Vector2i(-400,0);
//		mapTestWindow->Create();
//		mapTestWindow->ui = new UserInterface();
//		mapTestWindow->ui->Load("gui/MapTest.gui");
//		mapTestWindow->CreateGlobalUI();
//		mapTestWindow->DisableAllRenders();
//		mapTestWindow->renderUI = true;
//	}		
//	if (mapTestWindow)
//		mapTestWindow->Show();
//
//
//	SleepThread(100);
//	// Begin loading textures here for the UI
//	Graphics.QueueMessage(new GMSetUI(ui));
//	/// Depending on previous state, modify menu.
//	if (previousState->GetID() == RUNE_GAME_STATE_MAIN_MENU){
//		/// Hide the menu.
//		CloseMenu();
//	}
//
//	/// Grab first player, or create new one as needed?
//	if (PlayerMan.NumPlayers() == NULL){
//		RRPlayer * player = new RRPlayer("Player");
//		PlayerMan.AddPlayer(player);
//	}
//
//	/// Create entities for the players as needed? or later?
//
//	/// Continue should probably be default mode, yo.
//	if (enterMode == EnterMode::CONTINUE)
//	{
//		std::cout<<"\nMapState::OnEnter Continue";
//		// Reload map we were on!
//		Map * map = MapMan.GetMap(currentMap);
//		assert(map);
//		MapMan.MakeActive(map);
//
//		/// Stuff. o.o
//		/// Check if menu is open, if so set NavigateUI to true!
//		if (menuOpen){
//			InputMan.NavigateUI(true);
//		}
//
//		/// Stop all players movement, e.g. after returning from a battle? Could check previous state for this?
//		RREntityState * entityState = this->GetMainPlayerState();
//		if (entityState)
//		{
//			entityState->StopMoving();
//		}
//	}
//	else if (enterMode == EnterMode::TESTING_MAP)
//	{
//		assert(false);
//		/*
//		std::cout<<"\nMapState::OnEnter Testing map.";
//		assert(activeMap != NULL && "Test map should have been set before entering!");
//		// Load all maps that were created/added in the editor...!
//		activeMap->ResetEvents();
//		Graphics.QueueMessage(new GMSetUIb("MainMenuButton", GMUI::VISIBILITY, false));
//		Graphics.QueueMessage(new GMSetUIb("ReturnToEditorButton", GMUI::VISIBILITY, true));
//		/// Check for player character/spawn, if not, place somewhere within camera range.
//		playerEntity = activeMap->GetEntity("Player");
//		if (!playerEntity)
//			PlacePlayer(Vector3i(10,10,0));
//		/// Call it explicitly if we came from the editor.
//		activeMap->OnEnter();
//		*/
//	}
//	/// Track camera on ze playah!
//	TrackPlayer();
//
//	// And set it as active
//	Graphics.cameraToTrack = camera;
////	Graphics.UpdateProjection();
//	/// Toggle debug renders
//	Viewport * mainViewport = MainWindow()->MainViewport();
//	mainViewport->EnableAllDebugRenders(false);
//	mainViewport->renderFPS = true;
//	Graphics.selectionToRender = NULL;
//	mainViewport->renderNavMesh = true;
//
//	// And start tha music..
//#ifdef USE_AUDIO
//	AudioMan.Play(BGM, "2013-02-21 Impacto.ogg", true);
//#endif
//
//	ResetCamera();
//
//	// Set graphics manager to render UI, and remove the overlay-texture.
//	Graphics.QueueMessage(new GMSetOverlay(NULL));
//}
//
//void MapState::OnExit(AppState *nextState)
//{
//	if (mapTestWindow)
//		mapTestWindow->Hide();
//
//	// Remove pointers on any tiles to any entities, as they will be re-created all the time.
//	TileMap2D * map = this->ActiveMap();
//	std::cout<<"\nMapState::OnExit";
//	// Load initial texture and set it to render over everything else
//	Graphics.QueueMessage(new GMSetOverlay(TexMan.GetTexture("img/loadingData.png")));
//	
//	// Begin loading textures here for the UI
//	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
//	Graphics.cameraToTrack->entityToTrack = NULL;
//
//	std::cout<<"\nLeaving MapState state.";
//	MapMan.MakeActive(NULL);
//
//	// Set graphics manager to render UI, and remove the overlay-texture.
////	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
//	std::cout<<"\nLeaving MapState statewewe";
//	switch(nextState->GetID()){
//		case RUNE_GAME_STATE_BATTLE_STATE:
//		case RUNE_GAME_STATE_RUNE_STATE:
//			enterMode = EnterMode::CONTINUE;
//			this->currentMap = map->Name();
//			break;
//		default: 
//			enterMode = EnterMode::NULL_MODE;
//			break;
//	}
//}
//
//
//
//#include <ctime>
//
//void MapState::Process(int timeInMs)
//{
//	SleepThread(10);
//
//	/// Fetch playersss
//	List<RRPlayer*> players = GetPlayers();
//
//	for (int i = 0; i < players.Size(); ++i)
//	{
//		Entity * playerEntity = players[i]->mapEntity;
//	
//		// Update player position if possiblu
//		if (playerEntity)
//		{
//			Graphics.QueueMessage(new GMSetUIv2i("Position", GMUI::VECTOR_INPUT, playerEntity->position.Rounded(), mapTestWindow->ui));
//			static Vector3i lastPlayerPosition;
//			Vector3i playerPosition = playerEntity->position.Rounded();
//			if (playerPosition != lastPlayerPosition)
//			{
//				// Evaluate if we should trigger a new battle
//				lastPlayerPosition = playerPosition;
//				/// Evaluate if we should trigger some kinda battle.
//				PlayerPositionUpdated(playerPosition);
//			}
//		}
//	}
//};
//
///// Evaluate if we should trigger some kinda battle.
//void MapState::PlayerPositionUpdated(Vector3i position)
//{
//	stepsTakenAfterZoning++;
//	stepsSinceLastBattle++;
//	// Minimum X steps after zoning..?
//	if (stepsTakenAfterZoning < 5)
//		return;
//	if (stepsSinceLastBattle < 3)
//		return;
//	float totalEncounterChance = 0;
//	List<Population*> activePops = PopMan.ActivePopulations();
//	for (int i = 0; i < activePops.Size(); ++i)
//	{
//		Population * pop = activePops[i];
//		float encounterChance = pop->GetEncounterRatio(position);
//		totalEncounterChance += encounterChance;
//		String battleRef = pop->ShouldFight(position);
//		if (battleRef.Length())
//		{
//			// Start the battle..!
//			StartBattle(battleRef);
//			stepsSinceLastBattle = 0;
//		}
//	}
//	Graphics.QueueMessage(new GMSetUIf("TotalEncounterRatio", GMUI::FLOAT_INPUT, totalEncounterChance, mapTestWindow->ui));
//}
//
//enum mouseCameraStates {
//	NULL_STATE,
//	ROTATING,
//	PANNING
//};
//
//
///// Input functions for the various states
//void MapState::MouseClick(bool down, int x, int y, UIElement * elementClicked)
//{
//	if (down){
//		NavMesh * nm = WaypointMan.ActiveNavMesh();
//		if (!nm)
//			return;
//		Waypoint * wp = nm->GetClosestVacantWaypoint(cursorPosition);
//		std::cout<<"\nMouse-click waypoint: "<<wp;
//		if (wp)
//			std::cout<<" pos: "<<wp->position;
//		std::cout<<"\n";
//		/// Get main (our) player entity?
//		Entity * playerEntity = GetMainPlayerEntity();
//		if (playerEntity)
//			Physics.QueueMessage(new PMSetEntity(playerEntity, PT_DESTINATION, wp->position));
//	}
//}
//void MapState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
//}
//
///// Macros to access the active map via the MapManager.
//TileMap2D * MapState::ActiveMap(){
//	Map * map = MapMan.ActiveMap();
//	TileMap2D * tMap = (TileMap2D*)map;
//	return tMap;
//}
//	
//
//#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)
//void MapState::MouseMove(int x, int y, bool lDown, bool rDown, UIElement * elementOver)
//{
//	if (elementOver)
//		return;
//	Camera * camera = Graphics.cameraToTrack;
////	float diffX = mouseX - x;
////	float diffY = mouseY - y;
//	if (!ActiveMap())
//		return;
//	TileMapLevel * level = ActiveMap()->ActiveLevel();
//	
//	/// Get position in le welt.
//	Plane plane;
//	plane.Set3Points(Vector3f(0,-1,0), Vector3f(1,0,0), Vector3f(0,0,0));
//	Vector3f collisionPoint;
//	// , *Graphics.cameraToTrack
//	Ray clickRay = Graphics.ActiveCamera()->GetRayFromScreenCoordinates(WindowMan.MainWindow(), x, y);
//	if (clickRay.Intersect(plane, &collisionPoint))
//	{
//		cursorPosition = collisionPoint;
//	}
//	/*
//	/// Update cursor position!
//	int xi = cursorPosition.x, yi = cursorPosition.y;
//	String posString = "x: " + String::ToString(xi) +
//		" y:" + String::ToString(yi);
//	Graphics.QueueMessage(new GMSetUIs("CursorPosition", GMUI::TEXT, posString));
//	/// If good coordinate, check info
//	if (xi >= 0 && xi < size.x && yi >= 0 && yi < size.y){
//		String info;
//		switch(editMode){
//			case TILES: {
//				const Tile * t = map->GetTile(Vector2i(xi,yi));
//				if (t->type)
//					info = t->type->name;
//				else
//					info = "Empty tile";
//				break;
//			}
//			default:
//				break;
//		}
//		Graphics.QueueMessage(new GMSetUIs("CursorTargetInfo", GMUI::TEXT, info));
//	}
//
//	if (lDown){
//		/// Drag map if holding ctrl or space?
//		if (InputMan.KeyPressed(KEY::CTRL) || InputMan.KeyPressed(KEY::SPACE)){
//			if(camera){
//				if (InputMan.KeyPressed(KEY::CTRL)){
//					camera->position += camera->UpVector().CrossProduct(camera->LookingAt()) * diffX / 100.0f * PAN_SPEED_MULTIPLIER;
//					camera->position -= camera->UpVector() * diffY / 100.0f * PAN_SPEED_MULTIPLIER;
//				}
//				else if (InputMan.KeyPressed(KEY::SPACE)){
//					camera->rotation.x += diffY / 100.0f;
//					camera->rotation.y -= diffX / 100.0f;
//				}
//			}
//		}
//		else {
//			// Draw
//			switch(editMode){
//				case TILES: case TERRAIN:
//			//	case OBJECTS:
//					Paint();
//					break;
//			}
//		}
//	}
//	else if (rDown){
//		if(camera){
//			if (InputMan.KeyPressed(KEY::CTRL)){
//				camera->distanceFromCentreOfMovement += diffY;
//				if (camera->distanceFromCentreOfMovement > 0)
//					camera->distanceFromCentreOfMovement = 0;
//			}
//			else {
//				camera->position += camera->UpVector().CrossProduct(camera->LookingAt()) * diffX / 100.0f * PAN_SPEED_MULTIPLIER;
//				camera->position -= camera->UpVector() * diffY / 100.0f * PAN_SPEED_MULTIPLIER;
//			}
//		}
//	}
//
//	mouseX = x;
//	mouseY = y;
//	*/
//}
//
//void MapState::MouseWheel(float delta)
//{
//	camera->distanceFromCentreOfMovement += delta / 100.0f;
//	if (delta > 0){
//		camera->distanceFromCentreOfMovement *= 0.8f;
//	}
//	else if (delta < 0){
//		camera->distanceFromCentreOfMovement *= 1.25f;
//	}
//	if (camera->distanceFromCentreOfMovement > 0)
//		camera->distanceFromCentreOfMovement = 0;
//}
//
///// Callback from the Input-manager, query it for additional information as needed.
//void MapState::KeyPressed(int keyCode, bool downBefore)
//{
//	if (downBefore)
//		return;
////	std::cout<<"\nKey pressed: "<<keyCode;
//	switch(keyCode){
//		case KEY::ESCAPE:
//			std::cout<<"\nOpening menu";
//			OpenMenu();
//			break;
//		default: 
//			break;
//	}
//}
//
//void MapState::OpenMenu()
//{
//	/// Open the menu if it isn't open already.
//	InputMan.NavigateUI(true);
//	if (!ui->IsInStack("MainMenu"))
//	{
//		// Close all sub-menus.
//		HideMenus();
//		menuOpen = true;
//		std::cout<<"\nOpening menu.";
//		/// Push it to the stack!
//		Graphics.QueueMessage(new GMSetUIb("MainMenu", GMUI::VISIBILITY, true));
//		Graphics.QueueMessage(new GMPushUI("MainMenu", ui));
//		DisableMovement();
//	}
//	else {
//		std::cout<<"\nMenu already open o-o";
//	}
//}
//void MapState::CloseMenu()
//{
//	if (menuOpen){
//		menuOpen = false;
//		Graphics.QueueMessage(new GMPopUI("MainMenu", ui));
//	}
//	else
//		std::cout<<"\nMenu already closed o-o";
//}
//
///// Returns the entities the players are walking around with on the map.
//List<Entity*> MapState::GetPlayerEntities()
//{
//	List<Entity*> mapEntities;
//	List<RRPlayer*> players = GetPlayers();
//	for (int i = 0; i < players.Size(); ++i)
//	{
//		RRPlayer * player = players[i];
//		if (player->mapEntity)
//			mapEntities.Add(player->mapEntity);
//	}
//	return mapEntities;
//}
//
//void MapState::PlaceZone(String fromRef, int x, int y)
//{
//	String source = fromRef;
//	if (!source.Contains("data/scripts"))
//		fromRef = "data/scripts/" + fromRef;
//	List<String> lines = File::GetLines(fromRef);
//	
//	Vector2i spawnPos(x,y);
//	TileMap2D * activeMap = this->ActiveMap();
//	Tile * tile = activeMap->GetClosestVacantTile(spawnPos);	
//	if (!tile){
//		std::cout<<"\nERROR: Unable to place zone-point.";
//		return;
//	}
//	Script * script = new Script("Zone");
//	script->Load(fromRef);
//	tile->onEnter.Add(script);
//}
//
///// o-o
//void MapState::SpawnNPC(String fromRef, int x, int y)
//{
//	// Load file from ref.
//	String source = fromRef;
//	if (!source.Contains("data/NPCs"))
//		fromRef = "data/NPCs/" + fromRef;
//	List<String> lines = File::GetLines(fromRef);
//	
//	// Gather info
//	String name;
//	String onInteract;
//	for (int i = 0; i < lines.Size(); ++i)
//	{
//		String line = lines[i];
//		if (line.Contains("OnInteract"))
//			onInteract = line.Tokenize(" \t")[1];
//		else if (line.Contains("Name"))
//			name = line.Tokenize(" \t")[1];
//
//	}	
//
//	// Prepare creation
//	Model * m = ModelMan.GetModel("Sprite");
//	Texture * t = TexMan.GetTexture("Units/200");
//	Vector3f position;
//	/// Make sure an event triggered this!
//	Vector3f spawnPos(x, y, 0);
//	TileMap2D * activeMap = ActiveMap();
//	if (!activeMap)
//		return;
//	const Tile * tile = activeMap->GetClosestVacantTile(spawnPos);
//	if (!tile){
//		std::cout<<"\nERROR: Unable to spawn entity.";
//		return;
//	}
//	position = Vector3i(tile->position);
//	TileMap2D * tmap = (TileMap2D*)MapMan.ActiveMap();
//	if (!tmap->IsTileVacant(position)){
//		std::cout<<"\nERROR: Something already on that tile, or it not walkable, skipping SpawnEntity!";
//	}
//	// Create the entity
//	Entity * entity = MapMan.CreateEntity(name, m, t, position);
//	TileMap2D * tMap = (TileMap2D*)MapMan.ActiveMap();
//	if (entity == NULL){
//		std::cout<<"\nERROR: Unable to create entity in MapState!";
//		return;
//	}
//	/// Give it test animation
//	Graphics.QueueMessage(new GMSetEntity(entity, GT_ANIMATION_SET, "Map/Test"));
//	
//	// Set target waypoint to be occupied by this new entity.
//	Physics.QueueMessage(new PMSetWaypoint(position, PT_ENTITY, entity));
//	
//	// Set flags for the entity so it behaves and is removed correctly.
//	entity->flags |= SPAWNED_BY_EVENT;
//	entity->name = name;
//	entity->properties.Add(new RREntityState(entity));
//	lastModifiedEntity = entity;
//
//	// Add scripting property if lacking it.
//	if (lastModifiedEntity->scripts == NULL)
//		lastModifiedEntity->scripts = new ScriptProperty();
//	// Add OnInteract if specified in the NPC specification when parsed earlier.
//	if (onInteract.Length())
//	{
//		Script * event = new Script("OnInteract");
//		event->Load(onInteract);
//		lastModifiedEntity->scripts->onInteract = event;
//	}	
//	// Place it.
//
//}
//
///// Hides sub-menus in the main.. menu...
//void MapState::HideMenus()
//{
//	String uiString = "ItemMenu,StatusScreen";
//	List<String> uis = uiString.Tokenize(",");
//	for (int i = 0; i < uis.Size(); ++i)
//	{
//		Graphics.QueueMessage(new GMPopUI(uis[i], ui));
//	}
//}
//
///// Load shop ui for player interaction.
//void MapState::LoadShop(RuneShop * shop)
//{
//	// Clear list.
//	Graphics.QueueMessage(new GMClearUI("ShopItemList"));
//	// And re-fill it.
//	List<RuneItem*> items = shop->GetItems();
//	for (int i = 0; i < items.Size(); ++i)
//	{
//		RuneItem * item = items[i];
//		/// Create appropriate button/UI for them.
//		UIButton * button = new UIButton(item->name);
//		button->sizeRatioY = 0.1f;
//		button->activationMessage = "SelectItemToBuy:"+item->name;
//		/// Add a label on the right-hand side with price.
//		UILabel * priceLabel = new UILabel(String::ToString(item->price));
//		priceLabel->alignmentX = 0.9f;
//		priceLabel->sizeRatioX = 0.2f;
//		button->AddChild(priceLabel);
//		Graphics.QueueMessage(new GMAddUI(button, "ShopItemList"));
//	}
//}
//
//void MapState::ProcessMessage(Message * message)
//{
//	switch(message->type)
//	{
//		case MessageType::STRING: 
//		{
//			String string = message->msg;
//			String msg = message->msg;
//			string.SetComparisonMode(String::NOT_CASE_SENSITIVE);
//			string.RemoveInitialWhitespaces();
//			if (msg == "Item")
//			{
//				/// Hide all our menus,
//				HideMenus();
//				/// Then open up the item/inventory menu.
//				Graphics.QueueMessage(new GMPushUI("ItemMenu", ui));
//			}
//			else if (msg == "CloseMenu")
//			{
//				CloseMenu();
//			}
//			else if (msg == "ReloadPopulations")
//			{
//				PopMan.ReloadPopulations();
//			}
//			else if (msg.Contains("PlaceZone"))
//			{
//				List<String> args = msg.Tokenize("(), ");
//				int numArgs = args.Size();
//				int zoneCoords = (numArgs - 2) / 2;
//				if (zoneCoords < 0)
//					return;
//				String zoneScript = args[1];
//				for (int i = 0; i < zoneCoords; ++i)
//				{
//					int x = args[i*2+2].ParseInt();
//					int y = args[i*2+3].ParseInt();
//					this->PlaceZone(zoneScript, x,y);
//				}
//			}
//			else if (msg.Contains("SpawnNPC"))
//			{
//				List<String> args = msg.Tokenize("(),");
//				if (args.Size() < 4){
//					std::cout<<"\nHas to be 3 arguments when spawning npc!";				
//					return;
//				}
//				String npcRef = args[1];
//				int x = args[2].ParseInt();
//				int y = args[3].ParseInt();
//				this->SpawnNPC(npcRef, x,y);
//			}
//			else if (msg.Contains("SelectItemToBuy:"))
//			{
//			/*
//				String name = msg.Tokenize(":")[1];
//				RuneItem * item = activeShop->GetItem(name);
//				// Update active purchase info in the purchase-AppWindow.
//				Graphics.QueueMessage(new GMSetUIs("ItemToBuy", GMUI::TEXT, item->name));
//				Graphics.QueueMessage(new GMPushUI("PurchaseWindow", ui));
//				*/
//			}
//			else if (msg == "OpenTestShop")
//			{
//				/*
//				/// Push UI
//				Graphics.QueueMessage(new GMPushUI("gui/Shop.gui", ui));
//				RuneShop::SetupTestShop();
//				activeShop = &RuneShop::testShop;
//				// Load shop from... test.
//				LoadShop(&RuneShop::testShop);
//				*/
//			}
//			else if (string == "main_menu"){
//				StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MAIN_MENU));
//			}
//			else if (string == "stop_testing"){
//				assert(enterMode == EnterMode::TEST_LEVEL);
//				ReturnToEditor();
//			}
//			else if (string == "OpenMainMenu"){
//				OpenMenu();
//			}
//			else if (string == "begin_commandline"){
//				InputMan.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
//			}
//			else if (string == "interpret_console_Command"){
//				StateMan.ActiveState()->InputProcessor(INTERPRET_CONSOLE_COMMAND);
//				return;
//			}
//			else if (string.Contains("Zone("))
//			{
//				List<String> tokens = string.Tokenize("() ,");
//				if (tokens.Size() < 2){
//					assert(tokens.Size() >= 2);
//					return;
//				}
//				String mapName = tokens[1];
//				ZoneTo(mapName);
//				return;
//			}
//			else if (string.Contains("PlacePlayer(") ||
//				string.Contains("PlacePlayers("))
//			{
//				std::cout<<"\nPlacing player--";
//				List<String> tokens = string.Tokenize("() ,");
//				if (tokens.Size() < 3){
//					assert(tokens.Size() >= 3);
//					return;
//				}
//				int tileX = tokens[1].ParseInt();
//				int tileY = tokens[2].ParseInt();
//				PlacePlayers(Vector2i(tileX, tileY));
//			}
//			else if (string == "TrackPlayer"){
//				TrackPlayer();
//			}
//			else if (string == "MenuClosed"){
//				EnableMovement();
//				return;
//			}
//			else if (string == "OnReloadUI"){
//				if (this->menuOpen){
//					/// Make it visible again.
//					this->OpenMenu();
//				}
//			}
//			else if (string == "DisableMovement")
//			{	
//				DisableMovement();
//				return;
//			}
//			else if (string == "EnableMovement")
//			{
//				EnableMovement();
//				return;
//			}
//			else if (string.Contains("SpawnEntity"))
//			{
//				assert(false);
//				/*
//				std::cout<<"\nSpawning entity--";
//				Model * m = ModelMan.GetModel("Sprite");
//				Texture * t = TexMan.GetTexture("Units/200");
//				Vector3f position;
//				/// Make sure an event triggered this!
//				assert(message->event);
//				Vector3f eventPos = message->event->position;
//				TileMap2D * activeMap = ActiveMap();
//				assert(activeMap);
//				if (!activeMap)
//					return;
//				const Tile * tile = activeMap->GetClosestVacantTile(eventPos);
//				if (!tile){
//				//	assert(tile);
//					std::cout<<"\nERROR: Unable to spawn entity.";
//					return;
//				}
//				assert(tile->position.x != -1);
//				if (tile == NULL){
//					assert(false && "No valid walkable tile in range!");
//					StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EDITOR));
//					return;
//				}
//				position = Vector3i(tile->position);
//				TileMap2D * tmap = (TileMap2D*)MapMan.ActiveMap();
//				if (!tmap->IsTileVacant(position)){
//					std::cout<<"\nERROR: Something already on that tile, or it not walkable, skipping SpawnEntity!";
//				}
//				Entity * entity = MapMan.CreateEntity("Spawns an entity...? wat.", m, t, position);
//				TileMap2D * tMap = (TileMap2D*)MapMan.ActiveMap();
//				if (entity == NULL){
//					std::cout<<"\nERROR: Unable to create entity in MapState!";
//					return;
//				}
//				/// Give it test animation
//				Graphics.QueueMessage(new GMSetEntity(entity, ANIMATION_SET, "Map/Test"));
//				// Set target waypoint to be occupied by this new entity.
//				Physics.QueueMessage(new PMSetWaypoint(position, ENTITY, entity));
//				entity->flags |= SPAWNED_BY_EVENT;
//				String name = string.Tokenize("\"")[1];
//				entity->name = name;
//				/// Create a proper entity state for it based on the EntityStateTile2D
//				entity->properties.Add(new RREntityState(entity));
//				lastModifiedEntity = entity;
//				*/
//			}
//			else if (string.Contains("OnInteract"))
//			{
//				/*
//				if (!lastModifiedEntity){
//					std::cout<<"\nERROR: lastModifiedEntity NULL, cannot set OnInteract";
//					return;
//				}
//				assert(lastModifiedEntity);
//				if (lastModifiedEntity->events == NULL)
//					lastModifiedEntity->events = new ScriptProperty();
//				bool hasInteract = lastModifiedEntity->events->onInteract;
//				assert(hasInteract == false);
//				if (hasInteract == true){
//					std::cout<<"\nERROR: Entity already has OnInteract bound to it!";
//					return;
//				}
//				Script * event = new Script("OnInteract");
//				String source = string.Tokenize(" \t")[1];
//				event->Load(source);
//				lastModifiedEntity->events->onInteract = event;
//				*/
//			}
//		}
//	}
//	AppState::ProcessMessage(message);
//}
//
//// To look at ze player?
//void MapState::ResetCamera()
//{
//	camera->projectionType = Camera::ORTHOGONAL;
//	camera->rotation = Vector3f();
//	camera->zoom = 10.f;
//	camera->farPlane = -50.0f;
//	
//	Entity * mainPlayerEntity = GetMainPlayerEntity();
//	if (mainPlayerEntity)
//		camera->position = mainPlayerEntity->position;
////	camera->SetRatio(Graphics.width, Graphics.height);
//	camera->distanceFromCentreOfMovement = 10.f;
//	// Only update before rendering.
////	camera->Update();
//	/// Reset what parts of the map are rendered too..!
//	TileMap2D * map = (TileMap2D *)MapMan.ActiveMap();
//	if (map)
//		map->SetViewRange(20);
//}
//
//// For when testing..!
//void MapState::ReturnToEditor()
//{
//	MapMan.MakeActive(NULL);
//	StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EDITOR));
//}
//
//// Disables movement for the player!
//void MapState::DisableMovement()
//{
//	RREntityState * playerState = GetMainPlayerState();
//	if (playerState)
//		playerState->DisableMovement();
//}
//void MapState::EnableMovement()
//{
//	RREntityState * playerState = GetMainPlayerState();
//	if (playerState)
//		playerState->EnableMovement();
//}
//
///// New game, load game, testing map, etc.
//void MapState::SetEnterMode(int mode) {
//	enterMode = mode;
//}
//
//void MapState::SetCamera(Camera & reference)
//{
//	/// Just copy the camera data
//	*camera = reference;
//}
//
///// Place player on ze mappur ^3^
//bool MapState::PlacePlayers(Vector3i position)
//{
//	TileMap2D * activeMap = ActiveMap();
//	std::cout<<"\nPlacePlayer: "<<position;
//	assert(activeMap);
//	if (!activeMap)
//		return false;
//	Tile * tile = activeMap->GetClosestVacantTile(position);
//	assert(tile);
//	if (tile == NULL){
//		assert(false && "No valid walkable tile in range!");
//		return false;
//	}
//	position = tile->position;
//
//	// Playerrrsss
//	List<RRPlayer*> players = GetPlayers();
//
//	/// Check if they already exist...
//	Entity * e = MapMan.ActiveMap()->GetEntity("Player");
//	if (e){
//		std::cout<<"\nEntity already exists, moving it.";
//		activeMap->MoveEntity(e, position);
//		Physics.QueueMessage(new PMSetEntity(e, PT_POSITION, e->position));
//		return true;
//	}
//
//	/// New plaYYAHH
//	Model * m = ModelMan.GetModel("Sprite");
//	Texture * t = TexMan.GetTexture("Units/200");
//
//	std::cout<<"\nTile position: "<<tile->position;
//	assert(tile->position.x != -1);
//	
//	/// Creating them from scratch!
//	for (int i = 0; i < players.Size(); ++i)
//	{
//		RRPlayer * player = players[i];
//		Entity * playerEntity = NULL;
//		playerEntity = MapMan.CreateEntity("PlayerEntity: "+player->name, m, t, position);
//		if (playerEntity == NULL)
//		{
//			assert(playerEntity);
//			std::cout<<"\nError placing entity, aborting.";
//			return false;
//		}
//		playerEntity->name = "Player";
//		playerEntity->flags |= PLAYER_OWNED_ENTITY;
//
//		TileMap2D * tMap = (TileMap2D*)MapMan.ActiveMap();
//		RREntityState * playerState = new RREntityState(playerEntity);
//		playerEntity->properties.Add(playerState);
//		/// Give it test animation
//		Graphics.QueueMessage(new GMSetEntity(playerEntity, GT_ANIMATION_SET, "Map/Test"));
//		Physics.QueueMessage(new PMSetEntity(playerEntity, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));
//		Waypoint * wp = WaypointMan.GetClosestVacantWaypoint(position);
//		assert(wp);
//		Physics.QueueMessage(new PMSetWaypoint(wp->position, PT_ENTITY, playerEntity));
//
//		// Re-bind the player
//		player->mapEntity = playerEntity;
//		player->playerState = playerState;
//	}
//	return true;
//}
//
///// Zone to map!
//void MapState::ZoneTo(String mapName)
//{
//	std::cout<<"\n====================== ";
//	std::cout<<"\nZone: "<<mapName;
//	String filename = mapName;
//	TileMap2D * activeMap = ActiveMap();
//	if (activeMap){
//		String mn = activeMap->Name();
//		if (mn == mapName)
//			return;
//	}
//	if (!filename.Contains(MapMan.rootMapDir)){
//		filename = MapMan.rootMapDir + filename;
//	}
//	if (!filename.Contains(".tmap")){
//		filename += ".tmap";
//	}
//	Map * newMap = MapMan.GetMapBySource(filename);
//	if (newMap == NULL){
//		std::cout<<"\nUnable to get map \""<<filename<<"\" to zone to.";
//		return;
//	}
//
//	// If zoning between maps, remove connections to current map before zoning to next one.	
//	if (activeMap)
//	{	
//		// If the player already exists, remove it's connections to any tiles?
//		List<Entity*> playerEntities = GetPlayerEntities();
//		for (int i = 0; i < playerEntities.Size(); ++i)
//		{
//			Entity * playerEntity = playerEntities[i];
//			if (playerEntity)
//			{
//				/// Just remove the player from the map too, without de-allocating stuff?
//				activeMap->RemoveEntity(playerEntity);
//			}
//		}
//	}
//	// Reset variables..
//	stepsTakenAfterZoning = 0;
//
//	newMap->SetName(mapName);
//	bool result = MapMan.MakeActive(newMap);
//	assert(result);
//	activeMap = (TileMap2D*)newMap;
//
//	// Clear pointers to shit on the map.
//	activeMap->ResetTiles();
//
//	/// TODO: Change stuff in the graphics to inform that we are generating a navmesh for the map.
//	// Create navmesh.
//	NavMesh * navMesh = WaypointMan.GetNavMeshByName(newMap->Name());
//	if (!navMesh){
//		navMesh = WaypointMan.CreateNavMesh(newMap->Name());
//	}
//	/// TODO: Make sure that it doesn't already have any data, if so clear it?
//	/// Create waypoints using tiles in the map!
//	TileMap2D * tileMap2D = (TileMap2D*)newMap;
//	// Just delete all.
//	navMesh->Clear();
//	// Generate waypoints based on the existing tiles and stuff.
//	tileMap2D->GenerateWaypoints(navMesh);
//	WaypointMan.MakeActive(navMesh);
//
//	// Remove entities from the navmesh..
//	for (int i = 0; i < navMesh->waypoints.Size(); ++i)
//	{	
//		Waypoint * wp = navMesh->waypoints[i];
//		wp->entities.Clear();
//	}
//
//	// Load populations for this zone.
//	LoadPopulations(mapName);
//	
//	// Load OnEnter script for the zone.
//	Script * script = new Script();
//	script->Load("data/scripts/"+mapName+"/OnEnter.es");
//	script->SetDeleteOnEnd(true);
//	ScriptMan.PlayScript(script);
//
//		/// Query the physics-manager to generate a nav-mesh to be used for our map!
////	Physics.QueueMessage(new PMSet(NAV_MESH, navMesh));
//}
//
//// Load populations for this zone.
//void MapState::LoadPopulations(String forZone)
//{
//	String dir = "data/Populations/"+forZone+"/";
//	List<Population*> populations = PopMan.LoadPopulations(dir);
//	PopMan.MakeActive(populations);
//}
//
///// Bind camera to ze playah.-
//void MapState::TrackPlayer()
//{
//	camera->entityToTrack = GetMainPlayerEntity();
//	camera->trackingMode = TrackingMode::FROM_BEHIND;
//}
