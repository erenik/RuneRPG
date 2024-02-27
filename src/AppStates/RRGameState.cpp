/// Emil Hedemalm
/// 2014-04-19
/** Base class for all game-states belonging to the RuneRPG game.
	Provides pointers and accessing functions to the game session,
	players, etc.
*/

#include "RRGameState.h"
#include "Network/NetworkManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/MessageManager.h"
#include "StateManager.h"
#include "Message/Message.h"
#include "File/LogFile.h"

#include "Maps/MapManager.h"
#include "StateManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMLight.h"
#include "Graphics/Messages/GMUI.h"

#include "Physics/Integrators/FirstPersonIntegrator.h"
#include "Physics/CollisionDetectors/FirstPersonCD.h"
#include "Physics/CollisionResolvers/FirstPersonCR.h"

#include "Entity/EntityProperty.h"
#include "Zone.h"
#include "Graphics/Camera/CameraUtil.h"
#include "RRIntegrator.h"
#include "Physics/Messages/PhysicsMessage.h"

// Static variables.
RRSession * RRGameState::session;
String RRGameState::playerName = DEFAULT_NAME;
Zone zone;

RRGameState::RRGameState()
{
	// Initialization, set pointers to 0.
	session = NULL;
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void RRGameState::OnEnter(AppState * previousState)
{
	QueuePhysics(new PMSet(new RRIntegrator()));

	this->inputMapping.bindings.Add(CreateDefaultCameraBindings());

	/// Remove overlay.
	QueueGraphics(new GMSetOverlay(NULL));
	zone.SetupEditCamera();
	/// Set ambience to 1.0
	QueueGraphics(new GMSetAmbience(Vector3f(1,1,1)));

	QueueGraphics(new GMSetUI((UserInterface*)NULL));
	QueueGraphics(new GMSet(GT_CLEAR_COLOR, Vector3f(0,0,0)));
	QueuePhysics(new PMSet(new FirstPersonCD()));
	FirstPersonCR * cr = new FirstPersonCR();
	cr->inRestThreshold = 0.001f;
	QueuePhysics(new PMSet(cr));

	zone.Load("Player Village");
	/// Create a player?
	zone.playerSpawnPos = Vector3f(21,0,21);
}

/// Main processing function, using provided time since last frame.
void RRGameState::Process(int timeInMs)
{
	Sleep(10);
	zone.Process(timeInMs);
}
/// Function when leaving this state, providing a pointer to the next StateMan.
void RRGameState::OnExit(AppState * nextState)
{

}
/// yea.
void RRGameState::ProcessMessage(Message * message)
{
	zone.ProcessMessage(message);
}


// If not created, create and add the session to be handled by the network-manager.
void RRGameState::CreateSessionIfNeeded()
{
	if (!session){
//		session = new RRSession(playerName);
		// Add the session
//		NetworkMan.AddSession(session);
	}
}

/// For starting it from anywhere.
void RRGameState::StartBattle(String battleRef)
{
//	RuneBattleState.
//	BattleMan.QueueBattle(battleRef);
//	RuneBattleState * battleState = ;
	// Enter battle state.
	StateMan.QueueState(StateMan.GetStateByID(RUNE_GAME_STATE_BATTLE_STATE));
}


/// Hosts a game
bool RRGameState::Host(int port /*= 33010*/)
{
	// If not created, create and add the session to be handled by the network-manager.
	CreateSessionIfNeeded();
	bool success = session->Host(port);
	if (success)
	{
		// Show lobby gui!
		QueueGraphics(new GMPushUI("gui/Lobby.gui"));
		MesMan.QueueMessages("OnPlayersUpdated");
		// Since host, push ui to select if game type: new or load a saved game.
		QueueGraphics(new GMPushUI("gui/GameType.gui"));
	}
	return success;
}

// Stop hosting this game. 
bool RRGameState::CancelGame()
{
	assert(session);
	if (!session)
		return false;
	session->Stop();
	NetworkLog("Game canceled");
	// Remove lobby if it is up.
	QueueGraphics(new GMPopUI("gui/Lobby.gui"));
	return true;
}

bool RRGameState::Join(String ip, int port /*= 33010*/)
{
	/// Start SIP session if needed?
	bool success = NetworkMan.StartSIPServer();
	if (!success)
		return false;
	// If not created, create and add the session to be handled by the network-manager.
	CreateSessionIfNeeded();
	/// Check if already connected.
	if (session->IsConnected())
	{
		NetworkLog("Already connected.");
		// Show lobby?
		QueueGraphics(new GMPushUI("gui/Lobby.gui"));
		MesMan.QueueMessages("OnPlayersUpdated");
		return false;
	}
	/// Connect with our session.
	success = session->ConnectTo(ip, port);
	if (success)
	{
		NetworkLog("Connected successfully.");
		// Show lobby?
		QueueGraphics(new GMPushUI("gui/Lobby.gui"));
		MesMan.QueueMessages("OnPlayersUpdated");
	}
	return success;
}

/// Fetches players from the active session.
List<RRPlayer*> RRGameState::GetPlayers()
{
	return List<RRPlayer*>(); // session->GetPlayers();
}

/// Fetches our main player entity!
Entity * RRGameState::GetMainPlayerEntity()
{
	return 0;
	/*
	List<RRPlayer*> players = session->GetPlayers();
	for (int i = 0; i < players.Size(); ++i)
	{
		RRPlayer * player = players[i];
		return player->mapEntity;
	}
	return NULL;*/
}

RREntityState * RRGameState::GetMainPlayerState()
{
	Entity * mainPlayerEntity = GetMainPlayerEntity();
	RREntityState * rrState = (RREntityState *)mainPlayerEntity->GetProperty("RREntityState");
	return rrState;
}	


/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
void RRGameState::NetworkLog(String message)
{
	// Graphics.QueueMessage(new GMSetUIs("NetworkLog", GMUI::TEXT, message));
}
