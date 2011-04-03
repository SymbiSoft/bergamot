#ifndef __SYNCCLIENT_APPVIEW_H__
#define __SYNCCLIENT_APPVIEW_H__

// INCLUDE FILES
#include "SyncClientS60.hrh"
#include <coecntrl.h>
#include <aknsettingitemlist.h> 
#include "RSyncServerSession.h"
#include <eiklabel.h> 
#include <aknlists.h> 

// CLASS DECLARATION

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
 
     void SizeChanged();

private: // Functions from base classes

	/**
    * From CCoeControl,Draw.
    */
    void Draw(const TRect& aRect) const;

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
    void CreateEmptyListbox();
private: // Data members

    CAknSingleLargeStyleListBox * iEmptyListbox;
	CAknSettingItemList* iItemList;
	RSyncServerSession serverSession;
	RArray<TSmlProfileId> lastViewProfiles;
	int iProfiles;
    };


#endif
