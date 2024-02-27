/// Emil Hedemalm
/// 2016-07-30
/// Pathing property for RuneRPG, working with the RRMovingProperty, and in the background the PathableProperty in the Pathfinding system.

#include "RRPathingProperty.h"
#include "RRTime.h"
#include "Pathfinding/PathMessage.h"
#include "Random/Random.h"
#include "File/LogFile.h"

Random pathRand;

bool RRPathingProperty::pause = false;

RRPathingProperty::RRPathingProperty(Entity * owner, ConstVec3fr startingPosition)
	: EntityProperty("RRPathProp", ID(), owner), startingPosition(startingPosition)
{
	currentPatternIndex = -1;
	timeInCurrentPatternMs = 0;
	goQueued = false;
}

/// Initializes with a Random of 10 distance, for morning day, Home at evening/night.
void RRPathingProperty::InitRandom()
{
	pattern.Clear();
	/// Random!
	PathPattern day(PathPattern::RANDOM);
	day.distance = 10;
	day.condition = Condition::TIME_OF_DAY;
	day.tod.Add(TimeOfDay::MORNING, TimeOfDay::DAY);

	PathPattern dayPause(PathPattern::IDLE);
	dayPause.durationMs = 3000;
	dayPause.condition = Condition::TIME_OF_DAY;
	day.tod.Add(TimeOfDay::MORNING, TimeOfDay::DAY);

	PathPattern night(PathPattern::GO_HOME);
	night.condition = Condition::TIME_OF_DAY;
	night.tod.Add(TimeOfDay::EVENING, TimeOfDay::NIGHT);
	// Add 'em
	pattern.Add(day, dayPause, night);
}

/// Should be called on spawn, in order to be mostly message-based.
void RRPathingProperty::OnSpawn()
{
	// Start it up.
	currentPatternIndex = -1;
	Go();
}

void RRPathingProperty::Go()
{
	if (pause)
		return;
	/// Go to next one.
	currentPatternIndex = (currentPatternIndex + 1) % pattern.Size();
	PathPattern * pp = CurrentPattern();
	if (pp == 0)
		return;	
	/*
	if (pp->repeatWaitMs > 0 && timeInCurrentPatternMs < pp->repeatWaitMs)
	{
		goQueued = true;
		return;
	}
	goQueued = false;
	*/
	timeInCurrentPatternMs = 0;
	Do();
}

/// Repeat command to initiate this movement.
void RRPathingProperty::Do()
{
	PathPattern * pp = CurrentPattern();
	if (pp->type == PathPattern::RANDOM)
	{
		/// Randomize position.
		float max = pp->distance * 2, offset = pp->distance;
		Vector3f position = this->startingPosition + Vector3f(pathRand.Randf(max) - offset, 0, pathRand.Randf(max) - offset);  // Random X and Y?
		/// Go there.
		SetPathDestinationMessage pm(position);
		owner->ProcessMessage(&pm);
	}
	if (pp->type == PathPattern::IDLE)
	{
		/// Uh... stop?
		Message msg("StopMoving");
		owner->ProcessMessage(&msg);
	}
}

PathPattern * RRPathingProperty::CurrentPattern()
{
	if (currentPatternIndex < 0 || currentPatternIndex >= pattern.Size())
		return 0;
	return &pattern[currentPatternIndex];
}

void RRPathingProperty::ProcessMessage(Message * message)
{
	PathPattern * pp = CurrentPattern();
	if (pp == 0)
		return;

	if (message->msg == "RRMovingProperty::DistanceMovedZero")
	{
		LogMain(owner->name+" distance moved zero. Restarting pathing", LogLevel::DEBUG);
		/// Just issue Stop() again?
		Do();
//		Go();
	}
	

	switch(pp->type)
	{
		case PathPattern::RANDOM:
		case PathPattern::GO_HOME:
			if (message->msg == "DestinationReached")
				Go();
			if (message->msg == "PathableProperty::ZeroLengthPathReceived")
				Go();
			if (message->msg == "RRMovingProperty::DistanceMovedZero")
			{
				LogMain(owner->name+" distance moved zero. Restarting pathing", LogLevel::DEBUG);
				Go();
			}
			break;
	};
}

void RRPathingProperty::Process(int timeInMs)
{
	PathPattern * pp = CurrentPattern();
	if (pp == 0)
		return;
	timeInCurrentPatternMs += timeInMs;
	if (pp->durationMs > 0 &&  pp->durationMs < timeInCurrentPatternMs)
		Go();
}
	