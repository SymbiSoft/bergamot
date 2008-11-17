#include <badesca.h>
#include <eikchlst.h>
#include <QikScrollableContainer.h>
#include <QikRowLayoutManager.h>
#include <QikGridLayoutManager.h>
#include <QikStockControls.hrh>
#include <QikBuildingBlock.h>
#include <QikCommand.h>
#include "SyncClient.hrh"
#include "SyncClientView.h"
#include <SyncClient.rsg>
#include <QikGridLayoutManager.h>
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>
#include "SyncServerTimer.h"
#include "f32file.h"
#include "EIKDIALG.H"
#include <BAUTILS.H>
#include "debug.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CSyncClientView object
*/
CSyncClientView* CSyncClientView::NewLC(CQikAppUi& aAppUi)
	{
	CSyncClientView* self = new (ELeave) CSyncClientView(aAppUi);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/**
Constructor for the view.
Passes the application UI reference to the construction of the super class.

KNullViewId should normally be passed as parent view for the applications 
default view. The parent view is the logical view that is normally activated 
when a go back command is issued. KNullViewId will activate the system 
default view. 

@param aAppUi Reference to the application UI
*/
CSyncClientView::CSyncClientView(CQikAppUi& aAppUi) 
	: CQikViewBase(aAppUi, KNullViewId)
	{
	}

/**
Destructor for the view
*/
CSyncClientView::~CSyncClientView()
	{
	serverSession.Close();
	}

/**
2nd stage construction of the view.
*/
void CSyncClientView::ConstructL()
	{
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	}

/**
Returns the view Id

@return Returns the Uid of the view
*/
TVwsViewId CSyncClientView::ViewId()const
	{
	return TVwsViewId(KUidSyncClientID, KUidListBoxListView);
	}

/**
Handles all commands in the view.
Called by the UI framework when a command has been issued.
The command Ids are defined in the .hrh file.

@param aCommand The command to be executed
@see CQikViewBase::HandleCommandL
*/
void CSyncClientView::HandleCommandL(CQikCommand& aCommand)
	{
	switch(aCommand.Id())
		{
		// Just issue simple info messages to show that
		// the commands have been selected
		default:
			CQikViewBase::HandleCommandL(aCommand);
			break;
		}
	}

void CSyncClientView::CreateNoItemsLabel(CQikScrollableContainer* container) {
	DP("CreateNoItemsLabel START");
	CQikBuildingBlock* block = CQikBuildingBlock::CreateSystemBuildingBlockL(EQikCtOnelineBuildingBlock);
	container->AddControlLC(block, EMyViewBuildingBlockBase);
	block->ConstructL();
	block->SetUniqueHandle(EMyViewBuildingBlockBase);
	block->SetDividerBelow(EFalse);
	block->SetDimmed(ETrue);
	
	
	HBufC *str = iEikonEnv->AllocReadResourceLC(R_NO_ITEMS);
	block->SetCaptionL(*str, EQikItemSlot1); //the slot ids are defined in qikon.hrh
	CleanupStack::PopAndDestroy(str);
	CleanupStack::Pop(block);
	DP("CreateNoItemsLabel END");
}

void CSyncClientView::CreateChoiceListItem(CQikScrollableContainer* container, int id, TDesC16 &caption, int state ) {
	DP("CreateChoiceListItem START");
	// Check if control already exists
	if (container->Controls().Find(EMyViewBuildingBlockBase+id).IsValid() == (int) ETrue) {
		return;
	}
	// Create the System Building Block and set the caption.
		
	CQikBuildingBlock* block = CQikBuildingBlock::CreateSystemBuildingBlockL(EQikCtCaptionedTwolineBuildingBlock);
	container->AddControlLC(block, EMyViewBuildingBlockBase+id);
	block->ConstructL();
	block->SetUniqueHandle(EMyViewBuildingBlockBase+id);
	block->SetDividerBelow(ETrue);
	block->SetCaptionL(caption, EQikItemSlot1); //the slot ids are defined in qikon.hrh
	
	CEikChoiceList* chlst = new (ELeave) CEikChoiceList();
	block->AddControlLC(chlst, EQikItemSlot2);
	
	chlst->ConstructL(0 /*flags*/, 0 /*maxDisplayChar*/);
	chlst->SetObserver(this);
	CDesCArrayFlat* array = new(ELeave) CDesCArrayFlat(7);
	CleanupStack::PushL(array);
	
	HBufC *str = NULL;
	str = iEikonEnv->AllocReadResourceLC(R_OPTION_0);
	array->AppendL(*str);
	CleanupStack::PopAndDestroy(str);
	str = iEikonEnv->AllocReadResourceLC(R_OPTION_1);
	array->AppendL(*str);
	CleanupStack::PopAndDestroy(str);
	str = iEikonEnv->AllocReadResourceLC(R_OPTION_2);
	array->AppendL(*str);
	CleanupStack::PopAndDestroy(str);
	str = iEikonEnv->AllocReadResourceLC(R_OPTION_3);
	array->AppendL(*str);
	CleanupStack::PopAndDestroy(str);
	str = iEikonEnv->AllocReadResourceLC(R_OPTION_4);
	array->AppendL(*str);
	CleanupStack::PopAndDestroy(str);
	str = iEikonEnv->AllocReadResourceLC(R_OPTION_5);
	array->AppendL(*str);
	CleanupStack::PopAndDestroy(str);
	str = iEikonEnv->AllocReadResourceLC(R_OPTION_6);
	array->AppendL(*str);
	CleanupStack::PopAndDestroy(str);

	chlst->SetArrayL(array);
	CleanupStack::Pop(array);
	DP1("Setting handle to: %d", EMyViewChoiceListBase+id);
	chlst->SetUniqueHandle(EMyViewChoiceListBase+id);

	chlst->SetCurrentItem(state);
	chlst->SetObserver(this);
	CleanupStack::Pop(chlst);
	CleanupStack::Pop(block);
	DP("CreateChoiceListItem END");
}

void CSyncClientView::ShowSyncProfiles(CQikScrollableContainer* container) {
	DP("ShowSyncProfiles BEGIN");
	RSyncMLSession session;
	
	TInt error;
	TRAP(error, session.OpenL());
	if (error!=KErrNone) {
		DP("OpenL error");
	}

	RSyncMLDataSyncJob job;
	
	RArray<TSmlProfileId> profiles;
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
		CCoeControlArray &array = iContainer->Controls();
	
		// this causes crashes when the container tries to refocus the control
		//iContainer->ResetAndDestroy();
	    // so instead we remove them manually
		for (int i=0;i<array.Count();i++) {
			DP("Removing control");
			iContainer->RemoveControl(*(array.At(i).iControl));
		}
		
		DP1("profiles.Count()=%d", profiles.Count());
		if (profiles.Count() == 0) {
			CreateNoItemsLabel(container);
		} else {
			for (int i=0;i<profiles.Count();i++) {		
					
				RSyncMLDataSyncProfile profile;
				DP1("Reading profile %d", profiles[i]);
				TRAPD(error,profile.OpenL(session, profiles[i],ESmlOpenRead));
				if (error!=KErrNone)
					{
						DP1("profile.OpenL error: %d", error);
					}
			
				TRAP(error, profile.DisplayName());
				if (error!=KErrNone)
					{
					DP1("profile.DisplayName error: %d", error);
					}
				DP("before serverSession.GetTimer");
				
				TSyncServerPeriod period = serverSession.GetTimer(profiles[i]);
				DP2("DisplayName: %S, period=%d", &profile.DisplayName(), period);
		
				if(profile.DisplayName().Compare(_L("iSync")) != 0) {
					int selection = 0;
					if (period != -1) {
						selection = period;
					}
					CreateChoiceListItem(container, profiles[i], (TDesC &)profile.DisplayName(), selection);
				}
		
				profile.Close();
		
			}
		}
	} else {
		DP("Nothing new to show");
	}
	session.Close();
	DP("ShowSyncProfiles END");
}

