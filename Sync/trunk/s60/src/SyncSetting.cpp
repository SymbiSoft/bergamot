#include "SyncSetting.h"
#include "debug.h"

CSyncSetting::CSyncSetting(TInt aResourceId, TUint aProfileId, RSyncServerSession &aSession) :
	CAknEnumeratedTextPopupSettingItem(aResourceId, iValue), iValue(0), iProfileId(aProfileId), iSession(aSession)
	{
	}

CSyncSetting::~CSyncSetting()
	{
	}

void CSyncSetting::CompleteConstructionL()
	{
	// Complete construction
	CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();
	LoadL();
	}

void CSyncSetting::SetValue(TUint value) 
	{
	iValue = value;
	LoadL();
	}

void CSyncSetting::EditItemL(TBool aCalledFromMenu)
	{
	DP("CSyncSetting::EditItemL BEGIN");
	CAknEnumeratedTextPopupSettingItem::EditItemL(aCalledFromMenu);
	StoreL();
	DP2("External value toggled (StoreL), internal=%d, external=%d",InternalValue(), ExternalValue());
	DP("CSyncSetting::EditItemL END");
	}

void CSyncSetting::HandleSettingPageEventL(CAknSettingPage* aSettingPage, TAknSettingPageEvent aEventType)
	{
	CAknSettingItem::HandleSettingPageEventL(aSettingPage, aEventType);
	if (aEventType == EEventSettingOked) 
		{
		DP2("InternalValue=%d, ExternalValue=%d", InternalValue(), ExternalValue());
		StoreL();
		iSession.SetTimer(iProfileId, (TSyncServerPeriod) QueryValue()->CurrentValueIndex());
		}
	}

TInt CSyncSetting::HandleSetting()
	{
	return ETrue; 
	}
