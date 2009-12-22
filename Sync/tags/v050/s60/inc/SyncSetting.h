#ifndef SYNCSETTING_H
#define SYNCSETTING_H

#include <aknsettingitemlist.h>
#include "RSyncServerSession.h"

class CSyncSetting: public CAknEnumeratedTextPopupSettingItem 
{ 
public:
	CSyncSetting(TInt aResourceId, TUint aProfileId, RSyncServerSession &aSession);
    ~CSyncSetting();

    void EditItemL(TBool aCalledFromMenu);
	void HandleSettingPageEventL(CAknSettingPage* aSettingPage, TAknSettingPageEvent aEventType);

	void CompleteConstructionL();
	void SetValue(TUint value);
	TInt HandleSetting();
protected:
    TInt iValue;
    TUint iProfileId;
    RSyncServerSession &iSession;
};

#endif
