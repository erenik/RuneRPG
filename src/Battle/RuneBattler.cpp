/// Emil Hedemalm
/// 2013-07-09
/// A battler in a the RuneRPG battles!

#include "RuneBattler.h"

#include <cmath>
#include <fstream>
#include <cstring>
#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "RuneBattleAction.h"
#include "File/File.h"

#include "RuneBattleActionLibrary.h"

#include "Maps/MapManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Random/Random.h"

#include "MathLib/Function.h"
#include "BattleStats.h"
#include "Item/ItemTypes.h"
#include "StateManager.h"

RuneBattler::RuneBattler()
{
	Nullify();
}

RuneBattler::~RuneBattler()
{
	actions.ClearAndDelete();
	queuedActions.ClearAndDelete();
	actionCategories.ClearAndDelete();

	if (entity)
	{
		MapMan.DeleteEntity(entity);
	}
}

/// Attempts to equip the provided gear.
void RuneBattler::Equip(List<RuneItem> gear)
{
	/// Check the gear we are equipping.
	for (int i = 0; i < gear.Size(); ++i)
	{
		RuneItem gearPiece = gear[i];
		if (gearPiece.type == ItemType::WEAPON)
			*weapon = gearPiece;
		else if (gearPiece.type == ItemType::GEAR)
		{
			// Check slot.
			switch(gearPiece.slot)
			{
				case Slot::HEAD:
					*headPiece = gearPiece;
					break;
				case Slot::TORSO:
					*torsoPiece = gearPiece;
					break;
				case Slot::HANDS:
					*handsPiece = gearPiece;
					break;
				case Slot::FEET:
					*feetPiece = gearPiece;
					break;
				case Slot::OFF_HAND:
					*offHand = gearPiece;
					break;
			}
		}
	}
}


/// Adds the Attack, Item and Flee commands.
void RuneBattler::AddDefaultActions()
{
	// Attack,.. regular physical attack!
	const RBA * attack = RBALib.GetBattleAction("Attack");
	assert(attack);
	this->actions.Add(new RuneBattleAction(*attack));
	// Flee,  50% success, 40 ap,  1 second cast time, 0 second freeze-time.
	const RBA * flee = RBALib.GetBattleAction("Flee");
	assert(flee);
	this->actions.Add(new RuneBattleAction(*flee));
}


/// Queues a new battle action to be executed when possible.
void RuneBattler::QueueAction(RuneBattleAction * rba)
{
	queuedActions.Add(rba);
}

/// Creates the Entity to animate and visualize this battler while in action.
void RuneBattler::CreateEntity()
{
	/// Skip if we already have an entity. No need to re-create it, yo?
	if (entity)
		return;
	// Place them and their entities on the grid.
	Model * model = NULL;
	Texture * tex;
	if (isEnemy)
	{
		tex = TexMan.GetTextureByName(TextureCategory::Asset, "White");
		model = ModelMan.GetModel("obj/Sprite.obj");
	}
	// Player! Face 'em leftward!
	else 
	{
		tex = TexMan.GetTextureByName(TextureCategory::Asset, "Red");
		model = ModelMan.GetModel("obj/SpriteMirroredUVs.obj");
	}
	/// o.o
	entity = MapMan.CreateEntity("RuneBattlerEntity: "+name, model, tex);
	
	/// Give it animation set if applicable.
	if (animationSet.Length())
		QueueGraphics(new GMSetEntity(entity, GT_ANIMATION_SET, animationSet));
	/// Give a default animation set to those lacking one.
	else 
	{
		QueueGraphics(new GMSetEntity(entity, GT_ANIMATION_SET, "anim/Battle/SwordSlasher"));
	}
	/// Scale it.
	Physics.QueueMessage(new PMSetEntity(entity, PT_SET_SCALE, 2.f));
	/// Set relative position due to the animation's form.
}

