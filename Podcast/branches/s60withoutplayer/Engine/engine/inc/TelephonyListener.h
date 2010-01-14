#ifndef TELEPHONYLISTENER_H
#define TELEPHONYLISTENER_H
//
// CTelephonyListener
// Listens for incoming phone calls and pauses playback

#include <Etel3rdParty.h>

class CPodcastModel;

enum TCallState
{
    EInit, 
    EWaiting,		
    ERinging,
    ECalling,
    EAnswering
};

const TUint KHangupInProcessTimeout = 12; // 12 seconds, enough for bluetooth

class CTelephonyListener : CActive
{
public:
	CTelephonyListener(CPodcastModel &aPodcastModel);
	~CTelephonyListener();
	static CTelephonyListener* NewL(CPodcastModel &aPodcastModel);
	void ConstructL();
	void StartL();
	void RunL();
	void DoCancel();
private:
	CPodcastModel &iPodcastModel;
	CTelephony* iTelephony;
    CTelephony::TCallStatusV1 iLineStatus;
    CTelephony::TCallStatusV1Pckg iLineStatusPckg;
    TBool iPaused;
};
#endif
