#ifndef SYNCSERVER_H_
#define SYNCSERVER_H_

#include <e32base.h>
#include "SyncServerData.h"

const TUint KDefaultHeapSize=0x10000;
const TUint KDefaultMessageSlots=4;

_LIT(KConfigFileName, "config.dat");
_LIT(KSyncServerName, "SyncServer");

enum TSyncServerPanic
{
	EBadRequest,
	EGetTimer,
	ESetTimer,
	EMainSchedulerError,
	ECreateTrapCleanup,
	EStartServer
};

class CSyncServer : public CServer2
{
public:
	CSyncServer(CActive::TPriority aActiveObjectPriority);
	CSession2 * NewSessionL(const TVersion& aVersion,
	const RMessage2& aMessage) const;
	static TInt ThreadFunction(TAny *aParam);
	static void PanicServer(TSyncServerPanic aPanic);
public:
	void SetTimer(TSmlProfileId profileId, TSyncServerPeriod period, TDay day = ESunday, TInt hour = 0, TInt minute = 0);
	TSyncProfileDetails GetTimer(TSmlProfileId profileId);

public:
	void LoadSettingsL();
	void SaveSettings();
	
private:
	void RunL();
	RArray<CSyncServerData> timerArray;
};

#endif
