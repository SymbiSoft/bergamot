// INCLUDE FILES
#include <coemain.h>
#include <SyncClientS60.rsg>
#include "SyncClientS60.hrh"
#include "SyncClientAppView.h"
#include <eikfrlbd.h>
#include "debug.h"
#include "SyncSetting.h"
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>

// ================= MEMBER FUNCTIONS =======================

// Constructor
CSyncClientAppView::CSyncClientAppView() 
    {
    }

// Destructor
CSyncClientAppView::~CSyncClientAppView()
    {
    serverSession.Close();
	delete iItemList;
    }

// ----------------------------------------------------
// CSyncClientAppView::ConstructL()
// Symbian OS default constructor can leave.
// ----------------------------------------------------
//
void CSyncClientAppView::ConstructL(const TRect& aRect)
    {
    // Create a window for this application view
    CreateWindowL();

    DP("Before serverSession.Connect");
    User::LeaveIfError(serverSession.Connect());
    DP("After serverSession.Connect");

   	iItemList = new (ELeave) CAknSettingItemList;
   	iItemList->SetMopParent(this);
    iItemList->ConstructFromResourceL(R_ENTRY_SETTINGS_LIST); 
    
    LoadListL();
    
    iItemList->MakeVisible(ETrue);
    iItemList->SetRect(aRect);
    iItemList->ActivateL();
    iItemList->ListBox()->UpdateScrollBarsL();
    iItemList->DrawNow();    

    // Set the windows size
    SetRect(aRect);

    // Activate the window, which makes it ready to be drawn
    ActivateL();
    }

// ----------------------------------------------------
// CSyncClientAppView::Draw()
// This function is used for window server-initiated 
// redrawing of controls, and for some 
// application-initiated drawing.
// ----------------------------------------------------
//
void CSyncClientAppView::Draw(const TRect& /*aRect*/) const
    {
    // Get the standard graphics context 
    CWindowGc& gc = SystemGc();
    
    // Gets the control's extent
    TRect rect = Rect();
    
    // Clears the screen
    gc.Clear(rect);
    }

// ----------------------------------------------------
// CSyncClientAppView::CountComponentControls()
// Gets the number of controls contained in a compound 
// control. 
// ----------------------------------------------------
//
TInt CSyncClientAppView::CountComponentControls() const
	{
	TInt count = 0;
	if (iItemList)
		count++;
	return count;
	}
	
// ----------------------------------------------------
// CSyncClientAppView::ComponentControl()
// Gets the specified component of a compound control.
// ----------------------------------------------------
//
CCoeControl* CSyncClientAppView::ComponentControl(TInt /*aIndex*/) const
	{
	return iItemList;
	}
	
// ----------------------------------------------------
// CSyncClientAppView::OfferKeyEventL()
// When a key event occurs, the control framework calls 
// this function for each control on the control stack, 
// until one of them can process the key event 
// (and returns EKeyWasConsumed).
// ----------------------------------------------------
//	
TKeyResponse CSyncClientAppView::OfferKeyEventL(	const TKeyEvent& aKeyEvent, 
																TEventCode aType )
	{
    if(aType != EEventKey)
        {
        return EKeyWasNotConsumed;
        }    
    else if(iItemList)
        {
        return iItemList->OfferKeyEventL( aKeyEvent, aType );
        }
    else
        {
        return EKeyWasNotConsumed;
        }
	
	}

void CSyncClientAppView::CreateChoiceListItem(int aId, const TPtrC16 &aCaption, int aValue ) 
	{
	DP("CreateChoiceListItem START");

	TBool isNumberedStyle = iItemList->IsNumberedStyle();
	CArrayPtr<CGulIcon>* icons = iItemList->ListBox()->ItemDrawer()->FormattedCellData()->IconArray();
	CSyncSetting* item = new (ELeave) CSyncSetting(ESyncSettingItem+aId, aId, serverSession);
	CleanupStack::PushL(item);
	// The same resource id can be used for multiple enumerated text setting pages.
	item->ConstructL(isNumberedStyle, aId, aCaption, icons, R_ENUMERATEDTEXT_SETTING_PAGE, -1, 0, R_POPUP_SETTING_TEXTS);
	iItemList->SettingItemArray()->AppendL(item);
	
	if (aValue == -1) {
	} else {
		item->SetValue(aValue);
	}
	
	CleanupStack::Pop(item);

	DP("CreateChoiceListItem END");
}

