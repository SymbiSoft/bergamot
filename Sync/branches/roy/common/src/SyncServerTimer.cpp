#include "SyncServerTimer.h"
#include "e32std.h"
#include <EIKENV.h>
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>
#include "SyncServerData.h"
#include "debug.h"

CSyncServerTimer::CSyncServerTimer(TSmlProfileId anId) : CTimer::CTimer(EPriorityIdle), hour(0), minute(0), day(ESunday) {
	profileId = anId;
}

CSyncServerTimer::~CSyncServerTimer() {
}
void CSyncServerTimer::ConstructL() {
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
}

void CSyncServerTimer::RunL() {
	DP1("Sync account %d", profileId);
	
#ifndef __WINS__
	TTime time;
	time.UniversalTime();
	if (time < timeToSync) {
		// timer was probably reset, this happens on Nokia every 30 minutes
		DP("Timer reset");
		TRAPD(error,AtUTC(timeToSync));
		
		if (error != KErrNone) {
			DP1("Error from AtUTC: %d", error);
		}
		return;
	}

	RSyncMLSession session;
	TInt error;
	TRAP(error, session.OpenL());
	if (error!=KErrNone)
		{
		DP1("Session OpenL error: %d", error);
		}
	RSyncMLDataSyncJob job;
	TBuf<255> Buffer;
	
	RSyncMLDataSyncProfile profile;
	TRAP(error,profile.OpenL(session, profileId,ESmlOpenRead));
	if (error!=KErrNone)
	{
		DP1("ProfileOpenL error: %d", error);
	}
	
	profile.Close();
	
	// sync!
	TRAP(error,job.CreateL(session, profileId));
	
	if (error!=KErrNone)
	{
		DP1("CreateL error: %d", error);
	}
	
	job.Close();
	
	session.Close();
#endif
	// run again
	RunPeriodically();
}

void CSyncServerTimer::SetPeriod(int aPeriod) {
	DP1("Setting period to %d", aPeriod);
	thePeriod = aPeriod;
}

// Recalculates data member timeToSync
// by taking NOW (UniversalTime) and adding the "period" to it
// Period is Never, 15 minutes, one hour etc...
void CSyncServerTimer::RunPeriodically() {
	DP1("RunPeriodically; thePeriod=%d", thePeriod);
	TTime time;
	time.UniversalTime();

	switch(thePeriod) {
	case ENever:
		DP("ENever");
		return;
	case EInterval:
	{
		TTimeIntervalMinutes tmi = minute;
		TTimeIntervalHours thi = hour;
		time = time + tmi;
		time = time + thi;
		break;
	}
	case EFifteenMinutes: {
		DP("EFifteenMinutes");
		TTimeIntervalMinutes tmi;
		tmi = 15;
		time = time + tmi;
		break;
	}
	case EOneHour: {
		DP("EOneHour");
		TTimeIntervalMinutes tmi;
		tmi = 60;
		time = time + tmi;
		break;
	}
	case EFourHours: {
		DP("EFourHours");
		TTimeIntervalHours tmi;
		tmi = 4;
		time = time + tmi;
		break;
	}
	case ETwelveHours: {
		DP("ETwelveHours");
		TTimeIntervalHours tmi;
		tmi = 12;
		time = time + tmi;
		break;
	}	
	case EDaily: {
		DP("EDaily");
		TDateTime nowDateTime = time.DateTime();
		TDateTime syncDateTime(nowDateTime.Year(),nowDateTime.Month(),nowDateTime.Day(),hour,minute,0,0);
		TTime syncTime(syncDateTime);
		
		// If the sync time has already passed
		// we inrease by 1 day
		if (syncTime.Int64() <= time.Int64())
		{
			syncTime += TTimeIntervalDays(1);
		}
		
		time = syncTime;
		break;
	}
	case EWeekly: {
		DP("EWeekly");
		
		TDateTime nowDateTime = time.DateTime();
		TDateTime syncDateTime(nowDateTime.Year(),nowDateTime.Month(),day,hour,minute,0,0);
		TTime syncTime(syncDateTime);
		
		// If the sync time has already passed
		// we inrease by 1 day
		if (syncTime.Int64() <= time.Int64())
		{
			syncTime += TTimeIntervalDays(7);
		}
		
		time = syncTime;
		break;
	}
	default:
		DP1("ERROR: Invalid period=%d", thePeriod);
		return;
	}

	TBuf<100> timestr;
	time.FormatL(timestr, _L("%H:%T:%S"));
	DP1("Running timer, next sync at: %S", &timestr);
	timeToSync = time;
	TRAPD(error,AtUTC(timeToSync));
	
	if (error != KErrNone) {
		DP1("Error from AtUTC: %d", error);
	}
}

