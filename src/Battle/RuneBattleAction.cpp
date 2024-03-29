/// Emil Hedemalm
/// 2013-10-10

#include "RuneBattleAction.h"

#include "RuneBattler.h"
#include "BattleStats.h"
#include "BattleAnimation.h"

#include "Timer/Timer.h"

#include "Elements.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMAnimate.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"
#include <sstream>

#include "File/File.h"
#include "Random/Random.h"
#include "StateManager.h"

Random random;

RuneBattleActionCategory::RuneBattleActionCategory(String withName)
: name(withName)
{
	isAction = NULL;
};

RuneBattleAction::RuneBattleAction() 
{
	/// 2 seconds duration default.
	Nullify();
}

void RuneBattleAction::Nullify()
{
	primaryTarget = NULL;
	primarySubject = NULL;

	/// Default values..
	actionCost = 10;
	manaCost = 0;

	spellPower = 0;
	casted = false;
	freezeTimeOver = false;
	finished = false;
	allEffectsApplied = false;

	/// Default is 1 enemy, okay?
	targetFilter = TargetFilter::ENEMY;

	millisecondsProcessed = 0;
	millisAnimated = 0;
}


RuneBattleAction::~RuneBattleAction()
{
    //dtor
}

/// Based on the one in BattleAction?
bool RuneBattleAction::Load(String fromFile)
{
	assert(false);
	/*
	/// First load basic stuff.
	bool ok = BattleAction::Load(fromFile);
	/// Then load specific stuff for runeRPG in it.
	List<String> lines = File::GetLines(fromFile);
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.Contains("//"))
			continue;
		List<String> tokens = line.Tokenize(" \t");
		if (tokens.Size() < 2)
			continue;
		String key = tokens[0], value = tokens[1];
		key.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (key.Contains("SpellPower"))
			this->spellPower = value.ParseFloat();
	}
	*/
	bool ok = false;
	return ok;
}

/// First calling of the action when adding it to the list of active battle actions for a battler.
void RuneBattleAction::OnBegin(RBattleState & battleState)
{
	/// Do stuff..
}

/// Should return true once the action (including animation, sound etc.) has been finished.
void RuneBattleAction::Process(RBattleState & battleState)
{
	assert(primarySubject);
	int timeInMs = battleState.timeInMs;
	millisecondsProcessed += battleState.timeInMs;
	/// Wait for cast-time...
	int castTimeMillis = castTimeInSeconds * 1000;
	if (!casted && millisecondsProcessed > castTimeMillis)
	{
		battleState.log = primarySubject->name+" casts "+this->name+".";
		casted = true;
		// Start animation.
		// Show effects and damage numbers or do that mid-animation?

		/// Reduce the milliseconds passed to continue and evaluate the freeze-time next.
		millisecondsProcessed -= castTimeMillis;
	}

	int freezeTimeMillis = freezeTimeInSeconds * 1000;
	if (!freezeTimeOver && millisecondsProcessed > freezeTimeMillis)
	{
		freezeTimeOver = true;
	}
	if (casted)
		millisAnimated += timeInMs;
	if (!allEffectsApplied && millisAnimated > 200)
	{
		/// Apply effects!
		allEffectsApplied = ApplyEffects(battleState);
	}

	/// Alreayd finished?
	finished = true;
	for (int i = 0; i < animationSequences.Size(); ++i)
	{
		BattleAnimation & anim = animationSequences[i];
		anim.triggererAction = this;
		/// Process all animations.
		anim.Process(battleState.timeInMs);
		if (!anim.isOver)
		{
			finished = false;
		}
	}
	/// Wait until all animations are over and the freeze time has elapsed.
	if (finished && freezeTimeOver && allEffectsApplied)
		finished = true;
	else 
		finished = false;
}


void RuneBattleAction::EvaluateLine(String line)
{
	assert(false && "Deprecated for the time being");
	line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (line.Contains("//"))
		return;

	String s = line;
	if (s.Contains("QueueAnimation("))
	{
		String animation = s.Tokenize("()")[1];
		for (int i = 0; i < subjects.Size(); ++i)
		{
			RuneBattler * subject = subjects[i];
			Entity * entity = subject->entity;
			if (!entity)
				return;
			QueueGraphics(new GMQueueAnimation(animation, entity));
		}
	}
	else if (s.Contains("PhysicalDamage("))
	{
		PhysicalDamage(line);
	}
	else if (s.Contains("MagicDamage("))
	{
		MagicDamage(line);
	}
    else if (s.Contains("Damage("))
	{
         }
    else if (s.Contains("Initiative(") ||
		s.Contains("ActionPoints("))
	{
        String s2 = s.Tokenize("()")[1];
        int v = s2.ParseInt();

        for (int p = 0; p < subjects.Size(); ++p){
//            subjects[p]->initiative += v;
			subjects[p]->actionPoints -= v;
        }
    }
}


void RuneBattleAction::PhysicalDamage(String line)
{
	assert(false);
}



