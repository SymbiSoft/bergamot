/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastSettingsView.h"
#include "PodcastAppUi.h"
#include "Podcast.hrh"
#include <aknlists.h>
#include <aknsettingitemlist.h>
#include <aknnavide.h> 
#include <podcast.rsg>
#include "SettingsEngine.h"

#include <caknfileselectiondialog.h> 
#include <caknmemoryselectiondialog.h> 
#include <pathinfo.h>


class CIapSetting: public CAknEnumeratedTextPopupSettingItem 
{ 
public:
	CIapSetting(TInt aResourceId, TInt& aValue, CPodcastModel &aPodcastModel) :
		CAknEnumeratedTextPopupSettingItem(aResourceId, aValue), iPodcastModel(aPodcastModel)
		{
		}
	
	~CIapSetting()
		{
		}

	void CompleteConstructionL()
		{
		CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();
		
		CArrayPtr< CAknEnumeratedText > * enumeratedArr = EnumeratedTextArray();
		CArrayPtr< HBufC > * poppedUpTextArray = PoppedUpTextArray();
		enumeratedArr->Reset();
		poppedUpTextArray->Reset();
		CDesCArrayFlat *iapArray = iPodcastModel.IAPNames();

		DP2("InternalValue=%d, ExternalValue=%d", InternalValue(), ExternalValue());
		for (int i=0;i<iapArray->Count();i++) {
			HBufC *buf = (*iapArray)[i].AllocL();
			poppedUpTextArray->AppendL(buf);

			DP2("IAP name='%S', id=%d", buf, iPodcastModel.IAPIds()[i].iIapId);
			CAknEnumeratedText *enumerated = new CAknEnumeratedText(iPodcastModel.IAPIds()[i].iIapId, buf);
			enumeratedArr->AppendL(enumerated);
		}
		
		HandleTextArrayUpdateL();
			
		}

	void EditItemL(TBool aCalledFromMenu)
		{
		DP("CIapSetting::EditItemL BEGIN");
		LoadL();
		CAknEnumeratedTextPopupSettingItem::EditItemL(aCalledFromMenu);
		StoreL();		
		DP2("InternalValue=%d, ExternalValue=%d", InternalValue(), ExternalValue());
		DP("CIapSetting::EditItemL END");
		}

	void HandleSettingPageEventL(CAknSettingPage* aSettingPage, TAknSettingPageEvent aEventType)
		{
		DP("CIapSetting::HandleSettingPageEventL BEGIN");
		CAknSettingItem::HandleSettingPageEventL(aSettingPage, aEventType);
		/*if (aEventType == EEventSettingOked) 
			{
			DP2("InternalValue=%d, ExternalValue=%d", InternalValue(), ExternalValue());
			StoreL();
			}*/
		DP("CIapSetting::HandleSettingPageEventL END");
		}

protected:
  	CPodcastModel& iPodcastModel;
};

