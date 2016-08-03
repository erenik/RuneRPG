/// Emil Hedemalm
/// 2016-07-30
/// Time in-game. Built on top of Time::Time(

#include "RRTime.h"

/// Constructor with in-game time initializer.
RRTime::RRTime(int day, int hour, int minute)
	: Time(TimeType::MILLISECONDS_NO_CALENDER)
{
	/// Intervals will be in-game time.
	int hours = hour + day * 24;
	int minutes = hours * 60 + minute;
	int seconds = minutes * 60;
	intervals = seconds * IntervalsPerSecond();
	secondsPerIrlSecond = 30;
}
/// Adds real-time ms.
void RRTime::AddMs(int ms)
{
	intervals += ms * secondsPerIrlSecond;
}
/// Returns from the above enum TimeOfDay. Default 05-09 for morning, 09-17 for day, 17-21 evening, 21-05 night.
int RRTime::GetTimeOfDay()
{
	int hour = Hour();
	if (Hour() < 5 || Hour() >= 21)
		return TimeOfDay::NIGHT;
	if (Hour() < 9)
		return TimeOfDay::MORNING;
	if (Hour() < 17)
		return TimeOfDay::DAY;
	return TimeOfDay::EVENING;
}
