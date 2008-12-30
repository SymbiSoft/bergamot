#ifndef PODCASTUTILS_H_
#define PODCASTUTILS_H_

#include <e32cmn.h>

_LIT(KURLPrefix, "http://");
_LIT(KItpcPrefix, "itpc://");
_LIT(KPcastPrefix, "pcast://");			

class PodcastUtils
	{
public:
	static void FixProtocols(TDes &aUrl);
	
	};

#endif /* PODCASTUTILS_H_ */
