/// Emil Hedemalm
/// 2016-08-03
/// Dialogue property for entities in the RuneRPG game.

#include "RRDialogueProperty.h"
#include "Message/Message.h"
#include "UI/UIElement.h"
#include "StateManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/MessageManager.h"

int Dialogue::idEnumerator = 0;
UIElement * box = 0;
#define DIALOGUE_BOX "DialogueBox"

// Open dialogue UI main box.
void DisplayDialogueBox()
{
	if (box == 0)
	{
		box = new UILabel(DIALOGUE_BOX);
		box->sizeRatioX = 1;
		box->sizeRatioY = 0.2f;
		box->alignmentY = box->sizeRatioY * 0.5f;
		box->text.color = Color(255,255,255,255);
		box->removeOnPop = true;
	}
//	box = UserInterface::LoadUIAsElement("gui/DialogueBox.gui");
	// Might be enough?
	QueueGraphics(new GMPushUI(box));
}
void HideDialogueBox()
{
	QueueGraphics(new GMPopUI(DIALOGUE_BOX));
	box = 0;
}

void SetDialogueText(String str)
{
	QueueGraphics(new GMSetUIs(DIALOGUE_BOX, GMUI::TEXT, str));
}

/// DialoguePart -------------------------
void DialoguePart::Parse()
{
	// Do stuff.
	std::cout<<"\nTODOLODO";
	textQueued = text;
}

/// Returns 1/True if done, 0/False if not.
int DialoguePart::Continue()
{
	/// Take some text.
	String presentationText = textQueued;	
	/// Check length.
	// Present it.
	SetDialogueText(presentationText);
	textQueued = "";
	return 1;
}

/// Dialogue ----------------------
Dialogue::~Dialogue()
{
	parts.ClearAndDelete();
}

/// Checks prereqs.
bool Dialogue::CanRun()
{
	return true;
}

// Starts. Returns dialogue state.
int Dialogue::Start()
{
	if (activeDialogue)
		return state;
	if (!CanRun())
		return state;
	activeDialogue = this;
	/// Check prerequisites?
	DisplayDialogueBox();
	/// Get next part?
	activePart = parts[0];
	activePart->Parse(); // Parse contents of inital text.
	activePart->Continue(); // Present first text part.
	state = BEGUN;
	return state;
}
/// Called to continue the dialogue (usually an Enter press or mouse click). Returns dialogue state.
int Dialogue::Continue()
{
	if (!activePart)
	{
		End();
		return state;
	}
	if (activePart->Continue())
	{
		// Queue next one?
		int index = parts.GetIndexOf(activePart) + 1;
		if (index >= parts.Size())
			End();
		else
			activePart = parts[index + 1];
	}
	return state;
}

/// Called when selecting a choice in the dialogue. Returns dialogue state.
int Dialogue::Choose(String choice)
{
	return state;
}

void Dialogue::End()
{
	activeDialogue = 0;
	state = ENDED;
	HideDialogueBox();
	Message msg("OnDialogueEnded");
	owner->ProcessMessage(&msg);
}

Dialogue * Dialogue::activeDialogue = 0;
Dialogue * Dialogue::ActiveDialogue()
{
	return activeDialogue;
}


RRDialogueProperty::RRDialogueProperty(Entity * owner)
: EntityProperty("RRDialogueProperty", ID(), owner)
{	
	alternate = true;
	activeDialogue = 0;
}
RRDialogueProperty::~RRDialogueProperty()
{
	dialogues.ClearAndDelete();
}

void RRDialogueProperty::InitDefaultGreeting()
{
	Dialogue * d = new Dialogue();
	dialogues.AddItem(d);
	DialoguePart * dp = new DialoguePart();
	dp->text = "Hello!";
	d->parts.AddItem(dp);
}

// 
void RRDialogueProperty::ProcessMessage(Message * message)
{
	/* 	Relevant messages.
			Interact - sent to all RuneEntity's in front of the player.
			Continue
			Choose(choice)
			Cancel
*/
	String msg = message->msg;
	if (msg == "Interact")
	{
		activeDialogue = dialogues.Size()? dialogues[0] : 0;
		if (activeDialogue)
		{
			activeDialogue->Start();
			activeDialogue->owner = owner;
			if (activeDialogue->state == Dialogue::BEGUN)
			{
				/// Processes it straight away, no queueing.
				MesMan.ProcessMessage("OnDialogueBegun("+String(activeDialogue->ID())+")");
				/// Pause movement of this NPC or whatever.
				Message msg("PauseMovement");
				owner->ProcessMessage(&msg);
			}
		}
	}
	else if (msg == "Continue")
	{
		if (activeDialogue == 0)
		{
			MesMan.ProcessMessage("OnDialogueBegun("+String(activeDialogue->ID())+")");
			return;
		}
		activeDialogue->Continue();
		if (activeDialogue->state == Dialogue::ENDED)
		{
			OnDialogueEnded();
			return;
		}
	}
	else if (msg.Contains("Choose")){}
	else if (msg == "OnDialogueEnded")
	{
		activeDialogue = 0;
		OnDialogueEnded();
	}
	else if (msg == "Cancel")
	{
		
	}
}

void RRDialogueProperty::OnDialogueEnded()
{
	Message msg("ResumeMovement");
	owner->ProcessMessage(&msg);
	MesMan.ProcessMessage("OnDialogueEnded");
}