// INCLUDE FILES
#include <coemain.h>
#include <SyncClientS60_0x2002C265.rsg>
#include "SyncClientS60.hrh"
#include "SyncClientAppView.h"
#include <eikfrlbd.h>
#include "debug.h"
#include "SyncSetting.h"
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>
#include <aknquerydialog.h>
//#include <CommDbConnPref.h>  // for IAP retrieval
//#include <commdb.h> // CCommsDatabase
//#include <ApSelect.h> //CApSelect
#include <apsettingshandlerui.h> 
#include <cdbcols.h>

const TInt KMaxIapListItemLength = 32;

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


void CSyncClientAppView::ViewDetails()
{
	RSyncMLSession session;
	DP("After session");
	TInt error;
	TRAP(error, session.OpenL());
	DP("After OpenL");
	if (error!=KErrNone) {
		DP1("OpenL error %d", error);
		return;
	}
	
	// Retrieve profiles
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
	
	// No profiles available
	if (profiles.Count() == 0) 
	{
		profiles.Close();
		session.Close();
		return;
	}
	else 
 	{
		TInt currentIndex = iItemList->ListBox()->CurrentItemIndex();
	
		TSyncProfileDetails sData = serverSession.GetTimer(profiles[currentIndex]);
				
		TDay iDay;
		TInt iHour;
		TInt iMinute;
		
		if (sData.period == EInterval || sData.period == EDaily)
			{
				iHour = sData.hour;
				iMinute = sData.minute;
			}
		else if (sData.period == EWeekly)
			{
				iDay = sData.day;
				iHour = sData.hour;
				iMinute = sData.minute;		
			}
		
		// Show Sync Profile name, type with details and next sync.

 	}
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

	// profiles holds the Sync Profiles from the phone
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
	
	// lastViewProfiles is RArray<TSmlProfileId> data member
	if (mustRedraw) {
		DP("mustRedraw");
		lastViewProfiles = profiles;

		DP1("profiles.Count()=%d", profiles.Count());
		if (profiles.Count() > 0) {
			for (int i=0;i<profiles.Count();i++) {		
					
				// for each profile in profiles
				RSyncMLDataSyncProfile profile;
				DP1("Reading profile %d", profiles[i]);
				TRAPD(error,profile.OpenL(session, profiles[i],ESmlOpenRead));
				if (error!=KErrNone)
					{
						DP1("profile.OpenL error: %d", error);
						continue;
					}
			
				// Checks to see if the profile has a display name
				TRAP(error, profile.DisplayName());
				if (error!=KErrNone)
					{
					DP1("profile.DisplayName error: %d", error);
					}
				DP("before serverSession.GetTimer");
				
				/////////////////////////////////////////////////////////////////////////////////
				// This is very important
				// We get the timer setting for this profile from our very own implementation
				// of serverSession
				/////////////////////////////////////////////////////////////////////////////////
				TSyncProfileDetails profDetails = serverSession.GetTimer(profiles[i]);
				//DP2("DisplayName: %S, period=%d", &profile.DisplayName(), period);
		
				// We only want to show Sync Profiles which are not PC Suite or Lifeblog
				if(profile.DisplayName().Compare(_L("PC Suite")) != 0 &&
					profile.DisplayName().Compare(_L("Lifeblog")) != 0) 
				{
					// Selection is the index of the selected enumerated value
					int selection = 0;
					
					if (profDetails.period != -1) 
					{
						selection = profDetails.period;
					}
					
					// For each profile we create a setting item for it
					// with the selection taken from the timer
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

void CSyncClientAppView::DeleteSync()
{
	RSyncMLSession session;
	DP("After session");
	TInt error;
	TRAP(error, session.OpenL());
	DP("After OpenL");
	if (error!=KErrNone) {
		DP1("OpenL error %d", error);
		return;
	}
	
	// Retrieve profiles
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
	
	// No profiles available
	if (profiles.Count() == 0) 
	{
		profiles.Close();
		session.Close();
		return;
	}
	else 
 	{
		//////////////////////////////////////////////////////////////////////////
		//							Localization of strings
		//////////////////////////////////////////////////////////////////////////
		TBuf<150> AreYouSureDeleteMessage;
	
		CEikonEnv *iEikEnv = CEikonEnv::Static();
		iEikEnv->ReadResource(AreYouSureDeleteMessage,R_ARE_YOU_SURE_DELETE);
		//////////////////////////////////////////////////////////////////////////
	
		CAknQueryDialog* dlg = CAknQueryDialog::NewL( );
		if(dlg->ExecuteLD(R_DELETE_CONFIRMATION_QUERY, AreYouSureDeleteMessage)) 
		{
			TInt currentIndex = iItemList->ListBox()->CurrentItemIndex();
			
			// syncmlclient.h
			
			// syncmlclientds.h has all the definitions for sync profiles
			// where one can set the "Tasks" list (what gets synced)
			
			// IMPORT_C void DeleteProfileL(TSmlProfileId);
			session.DeleteProfileL(profiles[currentIndex]);
		}
	}	
	
	profiles.Close();
	session.Close();	
}

void CSyncClientAppView::CreateNewProfile(TInt aCommand)
{
    RSyncMLDataSyncProfile profile;
    
	RSyncMLSession session;
	DP("After session");
	TInt error;
	TRAP(error, session.OpenL());
	DP("After OpenL");
	if (error!=KErrNone) {
		DP1("OpenL error %d", error);
		return;
	}
    
    // Creates an unnamed Ds profile
	profile.CreateL( session );
		
	TInt result;
	
	// To set profile display name
	TBuf<100> text;
	TBuf8<100> text8;
	text.Append(_L(""));
	CAknTextQueryDialog* dlg = CAknTextQueryDialog::NewL(text);
	
	// Sets T9 support
	dlg->SetPredictiveTextInputPermitted(ETrue);

	TBool canContinue = ETrue;
	
	if(dlg->ExecuteLD(R_PROFILE_NAME_DIALOG)) {	
		profile.SetDisplayNameL(text);
		
		// If Funambol, we need to ask the user for the server details
		if (aCommand == ENewProfileFunambol)
		{
			
			dlg = CAknTextQueryDialog::NewL(text);
			
			// Sets T9 support
			dlg->SetPredictiveTextInputPermitted(ETrue);

			if (!dlg->ExecuteLD(R_SERVER_ADDRESS_DIALOG))
				{
					canContinue = EFalse;
				}
			else
				{
					text8.Copy(text);
				}			
		}
		
		if (canContinue)
		{
			TBuf<100> heapUserName;
			TBuf<32> heapPassword;
	
			TBuf8<100> heapUserName8;
			TBuf8<32> heapPassword8;
					
			CAknMultiLineDataQueryDialog* regDlg = CAknMultiLineDataQueryDialog::NewL(heapUserName,heapPassword);
			
			
			
			if (aCommand == ENewProfileGoogleCalendar || aCommand == ENewProfileGoogleContacts)
			{
				result = regDlg->ExecuteLD(R_ENTER_REGISTRATION_GOOGLE_CALENDAR_DIALOG);
			}
			else
			{
				result = regDlg->ExecuteLD(R_ENTER_REGISTRATION_DIALOG);
			}

			if(result) 
			{
				heapUserName8.Copy(heapUserName);
				heapPassword8.Copy(heapPassword);
			
					
				// create and enable tasks (applications)
				// retrieve needed data provider ids
				TSmlDataProviderId contactsProvider = -1;
				TSmlDataProviderId calendarProvider = -1;
				TSmlDataProviderId notesProvider = -1;
				
				TInt err = KErrNone;
				
				RArray<TSmlDataProviderId> dataProvidersArr;
				TRAP(err,session.ListDataProvidersL(dataProvidersArr));
				User::LeaveIfError(err);
				TInt count = dataProvidersArr.Count();
			
				// Fills the list of LOCAL dataProviders (Contacts, Calendar and Notes)
				 for(int i=0;i<count;i++)
				 {
					 RSyncMLDataProvider dataProvider;
					 TRAP(err,dataProvider.OpenL(session,dataProvidersArr[i]));
					 
					 User::LeaveIfError(err);
					
					 if(dataProvider.DefaultDataStoreName().Compare(CALENDAR_DATA_STORE_NAME)==0) 
					 {
						calendarProvider = dataProvider.Identifier();
					 }
					 else if(dataProvider.DefaultDataStoreName().Compare(CONTACTS_DATA_STORE_NAME)==0) 
					 {
						 contactsProvider = dataProvider.Identifier();
					 }
					 else if(dataProvider.DefaultDataStoreName().Compare(NOTES_DATA_STORE_NAME)==0) 
					 {
						 notesProvider = dataProvider.Identifier();
					 }
					 
					 dataProvider.Close();
				 }
				 dataProvidersArr.Reset();
				
				// To set User name
				profile.SetUserNameL(heapUserName8);
				// To set password
				profile.SetPasswordL(heapPassword8);
				
				// To set Protocol Version to be used when synchronizing
				TSmlProtocolVersion protocol = ESmlVersion1_2;
				profile.SetProtocolVersionL( protocol );
				
				// To sets the server-alerted notification user interaction.
				TSmlServerAlertedAction action = ESmlEnableSync;
				profile.SetSanUserInteractionL( action );	
				
				TSmlCreatorId creatorId(_UID3);
			
				profile.SetCreatorId(creatorId);
				
				// To save modifications
				profile.UpdateL();
				
				// IapName is the internet access point that the user wil use
				
				////////////////////////////////////////////////////////////////////////////////
				// I could add to give a list of options for this ("Ask", "Default" - or show 
				// list of IAPs to choose from).  Currently it is Default.
				////////////////////////////////////////////////////////////////////////////////
				TBuf<50> iIapName(_L("Default"));
			
				if (aCommand == ENewProfileGoogleCalendar)
				{
					TBuf<50> iServerId(_L("Calendar"));
					
					// ServerId is the NAME OF THE SERVER DATA SOURCE TO BE SYNCED AGAINST
					// IT IS NOT THE URL OF THE SERVER
					SetDataProvider(profile, calendarProvider , iServerId, ESyncCalendar);
					
					TBuf8<200> iServer(_L8("https://syncme.lorenc.se"));
					SetServer(profile, iServer, iIapName);
				}
				else if (aCommand == ENewProfileGoogleContacts)
				{
					TBuf<50> iServerId(_L("Google"));
					SetDataProvider(profile, contactsProvider , iServerId, ESyncContacts);
					
					TBuf8<200> iServer(_L8("https://m.google.com/syncml"));
					SetServer(profile, iServer, iIapName);					
				}
				else if (aCommand == ENewProfileOvi)
				{
					TBuf<50> iServerId(_L("./EventTask/Tasks"));
					SetDataProvider(profile, calendarProvider , iServerId, ESyncCalendar);
			
					iServerId.Copy(_L("./Contact/Unfiled"));
					SetDataProvider(profile, contactsProvider , iServerId, ESyncContacts);
	
					iServerId.Copy(_L("./Note/Unfiled"));
					SetDataProvider(profile, notesProvider , iServerId, ESyncNotes);
					
					TBuf8<200> iServer(_L8("https://sync.ovi.com/services/syncml"));
					SetServer(profile, iServer, iIapName);					
				}
				else if (aCommand == ENewProfileZyb)
					{
				
					}
			
				else if (aCommand == ENewProfileFunambol)
					{
						
						TBuf<50> iServerId(_L("cal"));
						SetDataProvider(profile, calendarProvider , iServerId, ESyncCalendar);
				
						iServerId.Copy(_L("card"));
						SetDataProvider(profile, contactsProvider , iServerId, ESyncContacts);
			
						iServerId.Copy(_L("note"));
						SetDataProvider(profile, notesProvider , iServerId, ESyncNotes);
						
						// text8 variable holds the server
						SetServer(profile, text8, iIapName);			
				
					}				
				else if (aCommand == ENewProfileMobical)
					{
						// MobiCal allows to sync also SMS but that is not yet supported
						TBuf<50> iServerId(_L("caltask"));
						SetDataProvider(profile, calendarProvider , iServerId, ESyncCalendar);
				
						iServerId.Copy(_L("con"));
						SetDataProvider(profile, contactsProvider , iServerId, ESyncContacts);
	
						iServerId.Copy(_L("pnote"));
						SetDataProvider(profile, notesProvider , iServerId, ESyncNotes);
						
						TBuf8<200> iServer(_L8("http://www.mobical.net/sync/server"));
						SetServer(profile, iServer, iIapName);				
				
					}							

						
				// close profile
				profile.Close();
				
				CEikonEnv::InfoWinL(_L("New Profile "),_L("Created"));	
			}	// username and pass
		} // canContinue
	} // profile name
	
	session.Close();
}

void CSyncClientAppView::SetDataProvider(RSyncMLDataSyncProfile& profile, TSmlDataProviderId& iProvider, const TDesC& serverURI,TSyncApp aSyncApp)
{
	TSmlCreatorId creatorId(_UID3);
	TInt err = KErrNone;
	
	if(iProvider == -1)
		return;
	
	 switch(aSyncApp)
	 {
		 case ESyncCalendar:
		 {
			// We set the "Calendar" app to this one
			RSyncMLTask calendarTask;
			TRAP(err, calendarTask.CreateL(profile, iProvider, serverURI, CALENDAR_DATA_STORE_NAME));
			// CleanupStack::PopAndDestroy(calURIBuf);
			User::LeaveIfError(err);
			calendarTask.SetCreatorId(creatorId);
			
			TRAP(err, {
			calendarTask.SetEnabledL(ETrue);
			calendarTask.SetDisplayNameL(_L("Calendar"));
			calendarTask.SetDefaultSyncTypeL(ESmlTwoWay);
			calendarTask.SetFilterMatchTypeL(ESyncMLMatchDisabled);
			calendarTask.UpdateL();
			});
			User::LeaveIfError(err);
			
			calendarTask.Close();
			 break;
		 }
		 case ESyncContacts:
		 {
		  // default Funambol server source is "card"
		  RSyncMLTask contactsTask;
		 
		  TRAP(err, contactsTask.CreateL(profile, iProvider, serverURI, CONTACTS_DATA_STORE_NAME));

		  User::LeaveIfError(err);
		  contactsTask.SetCreatorId(creatorId);
		 
		  TRAP(err, {
		  contactsTask.SetEnabledL(ETrue);
		  contactsTask.SetDisplayNameL(_L("Contacts"));
		  contactsTask.SetDefaultSyncTypeL(ESmlTwoWay);
		  contactsTask.SetFilterMatchTypeL(ESyncMLMatchDisabled);
		 
		  contactsTask.UpdateL();
		  });
		  User::LeaveIfError(err);
		  contactsTask.Close();			 
			 break;
		 }
		 case ESyncNotes:
		 {
		 RSyncMLTask notesTask;

		 TRAP(err, notesTask.CreateL(profile, iProvider, serverURI, NOTES_DATA_STORE_NAME));

		 User::LeaveIfError(err);
		 notesTask.SetCreatorId(creatorId);

		 TRAP(err, {
		 notesTask.SetEnabledL(ETrue);
		 notesTask.SetDisplayNameL(_L("Notes"));
		 notesTask.SetDefaultSyncTypeL(ESmlTwoWay);
		 notesTask.SetFilterMatchTypeL(ESyncMLMatchDisabled);
		 notesTask.UpdateL();
		 });
		 User::LeaveIfError(err);
		 notesTask.Close();			 
		 	break;
		 }
		 default:
		 {
			// User::Leave(KErrWrongSyncApp);
			 break;
		 }
	 }	
	 
	// Update the profile
	TRAP(err,profile.UpdateL());
	User::LeaveIfError(err);
}

void CSyncClientAppView::SetServer(RSyncMLDataSyncProfile& profile, const TDesC8& aServerURI, const TDesC& aIapName)
{	
	TInt err = KErrNone;
	
	// Set server URI and IapId
	// ------------------------
	// if NSmlIapId = -1, user is asked for IAP
	// otherwise it is set to the specified IapId
	// useful property names: NSmlIapId,NSmlIapId2,NSmlIapId3,NSmlIapId4,
	// NSmlUseRoaming,NSmlHTTPAuth,NSmlHTTPUsername,NSmlHTTPPassword
	RArray<TSmlConnectionId> connArr;
	TRAP(err,profile.ListConnectionsByConnectionIdL(connArr));
	User::LeaveIfError(err);
	TInt count2 = connArr.Count();
	for(int i=0;i<count2;i++)
	{
		RSyncMLConnection syncConnection;
		TRAP(err,syncConnection.OpenByConnectionIdL(profile,connArr[i]));
		User::LeaveIfError(err);
		setIapIdForConnPropertiesL(aIapName);
	
		TRAP(err, {
			syncConnection.SetPriority(0);
			syncConnection.SetRetryCount(2);
			syncConnection.SetServerURIL(aServerURI);
			syncConnection.SetPropertyL(_L8("NSmlIapId"),iIapId);
			syncConnection.UpdateL();
		});
		User::LeaveIfError(err);
		syncConnection.Close();
	}
	connArr.Reset();
	
	// Update the profile
	TRAP(err,profile.UpdateL());
	User::LeaveIfError(err);
}

/**
 * CSyncProfileManager::SetIapIdForConnProperties()
 * Set the private member iIapId to the IAP id of the specified IAP.
 * It is used by CreateProfileL().
 * If aIapName is set to "Ask" or empty, then the user will be asked every time;
 * if aIapName is set to <iapname>, then that particular IAP will be used,
 * if the IAP is not found, the default IAP is set;
 * if aIapName is set to Default, the default IAP set into packet data pref
 * is used. NOTE: the default set is a static set, if user change the
 * default IAP, the change is not propagated to the SyncML profile. 
 */
void CSyncClientAppView::setIapIdForConnPropertiesL(const TDesC& aIapName)
{
    
    if ( !aIapName.Compare(_L("Ask")) || !aIapName.Compare(_L("")) )
    {
        iIapId.Zero();
        iIapId.Copy(_L8("-1"));
    } 
    else if(aIapName.Compare(_L("Default"))==0)
    {
        // remember that this is a static set:
        // if the default IAP is changed, the change
        // is not reflected in the SyncML profile
        TUint32 iapId;
        // open the IAP communications database
        CCommsDatabase* commDB = CCommsDatabase::NewL();
        CleanupStack::PushL(commDB);
        // create a view in rank order
        CCommsDbConnectionPrefTableView* view;
        view = commDB->OpenConnectionPrefTableInRankOrderLC(ECommDbConnectionDirectionOutgoing);
        // Point to the first entry, the one set as default
        if (view->GotoFirstRecord() == KErrNone)
        {
            CCommsDbConnectionPrefTableView::TCommDbIapConnectionPref pref;
            view->ReadConnectionPreferenceL(pref);
            iapId=pref.iBearer.iIapId;
        
            iIapId.Zero();
            iIapId.AppendNum(iapId);
        
        }
        CleanupStack::PopAndDestroy(); // view
        CleanupStack::PopAndDestroy(); // commDB
    }
    else
    {
        TBool found = EFalse;
        
        // open commdb
        CCommsDatabase* commDb = CCommsDatabase::NewL(EDatabaseTypeIAP);
        CleanupStack::PushL(commDb);

        // open IAP table
        CCommsDbTableView* commView = 
            commDb->OpenIAPTableViewMatchingBearerSetLC(
                ECommDbBearerCSD|ECommDbBearerGPRS,
                ECommDbConnectionDirectionOutgoing);
        
        // open list
        iIAPs.Reset();
        
        // search all IAPs
        if (commView->GotoFirstRecord() == KErrNone)
            {
            do
                {
				TIAP iap;
                commView->ReadTextL(TPtrC(COMMDB_NAME), iap.iServiceName);
                commView->ReadUintL(TPtrC(COMMDB_ID), iap.iId);
                
                if ( iap.iServiceName == aIapName)
                {
                    found = ETrue;
                    TUint32 iapId = iap.iId;
                    iIapId.Zero();
                    iIapId.AppendNum(iapId);
                    break;
                }
                
                if (iIAPs.Append(iap) != KErrNone)
                    {
                    return;                 
                    }
                
                }
            while (commView->GotoNextRecord() == KErrNone);
            }
        CleanupStack::PopAndDestroy(commView);
        CleanupStack::PopAndDestroy(commDb);   
        
        if(!found)
            setIapIdForConnPropertiesL(_L("Default"));
    }
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
		
