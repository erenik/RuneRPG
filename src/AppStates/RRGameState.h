/// Emil Hedemalm
/// 2014-04-19
/** Base class for all game-states belonging to the RuneRPG game.
	Provides pointers and accessing functions to the game session,
	players, etc.
*/

#ifndef RR_GAMESTATE_H
#define RR_GAMESTATE_H

#include "AppStates/AppState.h"
#include "Network/RRSession.h"
#include "AppStates/RuneGameStatesEnum.h"

class RREntityState;

#define DEFAULT_NAME "No-name"

class RRGameState : public AppState 
{
public:
	RRGameState();

	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	virtual void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(AppState * nextState);
	/// yea.
	virtual void ProcessMessage(Message * message);

	/// Hosts a game
	bool Host(int port = RR_DEFAULT_PORT);
	// Stop hosting this game. 
	bool CancelGame();
	bool Join(String ip, int port = RR_DEFAULT_PORT);

	/// Fetches players from the active session.
	static List<RRPlayer*> GetPlayers();
	/// Fetches our main player entity!
	Entity * GetMainPlayerEntity();
	RREntityState * GetMainPlayerState();
	/// The main-session.
	static RRSession * session;
protected:
	// If not created, create and add the session to be handled by the network-manager.
	void CreateSessionIfNeeded();
	
	/// For starting it from anywhere.
	void StartBattle(String battleRef);


	// Name our player on this device.
	static String playerName;
	/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
	virtual void NetworkLog(String message);
};

#endif