void RuneBattler::Nullify()
{
	unconscious = false;
	delay = 5000 + rand()%10000;
	state = IDLE;

	entity = NULL;
	isAI = false;
	isEnemy = false;
	actionPoints = 0;
	actionPointsFillUpSpeed = 0;
	
	/// These require further investigation later on!
	battleGearWeight = 0;
	battleWeightLimit = 1;

	// Create stats.
	for (int i = 0; i < RStat::NUM_STATS; ++i)
	{
		/// Initialize all to 0-ints?
		baseStats.Add(Variable(GetStatShortString(i),0));
	}
	
	/// Reset all to 1.
	for (int i = 0; i < RStat::NUM_BATTLER_STATS; ++i)
	{
		baseStats[i].iValue = 1;
	}
	/// Set som default values for testing purposes.
	for (int i = RStat::FIRST_CORE_STAT; i <= RStat::LAST_CORE_STAT; ++i)
		baseStats[i].iValue = 5;
	for (int i = RStat::FIRST_SURVIVAL_STAT; i <= RStat::LAST_SURVIVAL_STAT; ++i)
		baseStats[i].iValue = 50;

	timeSinceLastManaGain = 0;

	/// Add ze weapon.
	allGear = RuneItem::DefaultGear();
	weapon = headPiece = torsoPiece = handsPiece = feetPiece = offHand = NULL;
//	allGear.Add(RuneItem::DefaultWeapon());
	for (int i = 0; i < allGear.Size(); ++i)
	{
		RuneItem & piece = allGear[i];
		switch(piece.type)
		{
			case ItemType::WEAPON:
				weapon = &piece;
				break;
			case ItemType::GEAR:
			{
				switch(piece.slot)
				{
					case Slot::HEAD:
						headPiece = &piece;
						break;
					case Slot::TORSO:
						torsoPiece = &piece;
						break;
					case Slot::HANDS:
						handsPiece = &piece;
						break;
					case Slot::FEET:
						feetPiece = &piece;
						break;
					case Slot::OFF_HAND:
						offHand = &piece;
						break;
				}
				break;
			}
		}
	}
	assert(headPiece && torsoPiece && handsPiece && feetPiece && offHand);
	
	/// yo.
	UpdateCurrentStats();
}

/// from 0 to 4, 0 being player, 1-3 being enemies 1-3
RuneBattler::RuneBattler(int defaultTypes) 
{
	assert(false);
	Nullify();
}

/// If dead, for example, it is not.
bool RuneBattler::IsARelevantTarget()
{
	if (this->state == RuneBattler::DEAD ||
		this->unconscious == true)
		return false;
	return true;
}


/// Called if actionPointsFillUpSpeed is set to 0. If speed changes, set it to 0 and it will be automatically re-calculated upon next iteration.
void RuneBattler::RecalculateActionPointsFillUpSpeed()
{
	Function actionBarFillupSpeedFunc = Function::GetFunctionByName("Action bar fill-up speed");
	assert(actionBarFillupSpeedFunc.Good());
	ExpressionResult result = actionBarFillupSpeedFunc.Evaluate(currentStats);
	this->actionPointsFillUpSpeed = result.GetFloat();
	std::cout<<"Action points fill up speed per millisecond recalculated to: "<<this->actionPointsFillUpSpeed;

	Function maxActionPointsFunc = Function::GetFunctionByName("Max action points");
	assert(maxActionPointsFunc.Good());
	result = maxActionPointsFunc.Evaluate(currentStats);
	maxActionPoints = result.GetFloat();
}

