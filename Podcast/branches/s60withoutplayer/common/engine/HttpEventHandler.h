#ifndef HTTPEVENTHANDLER_H
#define HTTPEVENTHANDLER_H

#include <e32base.h>
#include <http/mhttpdatasupplier.h>
#include <http/rhttpsession.h>
#include <http/rhttpheaders.h>
#include <http/mhttptransactioncallback.h>
#include <httpstringconstants.h>
#include "HttpClientObserver.h"

const TInt KMaxSubmitSize = 1024;
const TInt KMaxHeaderNameLen = 32;
const TInt KMaxHeaderValueLen = 128;
const TInt KByteOverlap = 4096;

class CHttpEventHandler : public CBase,
                          public MHTTPTransactionCallback
  {
  public:
	virtual ~CHttpEventHandler();
	static CHttpEventHandler* NewLC(CHttpClient* aClient, MHttpClientObserver &aCallbacks);
	static CHttpEventHandler* NewL(CHttpClient* aClient, MHttpClientObserver &aCallbacks);
	void SetSaveFileName(const TDesC &fName,TBool aContinue=EFalse);
	void CloseSaveFile();	
	// from MHTTPTransactionCallback
	virtual void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
	virtual TInt MHFRunError(TInt aError, RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
	void SetSilent(TBool aSilent);

	
protected:
	CHttpEventHandler(CHttpClient* aClient, MHttpClientObserver &aCallbacks);
	void ConstructL();
	
private:
	void DumpRespHeadersL(RHTTPTransaction& aTrans);
	void DumpRespBody(RHTTPTransaction& aTrans);
	void DumpIt(const TDesC8& aData);

private:
	TBool iVerbose;
	TBool iSavingResponseBody;
	RFs iFileServ;
	RFile iRespBodyFile;
	TFileName iFileName;
	TParse iParsedFileName;
	MHTTPDataSupplier* iRespBody;	
	TInt iBytesDownloaded;
	TInt iBytesTotal;
	CHttpClient* iHttpClient;
	MHttpClientObserver& iCallbacks;
	TBool iSilent;
	TBool iContinue;
};

#endif