void CSyncClientAppView::ShowSyncProfiles() {
#ifdef __WINS__

	CreateChoiceListItem(0, _L("Test profile 1"), 0);
	CreateChoiceListItem(1, _L("Test profile 2"), 0);
	CreateChoiceListItem(2, _L("Test profile 3"), 0);
#else
	DP("ShowSyncProfiles BEGIN");
	RSyncMLSession session;
	DP("After session");
	TInt error;
	TRAP(error, session.OpenL());
	DP("After OpenL");
	if (error!=KErrNone) {
		DP1("OpenL error %d", error);
		return;
	}
	DP("Before job");
	RSyncMLDataSyncJob job;
	DP("After job");
	RArray<TSmlProfileId> profiles;
	DP("Before ListProfilesL");
	TRAP(error, session.ListProfilesL(profiles, ESmlDataSync));
	if (error!=KErrNone)
		{
		DP("ListProfilesL error");
		}
	
	TInt numItems = profiles.Count();
	DP1("Found %d SyncML profiles", numItems);
	TBuf<255> Buffer;
	TBool mustRedraw = EFalse;
	
	if (profiles.Count() == 0 || profiles.Count() != lastViewProfiles.Count()) {
		DP("Profile count mismatch, clearing list");
		mustRedraw = ETrue;
	} else {
		for (int i=0;i<profiles.Count();i++) {
			if (lastViewProfiles.Find(profiles[i]) == KErrNotFound) {
				DP1("Did not find new profile %d, clearing list", profiles[i]);
				mustRedraw = ETrue;
				break;
			}
		}
	}
	
	if (mustRedraw) {
		DP("mustRedraw");
		lastViewProfiles = profiles;

		DP1("profiles.Count()=%d", profiles.Count());
		if (profiles.Count() > 0) {
			for (int i=0;i<profiles.Count();i++) {		
					
				RSyncMLDataSyncProfile profile;
				DP1("Reading profile %d", profiles[i]);
				TRAPD(error,profile.OpenL(session, profiles[i],ESmlOpenRead));
				if (error!=KErrNone)
					{
						DP1("profile.OpenL error: %d", error);
						continue;
					}
			
				TRAP(error, profile.DisplayName());
				if (error!=KErrNone)
					{
					DP1("profile.DisplayName error: %d", error);
					}
				DP("before serverSession.GetTimer");
				
				TSyncServerPeriod period = serverSession.GetTimer(profiles[i]);
				DP2("DisplayName: %S, period=%d", &profile.DisplayName(), period);
		
				if(profile.DisplayName().Compare(_L("PC Suite")) != 0 &&
					profile.DisplayName().Compare(_L("Lifeblog")) != 0) {
					int selection = 0;
					if (period != -1) {
						selection = period;
					}
					CreateChoiceListItem(profiles[i], profile.DisplayName(), selection);
				}
		
				profile.Close();
		
			}
		}
	} else {
		DP("Nothing new to show");
	}
	session.Close();
#endif
	DP("ShowSyncProfiles END");
}

// ----------------------------------------------------
// CSyncClientAppView::LoadListL()
// Loads the setting item list dynamically.
// ----------------------------------------------------
//
void CSyncClientAppView::LoadListL()
	{

	ShowSyncProfiles();
	
	// Required when there is only one setting item.
	iItemList->SettingItemArray()->RecalculateVisibleIndicesL();

	iItemList->HandleChangeInItemArrayOrVisibilityL();
	//iItemList->LoadSettingsL();
	}

// ----------------------------------------------------
// CSyncClientAppView::StoreSettingsL()
// Stores the settings of the setting list.
// ----------------------------------------------------
//	
void CSyncClientAppView::StoreSettingsL()
	{
	iItemList->StoreSettingsL();
	}
		