/// Increments action points and other stuff according to current effects.
/// For AI, it will also queue up actions accordingly to the state of the battle at large.
void RuneBattler::Process(RBattleState & battleState)
{
	int timeInMs = battleState.timeInMs;
	/// For each battle action, process it. Old actions may also be present which are still animating or have some effect to do after animation finishes.
	bool frozenInAction = false;
	for (int i = 0; i < activeBattleActions.Size(); ++i)
	{
		RuneBattleAction * rba = activeBattleActions[i];
		rba->Process(battleState);
		if (rba->finished)
		{
			activeBattleActions.Remove(rba);
			delete rba;
			--i;
			continue;
		}
		/// Check if frozen with this action.
		frozenInAction |= !rba->freezeTimeOver;
	}

	// If dead, don't process any more?
	if (state == RuneBattler::DEAD ||
		unconscious == true)
		return;

	/// Make active a queued action if we are not frozen.
	if (queuedActions.Size() && !frozenInAction)
	{
		RuneBattleAction * rba = queuedActions[0];
		/// Check if we can initiate the action yet.
		bool readyToStart = true;
		if (rba->actionCost > actionPoints ||
			rba->manaCost > currentStats[RStat::CURRENT_MP].iValue)
		{
			readyToStart = false;
		}
		if (readyToStart)
		{
			// Remove costs as necessary.
			actionPoints -= rba->actionCost;
			currentStats[RStat::CURRENT_MP].iValue -= rba->manaCost;

			if (!rba->primarySubject)
				rba->primarySubject = this;
			queuedActions.Remove(rba); // Remove from queue
			activeBattleActions.Add(rba); // Add to list of active actions.
			rba->OnBegin(battleState); // Call on Begin for it!
		}
	}

	/// Evaluate if any effects should be removed due to duration, or apply some effects?
	for (int i = 0; i < appliedEffects.Size(); ++i)
	{
		BattleEffect & effect = appliedEffects[i];
		bool remain = effect.Process(battleState);
		if (!remain)
		{
			battleState.log = name+"'s "+effect.name+" wears off.";
			appliedEffects.RemoveIndex(i);
			--i;
			/// Update stats as needed.
			this->UpdateCurrentStats();
		}
	}

    /// Check state, has it queued an action yet?
	if (state == RuneBattler::IDLE)
	{
		if (this->actionPointsFillUpSpeed == 0)
			this->RecalculateActionPointsFillUpSpeed();

		this->actionPoints += actionPointsFillUpSpeed * battleState.timeInMs * 0.001;
		if (actionPoints > maxActionPoints)
			actionPoints = maxActionPoints;

		/// For mana gain over time.
		timeSinceLastManaGain += timeInMs;
		if (timeSinceLastManaGain > 1000)
		{
			timeSinceLastManaGain = 0;
			int & currentMP = baseStats[RStat::CURRENT_MP].iValue;
			int & maxMP = baseStats[RStat::MAX_MP].iValue;
			++currentMP;
			if (currentMP > maxMP)
				currentMP = maxMP;
			else
				UpdateCurrentStats();
		}
		// Use some other boolean to just signify that the menu should be opened..
		/*
		Message * msg = new Message(MessageType::STRING);
		msg->msg = "BattlerReady("+this->name+")";
		msg->data = (char*)this;
		MesMan.QueueMessage(msg);
		*/
	}

    /// Process status-effects.

    /// Return if no AI.
	if (!isAI || state != RuneBattler::IDLE)
        return;

	/// Queue up an action, random ones work fine.
	if (actions.Size() == 0)
	{
//		std::cout<<"\nBattler lacking any valid action to take";
		// Load 'em.
		if (!UpdateActions())
			return;
	}
	// Return if already got an action queued.
	if (queuedActions.Size())
		return;
	RuneBattleAction * ba = actions[rand()%actions.Size()];
	RuneBattleAction * rba = new RuneBattleAction(*ba);
	rba->subjects = this;
	/// Fine applicable targets..
	List<RuneBattler*> applicableTargets;
	for (int i = 0; i < battleState.battlers.Size(); ++i)
	{
		RuneBattler * rb = battleState.battlers[i];
		if (rb->isEnemy == this->isEnemy)
			continue;
		if (rb->state == DEAD)
			continue;
		applicableTargets.Add(rb);
	}
	/// No valid target! Abort! o.o
	if (applicableTargets.Size())
	{	
		/// Then just randomize 1 for now.
		RuneBattler * b =  applicableTargets[rand()%applicableTargets.Size()];
		if (b)
			rba->targets.Add(b);
	}
	/// What if no valid targets?
	if (!rba->HasValidTargets())
	{
		// Then abort it.
		delete rba;
		return;
	}
	queuedActions.Add(rba);
    /// Since no good AI atm, return.
    return;
}

void RuneBattler::OnActionFinished()
{
	if (queuedActions.Size())
		state = RuneBattler::PREPARING_FOR_ACTION;
	else
		state = RuneBattler::IDLE;
}

/// Returns true if no actions are queued, false if not.
bool RuneBattler::IsIdle()
{
	if (queuedActions.Size() == 0)
        return true;
    return false;
}

/// Sets MP/HP to max, etc.
void RuneBattler::ResetStats()
{
	// Is this even used?
	/*
	hp = maxHP;
	mp = maxMP;
	this->actionPoints = 0;
	*/
}

/// Returns a list of variables containing each stat using their specified short-name, taken into consideration all effects that have been applied.
List<Variable> RuneBattler::GetCurrentStats()
{
	return currentStats;
}


