// Emil Hedemalm
// 2013-06-28

#include "StateManager.h"
#include "AppStates.h"

// Registers all states that will be used to the state manager upon startup
void RegisterStates(){
	RRGameState * rrState = new RRGameState();
	StateMan.RegisterState(rrState);
	StateMan.QueueState(rrState);
	//StateMan.RegisterState(new RuneGlobalState());
	//StateMan.RegisterState(new MainMenu());
	//StateMan.RegisterState(new RuneBattleState());
	auto mapState = new MapState();
	StateMan.RegisterState(mapState);
	//StateMan.RegisterState(new RuneEditor());
	//StateMan.RegisterState(new RuneState());
	//StateMan.RegisterState(new CutsceneState());


	StateMan.QueueState(mapState);
};