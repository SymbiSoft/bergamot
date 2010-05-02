#ifndef __SYNCCLIENT_APPVIEW_H__
#define __SYNCCLIENT_APPVIEW_H__

// INCLUDE FILES
#include "SyncClientS60.hrh"
#include <coecntrl.h>
#include <aknsettingitemlist.h> 
#include "RSyncServerSession.h"
#include <eiklabel.h> 
#include "SyncSetting.h"
#include <SyncMLClient.h>
#include <SyncMLClientDS.h>

#include <CommDbConnPref.h>
#include <es_sock.h>
#include <cdbcols.h>
#include <commdb.h>
#include <cdbtemp.h>

// CLASS DECLARATION
#define CONTACTS_DATA_STORE_NAME _L("C:Contacts.cdb")
#define CALENDAR_DATA_STORE_NAME _L("C:Calendar")
#define NOTES_DATA_STORE_NAME _L("c:Notepad.dat")

#define _UID3 0x2002C265
// CONSTANTS
const TUint32 KUndefinedIAPid = 0x00;

struct TIAP 
    {
    TBuf<KCommsDbSvrRealMaxFieldLength> iServiceName;
    TUint32 iId;
    };


enum TSyncApp
	{
	ESyncCalendar,
	ESyncContacts,
	ESyncNotes
	};

/**
* A view class for a dynamic setting list.
*/
class CSyncClientAppView : public CCoeControl
    {
public: // Constructor and destructor
	
    /**
    * Symbian OS default constructor.
    */      
	void ConstructL(const TRect& aRect);
	
	/**
	* Constructor
	*/
	CSyncClientAppView();
	
	/**
	* Destructor
	*/
     ~CSyncClientAppView();
          
     void DeleteSync();
     
     void CreateNewProfile(TInt aCommand);
     
     void ViewDetails();
     
private: // Functions from base classes

	/**
    * From CCoeControl,Draw.
    */
    void Draw(const TRect& aRect) const;

    void SetDataProvider(RSyncMLDataSyncProfile& profile, TSmlDataProviderId& iProvider, const TDesC& serverURI,TSyncApp aSyncApp);
    void setIapIdForConnPropertiesL(const TDesC& aIapName);
    void SetServer(RSyncMLDataSyncProfile& profile, const TDesC8& aServerURI, const TDesC& aIapName);
    /**
    * From CoeControl,CountComponentControls.
    */        
    TInt CountComponentControls() const;

	/**
    * From CCoeControl,ComponentControl.
    */
    CCoeControl* ComponentControl(TInt aIndex) const;

    void CreateChoiceListItem(int id, const TPtrC16 &caption, int state ); 

	/**
    * From CCoeControl,OfferKeyEventL.
    */
	TKeyResponse OfferKeyEventL(	const TKeyEvent& aKeyEvent, TEventCode aType );
	

private: // New Functions

	/**
	* Loads the setting list dynamically.
	*/	
	void LoadListL();

	/**
	* Stores the settings of the setting list.
	*/	
    void StoreSettingsL();
    
    void ShowSyncProfiles();

private: // Data members
    /*
     * descriptor storing the IAP id, used by CreateProfileL.
     */
    TBuf8<32>   iIapId;
    RArray<TIAP> iIAPs;

	CAknSettingItemList* iItemList;
	RSyncServerSession serverSession;
	RArray<TSmlProfileId> lastViewProfiles;
    };


#endif