/// Take damage o-o, returns true if it killed/incapacitated the battler.
bool RuneBattler::Damage(int dmg)
{
	Variable * hp = GetHP();
	hp->iValue -= dmg;
	UpdateCurrentStats();
	if (hp->iValue <= 0)
	{
		OnKO();
		return true;
	}
	return false;
}

/// See Dropbox/Emka Design/Karls/Rune RPG/Equations and testing for some formulae
bool RuneBattler::PhysicalDamage(int dmg)
{
	//float armorReduction = pow(0.99f, armor);
	//std::cout<<"\nPhysical damage reduced by armor from "<<dmg;
	//dmg *= armorReduction;
	//std::cout<<" to "<<dmg<<".";
	return Damage(dmg);
}

/// Perform magic-damage-reduction before applying it.
bool RuneBattler::MagicDamage(int dmg)
{
	//float magicArmorReduction = pow(0.98f, magicArmor);
	//std::cout<<"\nMagical damage reduced by magicArmor from "<<dmg;
	//dmg *= magicArmorReduction;
	//std::cout<<" to "<<dmg<<".";
	/// Currently no deduction is made.
	return Damage(dmg);
}

/// Updates which actions it has available using the RuneBattleAction library...?
bool RuneBattler::UpdateActions()
{
	actions.Clear();
	/// Always allow players to attack..
	if (!isAI)
		AddDefaultActions();

	for (int i = 0; i < actionNames.Size(); ++i)
	{
		String name = actionNames[i];
		name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		/// Special cases
		if (name == "All spells")
		{
			// Create a copy of each spell!
			List<const RuneBattleAction*> spells = RBALib.GetSpells();
			for (int j = 0; j < spells.Size(); ++j)
				actions.Add(new RuneBattleAction(*spells[j]));
			continue;
		}
		else if (name == "All skills")
		{
			// Create a copy of each spell!
			List<const RuneBattleAction*> skills = RBALib.GetSkills();
			for (int j = 0; j < skills.Size(); ++j)
				actions.Add(new RuneBattleAction(*skills[j]));
			continue;
		}
		else if (name == "Attack" || name == "Basic Attack")
		{
			const RuneBattleAction * action = RBALib.GetBattleAction("Attack");
			if (action)
				actions.Add(new RuneBattleAction(*action));
			continue;
		}
		else if (name.Contains("Skill:"))
		{
			String skillName = name.Tokenize(":")[1];
			// Fetching a range of skills based on id.
			if (skillName.Length() < 6 && skillName.Contains("-"))
			{
				String startID = skillName.Tokenize("-")[0];
				String stopID = skillName.Tokenize("-")[1];
				List<const RuneBattleAction*> skills = RBALib.GetSkills();
				bool started = false;
				for (int j = 0; j < skills.Size(); ++j)
				{
					const RuneBattleAction * skill = skills[j];
					if (skill->id == startID)
						started = true;
					if (started)
						actions.Add(new RuneBattleAction(*skill));
					if (skill->id == stopID)
						started = false;
				}
				continue;
			} 
			// Default, get by simple name.
			const RuneBattleAction * action = RBALib.GetBattleAction(skillName);
			if (action)
				actions.Add(new RuneBattleAction(*action));
			continue;
		}
		else if (name.Contains("Spell:"))
		{
			String skillName = name.Tokenize(":")[1];
			// Fetching a range of skills based on id.
			if (skillName.Length() < 6 && skillName.Contains("-"))
			{
				String startID = skillName.Tokenize("-")[0];
				String stopID = skillName.Tokenize("-")[1];
				List<const RuneBattleAction*> skills = RBALib.GetSpells();
				bool started = false;
				for (int j = 0; j < skills.Size(); ++j)
				{
					const RuneBattleAction * skill = skills[j];
					if (skill->id == startID)
						started = true;
					if (started)
						actions.Add(new RuneBattleAction(*skill));
					if (skill->id == stopID)
						started = false;
				}
				continue;
			} 
			// Default, get by simple name.
			const RuneBattleAction * action = RBALib.GetBattleAction(skillName);
			if (action)
				actions.Add(new RuneBattleAction(*action));
			continue;
		}
		const RuneBattleAction * action = RBALib.GetBattleAction(name);
		if (action)
			actions.Add(new RuneBattleAction(*action));
	}
	if (actions.Size())
		return true;
	return false;
}

