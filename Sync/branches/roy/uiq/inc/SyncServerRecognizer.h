#if !defined(__CSyncServerRecognizer_H__)
#define __CSyncServerRecognizer_H__

#include "e32base.h"
#if !defined(__APMREC_H__)
#include <apmrec.h>
#endif

class CAppStarter : public CActive
{
public:
	CAppStarter();
	void RunL();
	void DoCancel();
	
	void ConstructL();

private:
	RProcess iSysStartProcess;

};

class CSyncServerRecognizer: public CApaDataRecognizerType
	{
public:
	CSyncServerRecognizer();
    TUint PreferredBufSize();
	TDataType SupportedDataTypeL(TInt) const;   
    static CApaDataRecognizerType* CreateRecognizerL();
    void WaitForSysStartL();
private:
    void DoRecognizeL(const TDesC&, const TDesC8&);	
	CAppStarter iAppStarter;
	};

	
#endif


