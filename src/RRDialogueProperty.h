/// Emil Hedemalm
/// 2016-08-03
/// Dialogue property for entities in the RuneRPG game.

#ifndef RR_DIALOG_PROP
#define RR_DIALOG_PROP

#include "Properties.h"

class Prerequisite {};

//#include "Prerequisite.h"
//#include "String/AEString.h"

class DialoguePart 
{
public:
	enum {
		RESPONSE,
		OPTION, // for player
	};
	/* 	Text to be presented and run. This may be divided into several
		Dialogue boxes, and also run some commands.
	
		Commands are in arrow brackets < > and include:
			<Receive itemID>
			<QuestVar newInt>
	*/
	String text; 
	
	void Parse();
	/// Returns 1/True if done, 0/False if not.
	int Continue();
	
	/// Parts of text not presented yet. Depends on parse and previous continues.
	String textQueued;
	/// The optional commands. Filled after parsing.
	List<String> options;
	int type;
};

class Dialogue 
{
public:
	Dialogue(){ id = idEnumerator++; state = NOT_STARTED; activePart = 0; owner = 0; };
	virtual ~Dialogue();
	/// Checks prereqs.
	bool CanRun();
	// Starts. Returns dialogue state.
	int Start();
	/// Called to continue the dialogue (usually an Enter press or mouse click). Returns dialogue state.
	int Continue();
	/// Called when selecting a choice in the dialogue. Returns dialogue state.
	int Choose(String choice);
	/// Getter
	int ID(){return id;};
	void End();
	
	enum {
		NOT_STARTED,
		BEGUN,
		ENDED,
	};
	int state;
	DialoguePart * activePart;
	List<Prerequisite> prereqs;
	List<DialoguePart *> parts;
	Entity * owner;

	static Dialogue * ActiveDialogue();
private:
	static Dialogue * activeDialogue; // Yup.
	static int idEnumerator;
	int id;
};

/* Reacts to the following messages:
	Interact - sent to all RuneEntity's in front of the player.
	Continue
	Choose(choice)
	Cancel

May issue the following messages to the MesMan:
	OnDialogueBegun(dialogueID)
	OnDialogueEnded(dialogueID)
	ReceiveItem(itemID)
	SetQuestVar(varName, int)
	OpenShop(shopName)
*/
class RRDialogueProperty : public EntityProperty 
{
public:
	RRDialogueProperty(Entity * owner);
	virtual ~RRDialogueProperty();
	void InitDefaultGreeting();
	// 
	void ProcessMessage(Message * message);
		
	void OnDialogueEnded();
	static int ID() {return RR_DIALOGUE_PROPERTY; }; 
	/// Greeting, <VHello> randomed every time.
	/// Option/Answer,
	/// Shop,
	Dialogue * activeDialogue;
	bool alternate; // If true, alternates which dialogue to present each interaction.
	List<int> preferredDialogues; /// Contains IDs of dialogues to preferably present first. The latest added ID will have highest priority.
	List<Dialogue*> dialogues;

};

#endif