/// Divides the actions into categories depending on the given scheme.
void RuneBattler::UpdateActionCategories(int usingSortingScheme)
{
	actionCategories.ClearAndDelete();
	switch(usingSortingScheme)
	{
		// Default scheme! no categories! :D
		case 0:
			for (int i = 0; i < actions.Size(); ++i)
			{
				RuneBattleAction * action = actions[i];
				RuneBattleActionCategory * cat = new RuneBattleActionCategory();
				cat->name = action->name;
				cat->isAction = action;
				actionCategories.Add(cat);
			}
			break;
		// Standard categories: Attack, Spells, Skills, Items
		case 1:
		{
			RuneBattleActionCategory * attack = new RuneBattleActionCategory("Attack"),
				* spells = new RuneBattleActionCategory("Spells"),
				* skills = new RuneBattleActionCategory("Skills"),
				* mundaneActions = new RuneBattleActionCategory("Mundane Actions"),
				* items = new RuneBattleActionCategory("Items"),
				* flee = new RuneBattleActionCategory("Flee");
			actionCategories.Add(attack, spells, skills, 
				mundaneActions, items);
			actionCategories.AddItem(flee);
			for (int i = 0; i < actions.Size(); ++i)
			{
				RuneBattleAction * rba = actions[i];
				switch(rba->type)
				{
					case RuneBattleAction::MUNDANE_ACTION:
						/// Only a few of these..
						if (rba->name == "Attack" || rba->name == "Basic Attack")
						{
							attack->isAction = rba;
						}
						else if (rba->name == "Flee")
						{
							flee->isAction = rba;
						}
						else 
						{
							mundaneActions->actions.Add(rba);						
						}
						break;
					case RuneBattleAction::MAGIC_SKILL:
						skills->actions.Add(rba);
						break;
					case RuneBattleAction::MAGIC_SPELL:
						spells->actions.Add(rba);
						break;
					case RuneBattleAction::USE_ITEM:
						items->actions.Add(rba);
						break;
					default:
						assert(false && "Bad action type should be one of the following: magic skill, magic spell or mundane action");
				}
			}
			/// Remove those categories which have no ability at all.
			for (int i = 0; i < actionCategories.Size(); ++i)
			{
				RuneBattleActionCategory * rbac = actionCategories[i];
				if (rbac->actions.Size() == 0 && rbac->isAction == NULL)
				{
					actionCategories.Remove(rbac);
					delete rbac;
					--i;
				}
			}
			/// If we have no actions at all, create a category to show this explicitly?
			if (actionCategories.Size() == 0)
			{
				actionCategories.Add(new RuneBattleActionCategory("No valid actions available"));
			}
			break;
		}
	}
}

/// Average armor rating of all equipment pieces, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
float RuneBattler::ArmorRating()
{
	return 5.f;
}
/// Average magic armor rating of all equipment pieces, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
float RuneBattler::MagicArmorRating()
{
	return 5.f;
}
/// Weapon damage, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
float RuneBattler::WeaponDamage()
{
	return currentStats[RStat::WEAPON_MODIFIER].iValue;
}
/// Shield block rating, including buffs/debuffs. Will mostly vary within the 0.0 to 20.0 interval.
float RuneBattler::ShieldBlockRating()
{
	return 0.f;
}
/// Shield defense modifier, including buffs/debuffs. Will mostly vary within the 0.0 to 20.0 interval.
float RuneBattler::ShieldDefenseModifier()
{
	return 0.f;
}

/// Returns the variable containing HP, so that it may be adjusted.
Variable * RuneBattler::GetHP()
{
	return &baseStats[RStat::CURRENT_HP];
}

/// Returns current HP, including buffs/debuffs.
int RuneBattler::HP()
{
	return currentStats[RStat::CURRENT_HP].iValue;
}
int RuneBattler::MaxHP()
{
	return currentStats[RStat::MAX_HP].iValue;
}
int RuneBattler::MP()
{
	return currentStats[RStat::CURRENT_MP].iValue;
}
int RuneBattler::MaxMP()
{
	return currentStats[RStat::MAX_MP].iValue;
}


///// Returns current Attack power, including buffs/debuffs.
//int RuneBattler::AttackPower()
//{
//	assert(false0
//	return attackPower;
//}
///// Returns current physical and magical defense power, including buffs/debuffs.
//int RuneBattler::DefensePower()
//{
//	return defensePower;
//}
///// Returns current offensive magical power, including buffs/debuffs applied to it.
//int RuneBattler::MagicPower()
//{
//	return magicPower;
//}
///// Returns current agility, including buffs/debuffs.
//int RuneBattler::Agility()
//{
//	return agility;
//}
///// Returns current speed, including buffs/debuffs.
//int RuneBattler::Speed()
//{
//	return speed;
//}

