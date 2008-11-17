#ifndef SYNCSERVERDATA_H_
#define SYNCSERVERDATA_H_

#include <S32STRM.H>
#include "SyncServerTimer.h"

enum TSyncServerPeriod
{
	ENever = 0,
	EFifteenMinutes,
	EOneHour,
	EFourHours,
	ETwelveHours,
	EDaily,
	EWeekly
};

class CSyncServerData {
public:
	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);

public:
	TSmlProfileId profileId;
	TSyncServerPeriod period;
	CSyncServerTimer* timer;
};

#endif
