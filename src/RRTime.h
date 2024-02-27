/// Emil Hedemalm
/// 2016-07-30
/// Time in-game. Built on top of Time::Time(

#ifndef RR_TIME_H
#define RR_TIME_H

#include "Time/Time.h"

namespace TimeOfDay{
enum {
	MORNING,
	DAY,
	EVENING,
	NIGHT,
};
};
class RRTime : public Time 
{
public:
	/// Constructor with in-game time initializer.
	RRTime(int day = 0, int hour = 8, int minute = 0);
	/// Adds real-time ms.
	void AddMs(int ms);
	/// Returns from the above enum TimeOfDay. Default 05-09 for morning, 09-17 for day, 17-21 evening, 21-05 night.
	int GetTimeOfDay();
	/// Just use Hour(), Minute(), Day() of base Time class to get the time.
//	int GetInGameHour(); // 0-23
//	int GetInGameMinute(); // 0-59
//	int GetInGameDay(); // 0-7? 0-X?

	/// Multiplier. default 30 seconds per in-game second.
	int secondsPerIrlSecond;
private:
	int minute, hour, day, timeOfDay;

	/// Total ms played.
	int ms;
};

#endif
