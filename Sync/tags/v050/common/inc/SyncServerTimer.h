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
	
private:
	TTime timeToSync;
	TSmlProfileId profileId;
	int thePeriod;
//	RMutex mutex;
};
#endif

