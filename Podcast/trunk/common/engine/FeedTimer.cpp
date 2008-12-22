#include "FeedTimer.h"
#include "FeedEngine.h"
#include "debug.h"

CFeedTimer::CFeedTimer(CFeedEngine *aFeedEngine) : CTimer(EPriorityIdle), iFeedEngine(aFeedEngine) 
	{
	CActiveScheduler::Add(this);
	}

CFeedTimer::~CFeedTimer() 
	{
	}

void CFeedTimer::ConstructL() 
	{
	CTimer::ConstructL();
	}

void CFeedTimer::RunL() 
	{
	DP("CFeedTimer::RunL");

	// We need to trap this, otherwise we will not reschedule the timer
	TRAP_IGNORE(iFeedEngine->UpdateAllFeedsL());

	// run again
	RunPeriodically();
	}

void CFeedTimer::SetPeriod(TInt aPeriodMinutes) 
	{
	DP1("Setting sync period to %d", aPeriodMinutes);
	iPeriodMinutes = aPeriodMinutes;
	}

void CFeedTimer::SetSyncTime(TTime aTime) 
	{
	TTime time;
	time.HomeTime();

	DP5("Now is %4d-%02d-%02d, %02d:%02d", time.DateTime().Year(), time.DateTime().Month()+1, time.DateTime().Day()+1, time.DateTime().Hour(), time.DateTime().Minute());

	TInt hour = aTime.DateTime().Hour();
	TInt minute = aTime.DateTime().Minute();
	
	
	TDateTime dTime;
	
	dTime.Set(time.DateTime().Year(), time.DateTime().Month(),
			time.DateTime().Day(),aTime.DateTime().Hour(),
			aTime.DateTime().Minute(), 0, 0);

	TTimeIntervalMinutes tmi = 0;

	// if this time already passed, add one day
	if (time.DateTime().Hour() > hour || time.DateTime().Hour() == hour && time.DateTime().Minute() > minute) 
		{
		DP("Adding one day");
		tmi = 60*24;
		}
	
	TTime atTime(dTime);
	atTime = atTime + tmi;
	DP5("Setting sync timer to %4d-%02d-%02d, %02d:%02d", atTime.DateTime().Year(), atTime.DateTime().Month()+1, atTime.DateTime().Day()+1, atTime.DateTime().Hour(), atTime.DateTime().Minute());

	// Activate the timer
	At(atTime);
	}
	

void CFeedTimer::RunPeriodically() 
	{
	DP1("RunPeriodically; thePeriod=%d", iPeriodMinutes);
	TTime time;
	time.UniversalTime();

	TTimeIntervalMinutes tmi;
	tmi = iPeriodMinutes;
	time = time + tmi;
	DP("Running timer");

	AtUTC(time);
	}

