/// Emil Hedemalm
/// 2013-12-13
/// Game state for combination of runes and rune-templates! Menu-intensive with some cool visualizations hopefully ;)

#include "RuneState.h"

#include "../RuneGameStatesEnum.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/GraphicsManager.h"
#include "Message/Message.h"
#include "StateManager.h"

RuneState::RuneState(){
	id = RUNE_GAME_STATE_RUNE_STATE;
	name = "RuneState";
	primary = secondary = 0;
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void RuneState::OnEnter(AppState * previousState){
	/// Load Rune UI!
	QueueGraphics(new GMSetUI(ui));
	// Check if we have any existing templates, if not grey that option out!
	bool hasTemplates = false;
	QueueGraphics(new GMSetUIb("ExistingTemplates", GMUI::ENABLED, hasTemplates));
	// Push the RuneScreen to the UI, so that exiting it will return us to the previous state appropriately?
	QueueGraphics(new GMPushUI("RuneScreen"));
}
/// Main processing function, using provided time since last frame.
void RuneState::Process(int timeInMs){
}
/// Function when leaving this state, providing a pointer to the next StateMan.
void RuneState::OnExit(AppState * nextState){

}

void RuneState::ProcessMessage(Message * message){
	switch(message->type){
		case MessageType::STRING: {
			String & msg = message->msg;
			if (msg == "OnReloadUI"){
				// Push the RuneScreen to the UI, so that exiting it will return us to the previous state appropriately?
				QueueGraphics(new GMPushUI("RuneScreen"));
			}
			else if (msg == "AddMajor"){
				++primary;
				UpdateTemplateName();
			}
			else if (msg == "AddMinor"){
				++secondary;
				UpdateTemplateName();
			}
			else if (msg == "RemoveMajor"){
				--primary;
				if (primary < 0)
					primary = 0;
				UpdateTemplateName();
			}
			else if (msg == "RemoveMinor"){
				--secondary;
				if (secondary < 0)
					secondary = 0;
				UpdateTemplateName();
			}
			break;
		};
	}
}

/// Creates the user interface for this state
void RuneState::CreateUserInterface(){
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load(nullptr, "gui/RuneRPG/RuneState.gui");
}


void RuneState::UpdateTemplateName(){
	String templateName = String::ToString(primary) + "P";
	if (secondary)
		templateName += String::ToString(secondary) + "S";
	QueueGraphics(new GMSetUIs("TemplateName", UITarget::Text, templateName));
}