class CPodcastSettingItemList:public CAknSettingItemList
	{
public:
	CPodcastSettingItemList(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
		{
		}
	
	~CPodcastSettingItemList()
		{

		}

	void StoreSettings() {
		DP("StoreSettings BEGIN");
		StoreSettingsL();
		CSettingsEngine &se = iPodcastModel.SettingsEngine();
		se.SetBaseDir(iShowDir);
		DP1("Base Dir: %S", &iShowDir);
		se.SetUpdateAutomatically((TAutoUpdateSetting)iAutoUpdate);
		DP1("Update automatically: %d", iAutoUpdate);
		se.SetUpdateFeedInterval(iIntervalUpdate);
		DP1("Update feed interval: %d", iIntervalUpdate);
		se.SetUpdateFeedTime(iTimeUpdate);
		//DP1("Update feed time: %d", iTimeUpdate.Int64());
		
		if (iConnection == 1) {
			DP1("Specific IAP: %d", iIap);
			se.SetSpecificIAP(iIap);
			iPodcastModel.SetIap(iIap);
		} else {
			DP("Specific IAP: -1");
			se.SetSpecificIAP(-1);
			iPodcastModel.SetIap(-1);
		}
		
		DP1("Download automatically: %d", iAutoDownload);
		se.SetDownloadAutomatically(iAutoDownload);
		se.SaveSettingsL();
		DP("StoreSettings END");
	}
	
	void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
	{
		DP1("CPodcastSettingItemList::HandleListBoxEventL event %d", aEventType)
		CAknSettingItemList::HandleListBoxEventL(aListBox, aEventType);
	}
	
	void UpdateSettingVisibility()
		{
		LoadSettingsL();
		TBool dimAutoUpdateInterval = iConnection == -1 || iAutoUpdate != EAutoUpdatePeriodically;
		TBool dimAutoUpdateTime = iConnection == -1 || iAutoUpdate != EAutoUpdateAtTime;
		TBool dimAutoUpdate = iConnection == -1;
		TBool dimAutoDownload = iConnection == -1 || iAutoUpdate == EAutoUpdateOff;
		TBool dimIAP = iConnection < 0;
	
		iSettingAutoDownload->SetHidden(dimAutoDownload);
		iSettingAutoUpdate->SetHidden(dimAutoUpdate);
		iSettingAutoUpdateTime->SetHidden(dimAutoUpdateTime);
		iSettingAutoUpdateInterval->SetHidden(dimAutoUpdateInterval);
		iSettingIAP->SetHidden(dimIAP);
				
		TRAP_IGNORE(HandleChangeInItemArrayOrVisibilityL());
		}

	void  EditItemL (TInt aIndex, TBool aCalledFromMenu)
		{
		if (aIndex == 0) {
			CAknMemorySelectionDialog* memDlg = 
				CAknMemorySelectionDialog::NewL(ECFDDialogTypeNormal, ETrue);
			CleanupStack::PushL(memDlg);
			CAknMemorySelectionDialog::TMemory memory = 
				CAknMemorySelectionDialog::EPhoneMemory;
	
			if (memDlg->ExecuteL(memory))
				{
				TFileName importName;
			
				if (memory==CAknMemorySelectionDialog::EMemoryCard)
				{
					importName = PathInfo:: MemoryCardRootPath();
				}
				else
				{
					importName = PathInfo:: PhoneMemoryRootPath();
				}
	
				CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL(ECFDDialogTypeSave, R_PODCAST_SHOWDIR_SELECTOR);
				HBufC* select = iEikonEnv->AllocReadResourceLC(R_PODCAST_SOFTKEY_SELECT);
				dlg->SetLeftSoftkeyFileL(*select);
				CleanupStack::PopAndDestroy(select);
				CleanupStack::PushL(dlg);
	
				dlg->SetDefaultFolderL(importName);
				
				if(dlg->ExecuteL(importName))
					{
					importName.Append(_L("Podcasts"));
					iShowDir.Copy(importName);
					LoadSettingsL();
					}
				CleanupStack::PopAndDestroy(dlg);
				}
			CleanupStack::PopAndDestroy(memDlg);								
			}
		else {
			CAknSettingItemList::EditItemL(aIndex,aCalledFromMenu);
		}
			StoreSettingsL();
			UpdateSettingVisibility();
		}
	
	/**
	 * Framework method to create a setting item based upon the user id aSettingId. The 
	 * client code decides what type to contruct. new (ELeave) must then be used and the resulting 
	 * pointer returned. Ownership is thereafter base class's responsiblity.
	 *
	 * @param aSettingId	ID to use to determine the type of the setting item
	 * @return a constructed (not 2nd-stage constructed) setting item.
	 */
	CAknSettingItem* CreateSettingItemL( TInt aSettingId )
		{
		CSettingsEngine &se = iPodcastModel.SettingsEngine();
		iShowDir.Copy(se.BaseDir());
		iAutoUpdate = se.UpdateAutomatically();
		iIntervalUpdate = se.UpdateFeedInterval();
		iTimeUpdate = se.UpdateFeedTime();
		iConnection = se.SpecificIAP() <= 0 ? -1 : 1;
		iIap = se.SpecificIAP() <=0 ? iPodcastModel.IAPIds()[0].iIapId : se.SpecificIAP();
		iAutoDownload = se.DownloadAutomatically();
			
		switch(aSettingId)
			{
			case EPodcastSettingShowDir:
				return new (ELeave) CAknTextSettingItem(aSettingId, iShowDir);
				break;
			case EPodcastSettingAutoUpdate:
				iSettingAutoUpdate = new (ELeave) CAknEnumeratedTextPopupSettingItem(aSettingId, iAutoUpdate);
				return iSettingAutoUpdate;
				break;
			case EPodcastSettingUpdateInterval:
				iSettingAutoUpdateInterval = new (ELeave) CAknIntegerEdwinSettingItem(aSettingId, iIntervalUpdate);
				return iSettingAutoUpdateInterval; 
				break;
			case EPodcastSettingUpdateTime:
				iSettingAutoUpdateTime = new (ELeave) CAknTimeOrDateSettingItem(aSettingId, CAknTimeOrDateSettingItem::ETime, iTimeUpdate);
				return iSettingAutoUpdateTime;
				break;
			case EPodcastSettingConnection:
				return new (ELeave) CAknEnumeratedTextPopupSettingItem (aSettingId, iConnection);
				break;
			case EPodcastSettingIAPList:
				iSettingIAP = new (ELeave) CIapSetting (aSettingId, iIap, iPodcastModel);
				return iSettingIAP;
				break;
			case EPodcastSettingAutoDownload:
				iSettingAutoDownload = new (ELeave) CAknBinaryPopupSettingItem (aSettingId, iAutoDownload);
				return iSettingAutoDownload;
				break;
			default:
				return CAknSettingItemList::CreateSettingItemL(aSettingId);
				break;
			}
		return NULL;
		}
	
	TFileName iShowDir;
	
	TInt iIntervalUpdate;
	CAknSettingItem *iSettingAutoUpdateInterval;
	
	TTime iTimeUpdate;
	CAknSettingItem *iSettingAutoUpdateTime; 
	
	TInt iAutoUpdate;
	CAknSettingItem *iSettingAutoUpdate;
	
	TInt iAutoDownload;
	CAknSettingItem *iSettingAutoDownload; 

	TInt iConnection;
	
	TInt iIap;
	CAknSettingItem *iSettingIAP; 
		
	
	CPodcastModel &iPodcastModel;
	};


CPodcastSettingsView* CPodcastSettingsView::NewL(CPodcastModel& aPodcastModel)
    {
    CPodcastSettingsView* self = CPodcastSettingsView::NewLC(aPodcastModel);
    CleanupStack::Pop( self );
    return self;
    }

CPodcastSettingsView* CPodcastSettingsView::NewLC(CPodcastModel& aPodcastModel)
    {
    CPodcastSettingsView* self = new ( ELeave ) CPodcastSettingsView(aPodcastModel);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastSettingsView::CPodcastSettingsView(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

void CPodcastSettingsView::ConstructL()
{
	BaseConstructL(R_PODCAST_SETTINGSVIEW);	
	
	iListbox =new (ELeave) CPodcastSettingItemList(iPodcastModel);
	iListbox->SetMopParent( this );
	iListbox->ConstructFromResourceL(R_PODCAST_SETTINGS);
	iListbox->SetRect(ClientRect());
	iListbox->ActivateL();   
	iListbox->MakeVisible(EFalse);
	
	iNaviPane =( CAknNavigationControlContainer * ) StatusPane()->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) );
		
	HBufC *titleBuffer = iEikonEnv->AllocReadResourceL(R_SETTINGS_TITLE);
	iNaviDecorator  = iNaviPane->CreateNavigationLabelL(*titleBuffer);
	delete titleBuffer;
}
    
CPodcastSettingsView::~CPodcastSettingsView()
    {
	delete iListbox;
    }

TUid CPodcastSettingsView::Id() const
{
	return KUidPodcastSettingsViewID;
}
		
void CPodcastSettingsView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid /*aCustomMessageId*/,
	                                  const TDesC8& /*aCustomMessage*/)
{
	iPreviousView = aPrevViewId;
	
	if (iListbox) {
		iListbox->SetRect(ClientRect());
		AppUi()->AddToStackL(*this, iListbox);
		  
		iListbox->UpdateSettingVisibility();
		iListbox->MakeVisible(ETrue);
		iListbox->DrawNow();
		iListbox->SetFocus(ETrue);
	}
	
	if(iNaviDecorator && iNaviPane)
		{
		iNaviPane->PushL(*iNaviDecorator);
		}
}

void CPodcastSettingsView::DoDeactivate()
	{
	
	if(iListbox)
	    {
		AppUi()->RemoveFromViewStack( *this, iListbox );
		iListbox->StoreSettings();
		iListbox->MakeVisible(EFalse);
		}
	
	if(iNaviDecorator && iNaviPane)
		{
		iNaviPane->Pop(iNaviDecorator);
		}
		
	}

/** 
* Command handling function intended for overriding by sub classes. 
* Default implementation is empty.  
* @param aCommand ID of the command to respond to. 
*/
void CPodcastSettingsView::HandleCommandL(TInt aCommand)
{
	CAknView::HandleCommandL(aCommand);
	RDebug::Printf("CPodcastListView::HandleCommandL=%d", aCommand);
	switch(aCommand)
	{
	case EAknSoftkeyExit:
	case EEikCmdExit:
		{
            AppUi()->Exit();
            break;
		}
	case EAknSoftkeyBack:
		{
		AppUi()->ActivateViewL(iPreviousView);
		}break;	
	case EPodcastZoomSetting:
		break;
	case EPodcastAbout:
		break;
	default:
		break;
	}
}
