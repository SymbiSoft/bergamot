#ifndef PODCASTUTILS_H_
#define PODCASTUTILS_H_

#include <e32cmn.h>

_LIT(KURLPrefix, "http://");
_LIT(KItpcPrefix, "itpc://");
_LIT(KPcastPrefix, "pcast://");			

class PodcastUtils
	{
public:
	IMPORT_C static void FixProtocolsL(TDes &aUrl);
	
	};

#endif /* PODCASTUTILS_H_ */
