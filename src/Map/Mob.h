/// Evergreen IT AB
/// 2024-03-07

#pragma once

enum class MobType {
	Player,
	Default,
	Goblin,
	Orc
};

class Mob {
public:
	Mob();
	Mob(MobType type);
	int hp;
	int damage;
	int attack;
	int defense;
	MobType type;

	// processes regens
	void Process(int timeInMs);
	void Damage(int damage);

	void ConsumeFatigue(int amount);
	int Fatigue() const;

	// Regens
	int fatigueRegenPerSecond;

	void GetExp(int amount);

	void StartBlocking();
	bool IsBlocking() const { return isBlocking; }

	// For player onry.
	int exp;
	int level;

private:

	bool isBlocking = false;
	int timeBlockingMs = 0;

	int timeOutsideOfCombatMs = 0;

	bool KO = false;
	void Init(MobType type);

	void UpdateStats();

	int toNextLevel = 0;

	int fatigue;
	int maxFatigueMillis;
	int milliFatigue;
	int maxHp;
	int hpRegenPer3Seconds;

	int hpRegenTicker = 0;

};
