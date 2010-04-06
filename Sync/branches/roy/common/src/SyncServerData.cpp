#include "SyncServerData.h"
#include "debug.h"

void CSyncServerData::ExternalizeL(RWriteStream& aStream) const {
	aStream.WriteInt32L(profileId);
	aStream.WriteInt32L(period);
	//DP2("Wrote %d, %d", profileId, period);
}

void CSyncServerData::InternalizeL(RReadStream& aStream) {
	profileId = aStream.ReadInt32L();
	period = (TSyncServerPeriod) aStream.ReadInt32L();
	timer = NULL;
	//DP2("Read %d, %d", profileId, period);
}