void CSyncClientView::ViewConstructL()
    {
    	ViewConstructFromResourceL(R_SYNCCLIENT_BASEVIEW_UI_CONFIGURATIONS);
    	HBufC *navitext = iEikonEnv->AllocReadResourceLC(R_NAVI_TEXT);
    	ViewContext()->AddTextL(ESwimContextLabel, *navitext, EHCenterVCenter);
    	CleanupStack::PopAndDestroy(navitext);

        // Give a layout manager to the view
        CQikGridLayoutManager* gridlayout = CQikGridLayoutManager::NewLC();
        SetLayoutManagerL(gridlayout);
        CleanupStack::Pop(gridlayout);
                            
        // Create a container and add it to the view
        CQikScrollableContainer* container = new (ELeave) CQikScrollableContainer();
        Controls().Reset();
        Controls().AppendLC(container);
        container->ConstructL(EFalse);
        CleanupStack::Pop(container);
        iContainer = container;
                            
        // Create a layout manager to be used inside the container
        CQikRowLayoutManager* rowlayout = CQikRowLayoutManager::NewLC();
        container->SetLayoutManagerL(rowlayout);
        CleanupStack::Pop(rowlayout);        
        
        User::LeaveIfError(serverSession.Connect());
    }

void CSyncClientView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
	DP("ViewActivatedL START");

	iContainer->BeginUpdateLC();
	ShowSyncProfiles(iContainer);
	iContainer->EndUpdateL();

	CQikViewBase::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
	DP("ViewActivatedL END");
	}

void CSyncClientView::HandleControlEventL(CCoeControl *aControl, TCoeEvent aEventType) {
	if(aEventType == EEventStateChanged)
	{
		int profileId = aControl->UniqueHandle() - EMyViewChoiceListBase;
		CEikChoiceList* chlst = (CEikChoiceList*) aControl; //LocateControlByUniqueHandle<CEikChoiceList>(aControl->UniqueHandle());
		DP1("Handle: %d", aControl->UniqueHandle());
		TSyncServerPeriod period = (TSyncServerPeriod) chlst->CurrentItem();
		DP2("HandleControlEvent for profileNum=%d, item=%d", profileId, period);
		serverSession.SetTimer(profileId,period);
	}
}
