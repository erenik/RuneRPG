//// Emil Hedemalm
//// 2013-06-28
//
//#include "OS/Sleep.h"
//#include "MainMenu.h"
//#include "AppStates/GeneralManagerIncludes.h"
//
//#include "Message/Message.h"
//#include "Actions.h"
//#include "Graphics/Messages/GMUI.h"
//#include "Maps/Grids/TileTypeManager.h"
//#include "Maps/Grids/Tile.h"
//#include "../Map/MapState.h"
//#include "Graphics/Messages/GMSet.h"
//extern UserInterface * ui[GameStateID::MAX_GAME_STATES];
//#include "../RuneGameStatesEnum.h"
//#include "UI/UIQueryDialogue.h"
//#include "System/PreferencesManager.h"
//#include "Chat/ChatManager.h"
//#include "UI/UILists.h"
//#include "Window/AppWindowManager.h"
//
//MainMenu::MainMenu()
//{
//	id = GameStateID::GAME_STATE_MAIN_MENU;
//	requestedPlayers = 1;
//
//
//	enteredOnce = false;
//}
//
//MainMenu::~MainMenu()
//{
//}
//
//void MainMenu::OnEnter(AppState * previousState)
//{
//	std::cout<<"\nEntering Main Menu state";
//
//	// Load tile-types! Maybe do this only once?
//	TileTypes.LoadTileTypes("data/tiles.txt");
//
//	if (!ui)
//		CreateUserInterface();
//
////	Graphics.EnableAllDebugRenders(false);
//
//	// Begin loading textures here for the UI
//	Graphics.QueueMessage(new GMSetUI(ui));
//
//	// Set graphics manager to render UI, and remove the overlay-texture.
//	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));
//
//	// Pre-load the loadingData texture for future use!
//	TexMan.LoadTexture("loadingData.png");
//	TexMan.LoadTexture("deallocating.png");
//
//	/// Update options menu labels
//	Graphics.QueueMessage(new GMSetUIs("ActivePlayerButton", GMUI::TEXT, "Player 1"));
//	Graphics.QueueMessage(new GMSetUIs("ActivePlayerInput", GMUI::TEXT, "Keyboard 1"));
//
//	/// Notify that input-manager to use menu-navigation.
//	InputMan.ForceNavigateUI(true);
//	// Push the menu to be hovered on straight away, yo.
//	Graphics.QueueMessage(new GMPushUI("MainMenu", ui));
//
//	/// Do first-time loading of preferences, entering user-/player-name, etc.
//	if (!enteredOnce)
//		OnFirstEnter();
//
//	AppWindow * mainWindow = MainWindow();
//	float intensity = 0.1f;
//	mainWindow->backgroundColor = Vector4f(intensity,intensity,intensity,1);
//	mainWindow->renderState = false;
//
//};
//
//void MainMenu::OnExit(AppState *nextState)
//{
//	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
//	std::cout<<"\nLeaving MainMenu state.";
//	// Load initial texture and set it to render over everything else
//	if (nextState->GetID() != GameStateID::GAME_STATE_EXITING)
//		;// Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTexture("img/loadingData.png")));
//	else
//		Graphics.QueueMessage(new GMSet(GT_OVERLAY_TEXTURE, TexMan.GetTexture("img/deallocating.png")));
//}
//
//#include "../UI/UserInterface.h"
//#include "../Graphics/Messages/GMUI.h"
//
//void MainMenu::Process(int timeInMs)
//{
//	SleepThread(25);
//#ifdef USE_AUDIO
//	AudioMan.Update();
//#endif
//}
//
///// Callback function that will be triggered via the MessageManager when messages are processed.
//void MainMenu::ProcessMessage(Message * message){
//	std::cout<<"\nState::ProcessMessage called:";
//	String s = message->msg;
//	String msg = message->msg;
//	switch(message->type){
//		case MessageType::SET_STRING: 
//		{
//			SetStringMessage * ssm = (SetStringMessage*)message;
//			if (msg == "SetPlayerName")
//			{
//				playerName = ssm->value;
//				std::cout<<"\nName set to: "<<playerName;
//				Preferences.SetString("PlayerName", playerName);
//				// Add name to preferences if not there already.
//				// Save preferences.
//				bool result = Preferences.Save();
//				assert(result && "Unable to save preferences?");
//			}
//		}
//		case MessageType::STRING: {
//			s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
//			if (msg == "ChatInput")
//			{
//				// Stuff.
//			}
//			else if (msg == "GoToBattleState")
//			{
//				this->StartBattle("testBattle.battle");
//			}
//			else if (msg == "Ready")
//			{
//				// Set ready.
//				session->ToggleReady();
//			}
//			else if (msg == "PeerReadyStatesUpdated")
//			{
//				OnPlayerReadyStateUpdated();
//			}
//			else if (msg == "OnPlayersUpdated")
//			{
//				OnPlayersUpdated();
//			}
//			else if (s == "NEW_GAME")
//			{
//				CreateSessionIfNeeded();
//				// Start the game!
//				if (!session->IsConnected())
//				{
//					session->HostLocalGame();
//					session->NewGame();
//					session->StartGame();
//				}
//			}
//			else if (s == "go_to_main_menu")
//				StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MAIN_MENU));
//			else if (s == "go_to_editor")
//				StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EDITOR));
//			else if (s == "go_to_options")
//				Graphics.QueueMessage(new GMSet(GT_ACTIVE_USER_INTERFACE, (void*)NULL));
//			else if (s == "exit")
//				StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EXITING));
//			else if (s == "begin_input(this)"){
//				UserInterface * ui = StateMan.ActiveState()->GetUI();
//				UIElement * element = ui->GetActiveElement();
//				if (element != NULL){
//					assert(element->onTrigger);
//					InputMan.EnterTextInputMode(element->onTrigger);
//					if (element->text)
//						InputMan.SetInputBuffer(element->text.c_str());
//				}
//				else
//					assert(false && "NULL-element :<");
//			}
//			else {
//				std::cout<<"\nUndefined message received: "<<message->msg;
//			}
//			break;
//		}
//	}
//}
//
//#include "Input/Action.h"
///// Creates bindings that are used for debugging purposes only
//void MainMenu::CreateDefaultBindings()
//{
//	std::cout<<"\nMainMenu::CreateDefaultBindings() called";
//	/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
//	InputMapping * mapping = &this->inputMapping;
//	List<Binding*> & bindings = mapping->bindings;
//	//bindings.AddItem(new Binding(Action::FromEnum(GO_TO_AI_TEST, List<int>(KEY::CTRL, KEY::G, KEY::A)));
//	//mapping->CreateBinding(GO_TO_RACING_STATE, KEY::CTRL, KEY::G, KEY::R);
//	//mapping->CreateBinding(GO_TO_NETWORK_TEST, KEY::CTRL, KEY::G, KEY::N);
//	//mapping->CreateBinding(GO_TO_BLUEPRINT_EDITOR, KEY::CTRL, KEY::G, KEY::S);
//
//	//mapping->CreateBinding("GoToBattleState", KEY::CTRL, KEY::G, KEY::B);
//
//	///// Menu navigation, yush!
//	//mapping->CreateBinding(PREVIOUS_UI_ELEMENT, KEY::SHIFT, KEY::TAB);
//	//mapping->CreateBinding(NEXT_UI_ELEMENT, KEY::TAB);
//	//mapping->CreateBinding(ACTIVATE_UI_ELEMENT, KEY::ENTER);
//};
//
///// Creates the user interface for this state
//void MainMenu::CreateUserInterface(){
//	if (ui)
//		delete ui;
//	ui = new UserInterface();
//	ui->Load("MainMenu.gui");
//}
//
///// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
//void MainMenu::NetworkLog(String message)
//{
////	Graphics.QueueMessage(new GMSetUIs("NetworkLog", 
//	RRGameState::NetworkLog(message);
//}
//
//
///// Called once, in order to set player name and maybe notify of updates, etc?
//void MainMenu::OnFirstEnter()
//{
//	// Load preferences if not already done so!
//	Preferences.Load();
//	// Try to load name from preferences.
//	Preferences.GetString("PlayerName", &playerName);
//
//	/// Check name, if default query for a good name to save to preferences.
//	if (playerName == DEFAULT_NAME)
//	{
//		UIStringDialogue * nameDialogue = new UIStringDialogue("Enter your name", "SetPlayerName", "Name will be saved in preferences file locally and used as placeholder-name in new games.", playerName);
//		nameDialogue->textureSource = "Black";	
//		nameDialogue->CreateChildren();
//		Graphics.QueueMessage(new GMAddUI(nameDialogue));
//		Graphics.QueueMessage(new GMPushUI(nameDialogue, ui));
//	}
//	this->enteredOnce = true;
//}
//
//
//// Stuff! o.o
//void MainMenu::OnChatMessageReceived(ChatMessage * cm)
//{
//	/// Update log.
//	// Clear it.
//	Graphics.QueueMessage(new GMClearUI("ChatLog"));
//	// Fetch chat and push it to the lobby log.
//	List<ChatMessage*> cms = ChatMan.GetLastMessages(10);
//	for (int i = 0; i < cms.Size(); ++i)
//	{
//		ChatMessage * cm = cms[i];
//		UILabel * label = new UILabel("ChatMessage"+String::ToString(i));
//		if (cm->from)
//		{
//			label->text = cm->playerName + ": "+cm->text;
//		}
//		else 
//		{
//			label->text = cm->text;
//			label->text.color = Vector3f(1,1,0.5);
//		}
//		label->sizeRatioY = 0.1f;
//		Graphics.QueueMessage(new GMAddUI(label, "ChatLog"));
//	}
//}
//
//// For updating lobby-gui.
//void MainMenu::OnPlayersUpdated()
//{
//	Graphics.QueueMessage(new GMClearUI("PlayerList"));
//
//	List<RRPlayer*> players = GetPlayers();
//	for (int i = 0; i < players.Size(); ++i)
//	{
//		RRPlayer * player = players[i];
//		UIColumnList * clPlayer = new UIColumnList();
//		clPlayer->sizeRatioY = 0.25f;
//
//		UILabel * label = new UILabel(player->name);
//		label->sizeRatioX = 0.25f;
//		clPlayer->AddChild(label);
//
//		UILabel * readyLabel = new UILabel(player->name+"Ready");
//		readyLabel->sizeRatioX = 0.25f;
//		readyLabel->text = "";
//		readyLabel->text.color = Vector3f(0.5f, 0.6f, 1.0f);
//		if (player->isReady)
//			readyLabel->text = "Ready";
//		clPlayer->AddChild(readyLabel);
//
//		Graphics.QueueMessage(new GMAddUI(clPlayer, "PlayerList"));
//	}
//}
//
//// More gui.
//void MainMenu::OnPlayerReadyStateUpdated()
//{
//	List<RRPlayer*> players = GetPlayers();
//	for (int i = 0; i < players.Size(); ++i)
//	{
//		RRPlayer * player = players[i];
//		String readyText;
//		if (player->isReady)
//			readyText = "Ready";
//		Graphics.QueueMessage(new GMSetUIs(player->name+"Ready", GMUI::TEXT, readyText));
//	}
//}