void RuneBattleAction::MagicDamage(String line)
{
		assert(false);
}

void RuneBattleAction::Damage(String line)
{
	assert(false);
}


/// Send to battle narrator.
void RuneBattleAction::Narrate(String line)
{
	QueueGraphics(new GMSetUIs("Narrator", UITarget::Text, line));
}

/// Checks for a name and stuff.
bool RuneBattleAction::IsValid()
{
	if (name.Length() == 0)
		name = id;
	if (effects.Size() <= 0)
		return false;
	return true;
}

/// Parses a string with targets as defined in a CSV file. E.g. "Ally Select 1"
void RuneBattleAction::ParseTargets(String fromTargetString)
{
	targetFilter = ParseTargetFilter(fromTargetString);
}	

void RuneBattleAction::ParseElements(String fromString)
{
	List<String> toks = fromString.Tokenize(" ");
	for (int i = 0; i < toks.Size(); ++i)
	{
		// spell->SetTargetFilterByString(word);
	}	
}

/// Returns false if any errors occured while parsing. Errors may not always be fatal but could be highly relevant.
bool RuneBattleAction::ParseDurations(String fromString)
{
	// Get latest effect and set duration for it.
	if (effects.Size() == 0)
	{
	//	std::cout<<"\nNo effect to attach duration to!";					
		return false;
	}

	List<String> tokens = fromString.Tokenize(" ()");
	int effectIndex = 0;
	/// Poke Karl about this... easier parsing would be nice.
	for (int i = 0; i < tokens.Size(); ++i)
	{
		String tok = tokens[i];
		if (effectIndex >= effects.Size())
		{
			std::cout<<"\nParseDurations Error: effectIndex out of bounds.";
			return false;
		}
 		BattleEffect * effect = &effects[effectIndex];
		if (tok == "0")
		{
			effect->durationType = BattleEffect::INSTANTANEOUS;
		}
		else if (tok == "Time")
		{
			String nextTok = tokens[i+1];
			effect->durationType = BattleEffect::TIME_IN_MS;
			effect->durationValue = nextTok.ParseFloat() * 1000;
			++i;
		}
		else if (tok == "Turn")
		{
			String nextTok = tokens[i+1];
			effect->durationType = BattleEffect::TIME_IN_MS;
			effect->durationValue = nextTok.ParseFloat() * 1000;
			++i;
		}
		else if (tok == "Attack")
		{
			String nextTok = tokens[i+1];
			effect->durationType = BattleEffect::ATTACKS;
			effect->durationValue = nextTok.ParseInt();
			++i;
		}
		else if (tok == "inf")
		{
			effect->durationType = BattleEffect::PERMANENT;
		}
		else {
			assert(false && "Bad token in RuneBattleAction::ParseDuration");
		}
		++effectIndex;
	}

//	BattleEffect & effect = spell->effects.Last();
//	effect.durationInMs = BattleEffect::INSTANTANEOUS; // GetDurationByString(word);
	return true;
}

void RuneBattleAction::ParseEffects(String fromString)
{
	this->effects = BattleEffect::ParseEffects(fromString);
}

void RuneBattleAction::ParseOtherCosts(String fromString)
{
	// o.o
}

void RuneBattleAction::SetElements(String toParse)
{
	elements.Clear();
	toParse.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (toParse.Contains("Fire"))
		elements.Add(Element::FIRE);
	if (toParse.Contains("Water"))
		elements.Add(Element::WATER);
	if (toParse.Contains("EARTH"))
		elements.Add(Element::EARTH);
	if (toParse.Contains("AIR"))
		elements.Add(Element::AIR);
	
	if (toParse.Contains("CHAOS"))
		elements.Add(Element::CHAOS);
	if (toParse.Contains("BALANCE"))
		elements.Add(Element::BALANCE);
	if (toParse.Contains("LIFE"))
		elements.Add(Element::LIFE);
	if (toParse.Contains("DEATH"))
		elements.Add(Element::DEATH);
}



/// Will depend on the filter.
bool RuneBattleAction::HasValidTargets()
{
	switch(targetFilter)
	{
		case TargetFilter::ALLY:
		case TargetFilter::ALLIES:
		case TargetFilter::ENEMIES:
		case TargetFilter::ENEMY:
			if (this->targets.Size() == 0)
				return false;
			break;
		default:
			assert(false && "Implement");
	}
	return true;
}


/// Applies all effects this spells should apply. If needed to re-apply multiple effects, this function should be called again until it returns true.
bool RuneBattleAction::ApplyEffects(RBattleState & bs)
{
	bool allEffectsApplied = true;
	for (int i = 0; i < effects.Size(); ++i)
	{
		BattleEffect & effect = effects[i];
		effect.applied = true;
		for (int j = 0; j < targets.Size(); ++j)
		{
			effect.ApplyTo(targets[j], subjects[0], bs);
		}
		if (!effect.applied)
		{
			allEffectsApplied = false;
		}
	}
	return allEffectsApplied;
};


