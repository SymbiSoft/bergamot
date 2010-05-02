#ifndef SYNCSERVERTIMER_H
#define SYNCSERVERTIMER_H

#include <e32base.h>
#include "SyncMLDef.h"

_LIT(KSyncServerMutex, "SyncServerMutex");

class CSyncServerTimer : public CTimer {
public:
	CSyncServerTimer(TSmlProfileId anId);
	~CSyncServerTimer();
	void ConstructL();
	void RunL();
	void RunPeriodically();
	void SetPeriod(int period);
	
	void SetDay(TDay aDay) {day = aDay;}
	void SetHour(TInt aHour) {hour = aHour;}
	void SetMinute(TInt aMinute) {minute = aMinute;}
	
private:
	TTime timeToSync;
	TSmlProfileId profileId;
	int thePeriod;
	TDay day;
	TInt hour;
	TInt minute;
	
//	RMutex mutex;
};
#endif

