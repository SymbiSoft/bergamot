#ifndef SYNCSERVERDATA_H_
#define SYNCSERVERDATA_H_

#include <S32STRM.H>
#include "SyncServerTimer.h"

enum TSyncServerPeriod
{
	ENever = 0,
	EInterval,
	EFifteenMinutes,
	EOneHour,
	EFourHours,
	ETwelveHours,
	EDaily,
	EWeekly
};

struct TSyncProfileDetails
{
	int period;
	TDay day;
	TInt hour;
	TInt minute;
};

class CSyncServerData {
public:
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);

public:
	TSmlProfileId profileId;
	TSyncServerPeriod period;
	CSyncServerTimer* timer;
	TDay day;
	TInt hour;
	TInt minute;
};

#endif
