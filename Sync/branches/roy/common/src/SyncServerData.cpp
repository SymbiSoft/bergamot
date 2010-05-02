#include "SyncServerData.h"
#include "debug.h"

void CSyncServerData::ExternalizeL(RWriteStream& aStream) const {
	aStream.WriteInt32L(profileId);
	aStream.WriteInt32L(period);
	aStream.WriteInt32L(day);
	aStream.WriteInt32L(hour);
	aStream.WriteInt32L(minute);
	//DP2("Wrote %d, %d", profileId, period);
}

void CSyncServerData::InternalizeL(RReadStream& aStream) {
	profileId = aStream.ReadInt32L();
	period = (TSyncServerPeriod) aStream.ReadInt32L();
	day = (TDay) aStream.ReadInt32L();
	hour = aStream.ReadInt32L();
	minute = aStream.ReadInt32L();
	timer = NULL;
	//DP2("Read %d, %d", profileId, period);
}

