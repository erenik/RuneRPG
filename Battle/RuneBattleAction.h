/// Emil Hedemalm
/// 2014-08-26
/// Class for any action performed by the battlers in the battles!

#ifndef RUNEBATTLEACTION_H
#define RUNEBATTLEACTION_H

#include "String/AEString.h"
#include "System/DataTypes.h"
#include "BattleEffect.h"
#include "RuneBattle.h"
#include "TargetFilters.h"
#include "BattleAnimation.h"

class RuneBattler;
class RuneBattleAction;

class RuneBattleActionCategory 
{
public:
	RuneBattleActionCategory(String withName = "");
	String name;
	/// Actions available in this category.
	List<RuneBattleAction*> actions;
	/// If the category is an action, this will provide the pointer to it.
	RuneBattleAction * isAction;
};


typedef RuneBattleAction RBA;

class RuneBattleAction
{
public:

    RuneBattleAction();
    /// Sets relevant vars and pointers to 0/NULL upon creation.
	void Nullify();
	virtual ~RuneBattleAction();

	/// Loads from a single text file.. kinda obsolete?
    virtual bool Load(String fromFile);
	/// wat. this really needed anymore?
	/*
    virtual void OnBegin(BattleState & battleState);
    virtual void OnEnd(BattleState & battleState);
	*/

	/// First calling of the action when adding it to the list of active battle actions for a battler.
    virtual void OnBegin(RBattleState & battleState);
	/// Should return true once the action (including animation, sound etc.) has been finished.
    virtual void Process(RBattleState & battleState);
    
	String name;
	String id;
	String source;
	
	enum {
		MUNDANE_ACTION,
		MAGIC_SKILL,
		MAGIC_SPELL,
		USE_ITEM,
	};
	/// Mundane action, magic skill or magic spell. See enum above.
	int type;
	/// See TargetFilter namespace-enum above.
	int targetFilter;

	/// Battler targets for this specific instance of the action.
    List<RuneBattler*> subjects, targets;

	/// From 1 to 100. Used for all magical effects.
	int spellPower;

	String description;
	String keywords;

	int manaCost;
	int actionCost;

	/// Time in seconds that the participant is frozen for. This time starts when the animation starts.
	float freezeTimeInSeconds;
	/// The time in seconds before the action or spell's effect begins. Interruptions may occur before this time has ended.
	float castTimeInSeconds;

	/// Checks for a name and stuff.
	bool IsValid();
	
	/// Parses a string with targets as defined in a CSV file. E.g. "Ally Select 1"
	void ParseTargets(String fromString);
	void ParseElements(String fromString);
	/// Returns false if any errors occured while parsing. Errors may not always be fatal but could be highly relevant.
	bool ParseDurations(String fromString);
	void ParseEffects(String fromString);
	void ParseOtherCosts(String fromString);

	void SetElements(String toParse);

	/// Will depend on the filter.
	bool HasValidTargets();

	/// Which elements are associated with this spell.
	List<int> elements;
	
	List<BattleEffect> effects;


	/// When true, spell cast time has elapsed.
	bool casted;
	/// When true, the player is free to start casting his next spells or perform whatever action is queued up.
	bool freezeTimeOver;
	/// When true, remove the spell from the list of active actions and delete it.
	bool finished;

	/// Starts at 0, increments with the milliseconds as specified in the BattleState when Process is called.
	int millisecondsProcessed;
	/// Starts at 0, starts increasing after the animations begin when the casting time finishes.
	int millisAnimated;

	RuneBattler * primarySubject;
	RuneBattler * primaryTarget;

	/// o.o;
	List<BattleAnimation> animationSequences;

protected:

	/// Applies all effects this spells should apply. If needed to re-apply multiple effects, this function should be called again until it returns true.
	bool ApplyEffects(RBattleState & bs);

	/// Set to true once all effects have been applied.
	bool allEffectsApplied;

	void EvaluateLine(String line);
	void PhysicalDamage(String line);
	void MagicDamage(String line);
	void Damage(String line);

private:
	/// Send to battle narrator.
	void Narrate(String line);

	/// Set during processing.
	String narr;
	bool died;
	// Start time of the action?
    int64 startTime;
};

#endif // RUNEBATTLEACTION_H
