#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <http/rhttpsession.h>
#include "HttpClientObserver.h"
#include "HttpEventHandler.h"
#include "PodcastModel.h"
#include "es_sock.h"

#ifdef UIQ3
_LIT8(KUserAgent, "Escarpod/UIQ");
#else
_LIT8(KUserAgent, "Escarpod/S60");
#endif
_LIT8(KAccept, "*/*");

class CHttpClient : public CBase
{
public:
	virtual ~CHttpClient();
	static CHttpClient* NewL(CPodcastModel& aPodcastModel, MHttpClientObserver& aResObs);
	TBool GetL(const TDesC& url, const TDesC& fileName, TBool aSilent = EFalse);
	void Stop();
  	TBool IsActive();
	void ClientRequestCompleteL(TBool aSuccessful);
	void SetResumeEnabled(TBool aEnabled);

private:
	CHttpClient(CPodcastModel& aPodcastModel, MHttpClientObserver& aResObs);
	static CHttpClient* NewLC(CPodcastModel& aPodcastModel, MHttpClientObserver& aResObs);
	void ConstructL();
	void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, const TDesC8& aHdrValue);

private:
	RHTTPSession iSession;	
	TBool iIsActive;
	RHTTPTransaction iTrans;
	CHttpEventHandler* iHandler;
	TBool iResumeEnabled;
	CPodcastModel& iPodcastModel;
	MHttpClientObserver& iObserver;
	TInt iTransactionCount;
};
#endif

