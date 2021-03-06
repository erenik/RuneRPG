// Emil Hedemalm
// 2014-04-06
// Global app/game-state for the RuneRPG game.

#ifndef RUNE_GLOBAL_STATE_H
#define RUNE_GLOBAL_STATE_H

#include "../RRGameState.h"
#include "Entity/Entities.h"

#define TEST_BATTLE_PATH "data/RuneRPG/testBattle.txt"
#define BATTLERS_DIRECTORY "data/RuneRPG/Battlers/"
#define BATTLES_DIRECTORY	"data/RuneRPG/Battles/"
#define ACTIONS_DIRECTORY   "data/RuneRPG/BattleActions/"

class RuneGlobalState : public RRGameState {
public:
	RuneGlobalState();
	void OnEnter(AppState * previousState);
	void Process(int timeInMs);
	void OnExit(AppState * nextState);

	void ProcessPacket(Packet * packet);
	void ProcessMessage(Message * message);

	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);

	/// Input functions for the various states
	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	virtual void MouseMove(int x, int y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	void MouseWheel(float delta);
	/// Interpret selection queries for additional processing (updating UI for example).
	void OnSelect(Entities &selection);

	/// Increases playback speed and notifies relevant systems of the change
	void IncreaseSpeed();
	/// Decreases playback speed and notifies relevant systems of the change
	void DecreaseSpeed();
private:
	/// For host/join.
	int targetPort;
	String targetIP;
};

#endif
