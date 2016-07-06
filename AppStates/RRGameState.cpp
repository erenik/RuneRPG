/// Emil Hedemalm
/// 2014-04-19
/** Base class for all game-states belonging to the RuneRPG game.
	Provides pointers and accessing functions to the game session,
	players, etc.
*/

#include "RRGameState.h"
#include "Network/NetworkManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/GraphicsManager.h"
#include "Message/MessageManager.h"
#include "StateManager.h"

// Static variables.
RRSession * RRGameState::session;
String RRGameState::playerName = DEFAULT_NAME;

RRGameState::RRGameState()
{
	// Initialization, set pointers to 0.
	session = NULL;
}

#include "XML/XMLParser.h"
#include "XML/XMLElement.h"

#include "Maps/MapManager.h"
#include "StateManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

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

void LoadMap()
{
	styles.Clear();
	XMLParser parser;
	parser.Read("data/Zones/East Road.html");
	bool ok = parser.Parse();
	XMLElement * e = parser.GetElement("style");
	assert(e && "No style element?");
	String style = e->data;
	int place = 0;
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
	for (int i = 0; i < styles.Size(); ++i)
	{
		styles[i].Print();
	}

	/// Parse the table.
	XMLElement * body = parser.GetElement("tbody"); // Body
	for (int i = 0; i < body->children.Size(); ++i) // Rows
	{
		XMLElement * row = body->children[i];
		for (int j = 0; j < row->children.Size(); ++j) // Cell in row (column)
		{
			XMLElement * xe = row->children[j];
			if (xe->name != "td")
				continue; // Skip header-skills in the edges.
			assert(xe->name == "td");
			if (xe->args.Size() == 0)
				continue; // Skip data without argument -> no relevant info.
			XMLArgument * arg = xe->GetArgument("class");
			int st = (arg->value - "s").ParseInt();
			Style * s = GetStyle(st);
			if (s->hexColor != "#b7b7b7")
				continue;
			// Do different stuff depending on the style?
			MapMan.CreateEntity("ij", ModelMan.GetModel("cube"), TexMan.GetTexture("0xFF"), Vector3f(i, 0, j));
		}
	}
}

#include "Graphics/Camera/CameraUtil.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMCamera.h"

/// Function when entering this state, providing a pointer to the previous StateMan.
void RRGameState::OnEnter(AppState * previousState)
{

	this->inputMapping.bindings.Add(CreateDefaultCameraBindings());

	/// Remove overlay.
	QueueGraphics(new GMSetOverlay(NULL));
	LoadMap();
	Camera * cam = CameraMan.DefaultCamera();
	cam->trackingMode = TrackingMode::NONE;
	QueueGraphics(new GMSetCamera(cam));
}




/// Main processing function, using provided time since last frame.
void RRGameState::Process(int timeInMs)
{

}
/// Function when leaving this state, providing a pointer to the next StateMan.
void RRGameState::OnExit(AppState * nextState)
{

}
#include "Message/Message.h"
/// yea.
void RRGameState::ProcessMessage(Message * message)
{
	String msg = message->msg;
	ProcessCameraMessages(msg, CameraMan.ActiveCamera());
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
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui", GetUI()));
		MesMan.QueueMessages("OnPlayersUpdated");
		// Since host, push ui to select if game type: new or load a saved game.
		Graphics.QueueMessage(new GMPushUI("gui/GameType.gui", GetUI()));
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
	Graphics.QueueMessage(new GMPopUI("gui/Lobby.gui", GetUI(), true));
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
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui", GetUI()));
		MesMan.QueueMessages("OnPlayersUpdated");
		return false;
	}
	/// Connect with our session.
	success = session->ConnectTo(ip, port);
	if (success)
	{
		NetworkLog("Connected successfully.");
		// Show lobby?
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui", GetUI()));
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
	Graphics.QueueMessage(new GMSetUIs("NetworkLog", GMUI::TEXT, message));
}
