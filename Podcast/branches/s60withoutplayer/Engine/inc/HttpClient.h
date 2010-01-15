/*
* Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
*
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* EmbedDev AB - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <http/rhttpsession.h>
#include "HttpClientObserver.h"
#include "HttpEventHandler.h"
#include "PodcastModel.h"
#include "es_sock.h"

_LIT8(KUserAgent, "Podcasting/Symbian");
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

