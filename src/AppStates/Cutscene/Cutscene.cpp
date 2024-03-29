// Emil Hedemalm
// 2013-06-28

#include "OS/Sleep.h"
#include "Cutscene.h"
#include "AppStates/GeneralManagerIncludes.h"

#include "Message/Message.h"
#include "Graphics/Messages/GMUI.h"
#include "Maps/Grids/TileTypeManager.h"
#include "Maps/Grids/Tile.h"
#include "../Map/MapState.h"
#include "Graphics/Messages/GMSet.h"
#include "../RuneGameStatesEnum.h"
#include "UI/UIQueryDialogue.h"
#include "System/PreferencesManager.h"
#include "Chat/ChatManager.h"
#include "UI/UILists.h"
#include "Script/ScriptManager.h"
#include "Audio/TrackManager.h"
#include "Input/Action.h"

extern UserInterface * ui[GameStateID::MAX_GAME_STATES];

CutsceneState::CutsceneState()
: RRGameState()
{
	this->name = "Cutscene";
	id = GameStateID::GAME_STATE_CUTSCENE;
	requestedPlayers = 1;
	enteredOnce = false;
}

CutsceneState::~CutsceneState()
{
}

void CutsceneState::OnEnter(AppState * previousState)
{
	std::cout<<"\nEntering Cutscene state";

	// TODO: Maybe create some ui to show cut-scenes entirely in this state instead? o.o .. or?
//	QueueGraphics .EnableAllDebugRenders(false);
	paused = false;
	/* 
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	// Pre-load the loadingData texture for future use!
	TexMan.LoadTexture("loadingData.png");
	TexMan.LoadTexture("deallocating.png");

	/// Update options menu labels
	Graphics.QueueMessage(new GMSetUIs("ActivePlayerButton", GMUI::TEXT, "Player 1"));
	Graphics.QueueMessage(new GMSetUIs("ActivePlayerInput", GMUI::TEXT, "Keyboard 1"));

	/// Notify that input-manager to use menu-navigation.
	InputMan.ForceNavigateUI(true);
	// Push the menu to be hovered on straight away, yo.
	Graphics.QueueMessage(new GMPushUI("CutsceneState", ui));

	/// Do first-time loading of preferences, entering user-/player-name, etc.
	if (!enteredOnce)
		OnFirstEnter();	
	*/
};

void CutsceneState::OnExit(AppState *nextState)
{
}

#include "../UI/UserInterface.h"
#include "../Graphics/Messages/GMUI.h"

void CutsceneState::Process(int timeInMs)
{
	SleepThread(25);
#ifdef USE_AUDIO
	AudioMan.Update();
#endif
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void CutsceneState::ProcessMessage(Message * message){
	std::cout<<"\nState::ProcessMessage called:";
	String s = message->msg;
	String msg = message->msg;
	switch(message->type){
		case MessageType::STRING: 
		{
			s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (msg == "PauseCutscene")
			{
				// Stuff!	
				if (paused)
				{
					Resume();
				}
				else 
				{
					Pause();
				}
			}
			else if (msg == "Continue")
			{
				Resume();
			}
			else if (msg == "SkipCutscene")
			{
				// Send the skip-cutscene message to all scripts?
				ScriptMan.SkipCutscene();
				// And resume the scripts also afterwards..!
				Resume();
			}
			break;
		}
	}
}


// Pauses all running scripts.
void CutsceneState::Pause()
{
	// Push the cut-scene menu.
	QueueGraphics(new GMPushUI("CutsceneMenu"));
	scriptsPaused = ScriptMan.PauseAll();
	TrackMan.Pause();
	paused = true;
}
// Resumes all paused scripts.
void CutsceneState::Resume()
{
	// Pop the cut-scene menu.
	QueueGraphics(new GMPopUI("CutsceneMenu", GlobalUI()));
	ScriptMan.ResumeScripts(scriptsPaused);
	TrackMan.Resume();
	paused = false;
}

/// Creates bindings that are used for debugging purposes only
void CutsceneState::CreateDefaultBindings(){
	std::cout<<"\nCutsceneState::CreateDefaultBindings() called";
	/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	InputMapping * mapping = &this->inputMapping;
	mapping->bindings.AddItem(new Binding(Action::FromString("PauseCutscene"), KEY::ESCAPE));
};

/// Creates the user interface for this state
void CutsceneState::CreateUserInterface(){
/*	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("CutsceneState.gui");
*/
}

/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
void CutsceneState::NetworkLog(String message)
{
//	Graphics.QueueMessage(new GMSetUIs("NetworkLog", 
	RRGameState::NetworkLog(message);
}


/// Called once, in order to set player name and maybe notify of updates, etc?
void CutsceneState::OnFirstEnter()
{
}


// Stuff! o.o
void CutsceneState::OnChatMessageReceived(ChatMessage * cm)
{
}

void CutsceneState::InputProcessor(int action, int inputDevice /*= 0*/)
{
	switch(action)
	{
	default:
		break;
	}
}