/// Returns current amount of action points, floored.
int RuneBattler::ActionPoints()
{
	return int(actionPoints);
}

Random battlerRandom;
#define Maximum(a,b) ((a) > (b)? (a) : (b))

/// Returns true if a dodge occurred. The resulting damage is stored in the second parameter.
bool RuneBattler::Dodge(int preDodgeDamage, int & dodgeModifiedDamage)
{
	assert(false);
/*
	float baseDodgeRating = Agility() * 0.25f + 1;
	float dodgeWeightModifier = Maximum(battleGearWeight / battleWeightLimit - 0.25, 0);
	float dodgeRating = baseDodgeRating * (1 - dodgeWeightModifier);
	float dodgeRand = battlerRandom.Randf() * 100.f;
	if (dodgeRand > dodgeRating)
		return false;
	/// In percent.
	float dodgeAmount = 0.40 + battlerRandom.Randf() * (0.80 + Agility() * 0.005);
	dodgeModifiedDamage *= 1 - dodgeAmount;
	*/
	return true;
}

/// Returns true if a parry occurred. The resulting damage is stored in the second parameter.
bool RuneBattler::Parry(int preParryDamage, int & parryModifiedDamage)
{
	assert(false);
	/*
	float parryRating = Agility() * 0.5f + 1;
	float parryRand = battlerRandom.Randf() * 100.f;
	if (parryRand > parryRating)
		return false;
	/// In percent.
	float parryAmount = 0.40 + battlerRandom.Randf() * (0.60 + Agility() * 0.00125);
	parryModifiedDamage *= 1 - parryAmount;
	*/
	return true;
}
/// Returns true if a block occurred. The resulting damage is stored in the second parameter.
bool RuneBattler::ShieldBlock(int preBlockDamage, int & blockModifiedDamage)
{
	assert(false);
	/*
	float baseBlockRating = 10 + 2 * ShieldBlockRating() * (1 + Agility() * 0.01f);
	float blockRand = battlerRandom.Randf() * 100.f;
	if (blockRand > baseBlockRating)
		return false;
	/// In percent. Allow blocking even without a shield? Sure!
	float blockAmount = 0.10 + 0.02 * (1 + ShieldDefenseModifier()) * (1 +  DefensePower() * 0.01f);
	int diff = blockAmount * preBlockDamage;
	if (diff <= 0)
		diff = 1;
	blockModifiedDamage -= diff;
	*/
	return true;
}
/// Returns true if a critical occurred. The resulting damage is stored in the second parameter.
bool RuneBattler::Critical(int preCriticalDamage, int & postCriticalDamage)
{
	assert(false);
	/*
//	Base critical hit rate (5 to 55%) = 5 + Max(Agility, Attack power) * 0.125 %
//	Critical damage increase (+100 to +250%) = 100 + Random(0 to Max(Agility, Attack power) * 0.25) %
	float baseCriticalHitRate = 5 + Maximum(Agility(), AttackPower()) * 0.125f;
	float critRand = battlerRandom.Randf() * 100.f;
	if (critRand > baseCriticalHitRate)
		return false;
	float criticalIncrease = 1.f + battlerRandom.Randf() * Maximum(Agility(), AttackPower()) * 0.0025f;
	postCriticalDamage *= 1 + criticalIncrease;
	*/
	return true;
}


/// When getting knocked out/incapacitated, cancels queued actions, etc.
void RuneBattler::OnKO()
{
	// Reset some variables.
//	hp = 0;
	unconscious = true;
	state = RuneBattler::DEAD;

	/// Queue a message of the event to the game state.
	MesMan.QueueMessage(new Message("OnBattlerIncapacitated("+name+")"));
	
	// Notify all queued actions to be halted. -> Delete 'em.
	queuedActions.ClearAndDelete();

	/// If we got a visible entity, animate it for death.
	if (entity)
	{
		// Make it invisible!
		QueueGraphics(new GMSetEntityb(entity, GT_VISIBILITY, false));
	}
}


