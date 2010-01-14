#ifndef FEEDPARSEROBSERVER_H_
#define FEEDPARSEROBSERVER_H_

#include "ShowInfo.h"
#include "FeedInfo.h"

class MFeedParserObserver
{
public:
	virtual TBool NewShow(CShowInfo *item) = 0;
	virtual void ParsingComplete(CFeedInfo *item) = 0;
};

#endif