/// Bleh o-o
bool RuneBattler::Load(String source)
{
	this->source = source;
	enum {
		NULL_MODE,
		ACTIONS, SKILLS = ACTIONS,
	};
	int inputMode = NULL_MODE;
	List<String> lines = File::GetLines(source);
	if (!lines.Size())
		return false;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
		List<String> tokens = line.Tokenize(" \t");
		/// One or more word below
		if (tokens.Size() == 0)
			continue;
		String token = tokens[0];
		String key = tokens[0];
		key.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (key == "Actions" || key == "Skills")
		{
			inputMode = ACTIONS;
			continue;
		}
		else if (inputMode == ACTIONS)
		{
			this->actionNames.Add(line);
		}

	

		String token2;
		if (tokens.Size() >= 2)
			token2 = tokens[1];
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		std::cout<<"\nLine "<<i<<": "<<line;
		if (line.Length() < 2)
			continue;
		else if (line.Contains("name")){
			if (line.Contains("\"")){
				name = line.Tokenize("\"")[1];
			}
			else
				name = token2;
		}
		else if (line.Contains("animationSet"))
		{
			animationSet = token2;
		}
		//else if (token == "hp"){
		//	maxHP = hp = token2.ParseFloat();
		//}
		//else if (token == "mp"){
		//	maxMP = mp = token2.ParseInt();
		//}
		//else if (token == "attack"){
		//	attackPower = token2.ParseInt();
		//}
		//else if (token == "agility"){
		//	agility = token2.ParseInt();
		//}
		//else if (token == "defense"){
		//	defensePower = token2.ParseInt();
		//}
		//else if (token == "magicPower"){
		//	magicPower = token2.ParseInt();
		//}
		//else if (token == "magicSkill"){
		//	magicSkill = token2.ParseInt();
		//}
		//else if (token == "speed"){
		//	speed = token2.ParseInt();
		//}
		//else if (token == "weaponDamage"){
		//	weaponDamage = token2.ParseInt();
		//}
		//else if (token == "armor"){
		//	armor = token2.ParseInt();
		//}
		//else if (token == "magicArmor"){
		//	magicArmor = token2.ParseInt();
		//}
		//else if (token == "spellDamage"){
		//	spellDamage = token2.ParseInt();
		//}
		else if (token == "actions"){
		    String s = token2;
			std::cout<<"\nAction names before parsing: "<<actionNames.Size();
			for (int j = 0; j < actionNames.Size(); ++j){
				
			}
		    actionNames = s.Tokenize("\",");
		}
		else {
			std::cout<<"\nUnknown token "<<tokens[0]<<" while parsing RuneBattler data";
		//	assert(false && "Unknown token while parsing RuneBattler data");
		}
	}
	return true;
}


/// Updates all current stats. Should be called every time effects have been applied or any other change is done to the base-stats.
void RuneBattler::UpdateCurrentStats()
{
	currentStats.Clear();
	for (int i = 0; i < baseStats.Size(); ++i)
	{
		Variable baseStat = baseStats[i];

		/// Look at all gear.
		for (int j = 0; j < allGear.Size(); ++j)
		{
			RuneItem & gear = allGear[j];
			for (int k = 0; k < gear.equipEffects.Size(); ++k)
			{
				BattleEffect & effect = gear.equipEffects[k];
				switch(effect.type)
				{
					case BattleEffect::INCREASE:
					case BattleEffect::DECREASE:
					{
						int statType = effect.statType;
						// i = stat type in this case.
						if (statType == i)
						{
							baseStat.iValue = effect.AdjustedStat(baseStat.iValue);
						}
						break;
					}
					default:
					{
						std::cout<<"\nDefault, no effect on this stat..";
						break;
					}
				}
			}
		}

		/// Look at all effects..!
		for (int j = 0; j < appliedEffects.Size(); ++j)
		{
			BattleEffect effect = appliedEffects[j];
			switch(effect.type)
			{
				case BattleEffect::INCREASE:
				case BattleEffect::DECREASE:
				{
					int statType = effect.statType;
					// i = stat type in this case.
					if (statType == i)
					{
						baseStat.iValue = effect.AdjustedStat(baseStat.iValue);
					}
					break;
				}
				default:
				{
					std::cout<<"\nDefault, no effect on this stat..";
					break;
				}
			}
		}
		currentStats.Add(baseStat);
	}
	assert(currentStats.Size() == baseStats.Size());
}
