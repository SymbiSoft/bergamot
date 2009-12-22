#include <eikedwin.h>
#include <e32hashtab.h>
#include <eikchlst.h>
#include <QikNumberEditor.h>
#include <eikchkbx.h>
#include <QikSelectFolderDlg.h>
#include <QikCommandManager.h>
#include <QikSlider.h>
#include <QikTimeEditor.h>
#include <eiklabel.h>
#include <PodCastClient.rsg>
#include <MQikListBoxModel.h>
#include <MQikListBoxData.h>

#include "ShowEngine.h"
#include "PodcastClient.hrh"
#include "PodcastClientSettingsDlg.h"
#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SettingsEngine.h"


CPodcastClientSetAudioBookOrderDlg::CPodcastClientSetAudioBookOrderDlg(CPodcastModel& aPodcastModel, TUint aFeedId):iPodcastModel(aPodcastModel), iFeedId(aFeedId)
{
}

CPodcastClientSetAudioBookOrderDlg::~CPodcastClientSetAudioBookOrderDlg()
{
	iShowInfoArray.Close();
}

TBool CPodcastClientSetAudioBookOrderDlg::OkToExitL(TInt aCommandId)
{
	if(aCommandId == EPodcastSetAudioBookPlayOrderSwap) 
	{
		TBuf<64> buf;
		MQikListBoxModel& model(iListbox->Model());
		
		if(iSwapSet)
		{
			iEikonEnv->ReadResourceL(buf, R_PODCAST_SORT_AUDIOBOOK_SELECT);
			iSetLabel->SetTextL(buf);
			iSwapSet = EFalse;

			model.ModelBeginUpdateLC();													
			MQikListBoxData* data = model.RetrieveDataL(iSwapIndex);	
			CleanupClosePushL(*data);
			data->SetEmphasis(EFalse);								
			CleanupStack::PopAndDestroy();//close data
			model.DataUpdatedL(iSwapIndex);					
			model.ModelEndUpdateL();
			TInt newIndex = iListbox->CurrentItemIndex();
			TInt trackNo1 = iShowInfoArray[iSwapIndex]->TrackNo();
			TInt trackNo2 = iShowInfoArray[newIndex]->TrackNo();
			iShowInfoArray[iSwapIndex]->SetTrackNo(trackNo2);
			iShowInfoArray[newIndex]->SetTrackNo(trackNo1);
			PopulateListboxL();
		}
		else
		{			
			iEikonEnv->ReadResourceL(buf, R_PODCAST_SORT_AUDIOBOOK_SWAP);
			iSetLabel->SetTextL(buf);
			iSwapSet = ETrue;
			iSwapIndex = iListbox->CurrentItemIndex();

			model.ModelBeginUpdateLC();													
			MQikListBoxData* data = model.RetrieveDataL(iSwapIndex);	
			CleanupClosePushL(*data);
			data->SetEmphasis(ETrue);								
			CleanupStack::PopAndDestroy();//close data
			model.DataUpdatedL(iSwapIndex);					
			model.ModelEndUpdateL();
		}
		iSetLabel->DrawDeferred();

		return EFalse;
	}
	
	return ETrue;
}

TInt CPodcastClientSetAudioBookOrderDlg::CompareShowsByTrack(const CShowInfo &a, const CShowInfo &b)
	{
	return a.TrackNo()-b.TrackNo();
	}


void CPodcastClientSetAudioBookOrderDlg::PreLayoutDynInitL()
{
	iSetLabel = static_cast<CEikLabel*>(ControlOrNull(EPodcastSetAudioBookPlayOrderLabel));

	iListbox = static_cast<CQikListBox*>(ControlOrNull(EPodcastSetAudioBookPlayOrderListbox));
	iPodcastModel.ShowEngine().GetShowsByFeed(iShowInfoArray, iFeedId); 

	PopulateListboxL();
}

_LIT(KNumberFormat,"%03d:");
void CPodcastClientSetAudioBookOrderDlg::PopulateListboxL()
{
	TInt cnt = iShowInfoArray.Count();

	TLinearOrder<CShowInfo> sortOrder(CompareShowsByTrack);
	iShowInfoArray.Sort(sortOrder);
	
	TInt lastIndex = iListbox->CurrentItemIndex();
	TInt lastTopIndex = iListbox->TopItemIndex();
	iListbox->RemoveAllItemsL();

	MQikListBoxModel& model(iListbox->Model());
	model.ModelBeginUpdateLC();
	MQikListBoxData* listBoxData = NULL;
	TBuf<32> numberFormatBuf;
	
	for(TInt loop = 0;loop<cnt;loop++)
	{
		listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
		CleanupClosePushL(*listBoxData);
		numberFormatBuf.Format(KNumberFormat(), iShowInfoArray[loop]->TrackNo());
		listBoxData->AddTextL(numberFormatBuf, EQikListBoxSlotText1);
		listBoxData->AddTextL(iShowInfoArray[loop]->Title(), EQikListBoxSlotText3);
		TParsePtrC parser(iShowInfoArray[loop]->FileName());
		listBoxData->AddTextL(parser.NameAndExt(), EQikListBoxSlotText2);
		CleanupStack::PopAndDestroy(listBoxData); // close listbox data
	}

	model.ModelEndUpdateL();

	if(lastIndex >= 0 && lastIndex<cnt)
		{
		iListbox->SetCurrentAndTopItemIndexL(lastIndex, lastTopIndex, EDrawNow);
		}
}


CPodcastClientIAPDlg::CPodcastClientIAPDlg(CPodcastModel& aPodcastModel, TInt& aSelectedIAP):iPodcastModel(aPodcastModel), iSelectedIAP(aSelectedIAP)
{
}

CPodcastClientIAPDlg::~CPodcastClientIAPDlg()
{
}

void CPodcastClientIAPDlg::PreLayoutDynInitL()
{
	iPodcastModel.UpdateIAPListL();
	iIAPListCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingIAPList));
	iIAPListCtrl->SetArrayL((MDesCArray*) NULL);
	iIAPListCtrl->SetArrayExternalOwnership(ETrue);
	iIAPListCtrl->SetArrayL(iPodcastModel.IAPNames());
	iIAPListCtrl->SetFocus(ETrue);
	
	TInt cnt = iPodcastModel.IAPIds().Count();
	for(TInt loop = 0;loop<cnt;loop++)
	{
		if(iPodcastModel.IAPIds()[loop].iIapId == (TUint32) iSelectedIAP)
		{
			iIAPListCtrl->SetCurrentItem(loop);
			break;
		}
		
	}
}


TBool CPodcastClientIAPDlg::OkToExitL(TInt aCommandId)
{
	if(aCommandId != EEikBidCancel)
	{
		iSelectedIAP = iPodcastModel.IAPIds()[iIAPListCtrl->CurrentItem()].iIapId;
	}

	return ETrue;
}

CPodcastClientVolumeDlg::CPodcastClientVolumeDlg(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

CPodcastClientVolumeDlg::~CPodcastClientVolumeDlg()
{
}

void CPodcastClientVolumeDlg::PreLayoutDynInitL()
{
	iVolumeSlider =	 static_cast<CQikSlider*> (ControlOrNull(EPodcastSettingsVolume));
	iVolumeSlider->SetValue(iPodcastModel.SettingsEngine().Volume()/KVolumeDelta);
}

TBool CPodcastClientVolumeDlg::OkToExitL(TInt /*aCommandId*/)
{
	iPodcastModel.SettingsEngine().SetVolume(iVolumeSlider->CurrentValue()*KVolumeDelta);

	return ETrue;
}

void CPodcastClientVolumeDlg::HandleControlStateChangeL(TInt aControlId)
{
	CEikDialog::HandleControlStateChangeL(aControlId);

	if(EPodcastSettingsVolume == aControlId)
	{
		iPodcastModel.SettingsEngine().SetVolume(iVolumeSlider->CurrentValue()*KVolumeDelta);
	}
}

enum
{
	EUseDefaultAccount,
	EUseAskMeAccount,
	EUseSpecifiedIAP
};

enum
{
	ENoAutoDownload,
	EPeriodically,
	EAtClock
};

CPodcastClientSettingsDlg::CPodcastClientSettingsDlg(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

CPodcastClientSettingsDlg::~CPodcastClientSettingsDlg()
{
}

void CPodcastClientSettingsDlg::PreLayoutDynInitL()
{
	// First init control pointers
	iShowBaseDirCtrl = static_cast<CEikEdwin*> (ControlOrNull(EPodcastSettingShowDir));
	iAutoUpdateCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingAutoUpdate));
	iAutoDownloadCtrl = static_cast<CEikCheckBox*> (ControlOrNull(EPodcastSettingAutoDownload));
	iUpdateIntervalCtrl = static_cast<CQikNumberEditor*> (ControlOrNull(EPodcastSettingUpdateInterval));
//	iMaxSimDlsCtrl = static_cast<CQikNumberEditor*> (ControlOrNull(EPodcastSettingMaxsimdls));
	iConnectionCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingConnection));
	iUpdateAtCtrl = static_cast<CQikTimeEditor*> (ControlOrNull(EPodcastSettingUpdateTime));
	iIAPListCtrl = static_cast<CEikChoiceList*> (ControlOrNull(EPodcastSettingIAPList));
	iIAPListCtrl->SetArrayL((MDesCArray*) NULL);
	iIAPListCtrl->SetArrayExternalOwnership(ETrue);
	iIAPListCtrl->SetArrayL(iPodcastModel.IAPNames());

	// Populate data
	iSelectedPathTemp = iPodcastModel.SettingsEngine().BaseDir();
	iShowBaseDirCtrl->SetTextL(&iSelectedPathTemp);

	iAutoUpdateCtrl->SetCurrentItem(iPodcastModel.SettingsEngine().UpdateAutomatically());
	iAutoDownloadCtrl->SetState(iPodcastModel.SettingsEngine().DownloadAutomatically()?CEikButtonBase::ESet:CEikButtonBase::EClear);

	iUpdateIntervalCtrl->SetValueL(iPodcastModel.SettingsEngine().UpdateFeedInterval());
	iUpdateAtCtrl->SetTimeL(iPodcastModel.SettingsEngine().UpdateFeedTime());
	RefreshUpdateOptions();
	
//	iMaxSimDlsCtrl->SetValueL(iPodcastModel.SettingsEngine().MaxSimultaneousDownloads());

	if(iPodcastModel.SettingsEngine().SpecificIAP() == -1)
	{
		iConnectionCtrl->SetCurrentItem(EUseAskMeAccount);
		SetLineDimmedNow(EPodcastSettingIAPList, ETrue);
		SetLineDimmedNow(EPodcastSettingAutoUpdate, ETrue);
		SetLineDimmedNow(EPodcastSettingAutoDownload, ETrue);

		//MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
	}
	else if(iPodcastModel.SettingsEngine().SpecificIAP() > 0)
	{
		iConnectionCtrl->SetCurrentItem(EUseSpecifiedIAP);
		//MakeWholeLineVisible(EPodcastSettingIAPList, ETrue);
		SetLineDimmedNow(EPodcastSettingIAPList, EFalse);
		TInt cnt = iPodcastModel.IAPIds().Count();
		for(TInt loop = 0;loop<cnt;loop++)
		{
			if(iPodcastModel.IAPIds()[loop].iIapId == (TUint32) iPodcastModel.SettingsEngine().SpecificIAP())
			{
				iIAPListCtrl->SetCurrentItem(loop);
				break;
			}

		}
	}
	else
	{
		iConnectionCtrl->SetCurrentItem(EUseDefaultAccount);
		//MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
		SetLineDimmedNow(EPodcastSettingIAPList, ETrue);
	}
}


TBool CPodcastClientSettingsDlg::OkToExitL(TInt aCommandId)
{
	switch(aCommandId)
	{
	case EEikBidOk:
		{
			iShowBaseDirCtrl->GetText(iSelectedPathTemp);
			iPodcastModel.SettingsEngine().SetBaseDir(iSelectedPathTemp);
			iPodcastModel.SettingsEngine().SetUpdateAutomatically((TAutoUpdateSetting) iAutoUpdateCtrl->CurrentItem());
			iPodcastModel.SettingsEngine().SetDownloadAutomatically(iAutoDownloadCtrl->State() == CEikButtonBase::ESet);
			
			if (iPodcastModel.SettingsEngine().UpdateAutomatically() == EAutoUpdatePeriodically) {
				iPodcastModel.SettingsEngine().SetUpdateFeedInterval(iUpdateIntervalCtrl->Value());
			} else {
		    	iPodcastModel.SettingsEngine().SetUpdateFeedTime(iUpdateAtCtrl->Time());
			}


//			iPodcastModel.SettingsEngine().SetMaxSimultaneousDownloads(iMaxSimDlsCtrl->Value());
			/*if(iVolumeSlider != NULL)
			{
				iPodcastModel.SettingsEngine().SetVolume(iVolumeSlider->CurrentValue());
			}*/

			switch(iConnectionCtrl->CurrentItem())
			{
			case EUseDefaultAccount:
				{
					iPodcastModel.SettingsEngine().SetSpecificIAP(0);
					iPodcastModel.SetIap(0);
				}break;
			case EUseAskMeAccount:
				{
					iPodcastModel.SettingsEngine().SetSpecificIAP(-1);
					iPodcastModel.SetIap(-1);
				}break;
			case EUseSpecifiedIAP:
				{
					iPodcastModel.SettingsEngine().SetSpecificIAP(iPodcastModel.IAPIds()[iIAPListCtrl->CurrentItem()].iIapId);
					iPodcastModel.SetIap(iPodcastModel.IAPIds()[iIAPListCtrl->CurrentItem()].iIapId);
				}
				break;
			}
			iPodcastModel.SettingsEngine().SaveSettingsL();
			
		}break;
	case EPodcastSelectShowDir:
		{
			TFileName newTemp = iSelectedPathTemp;
			if(CQikSelectFolderDlg::RunDlgLD(newTemp, &iSelectedPathTemp))
			{
				iSelectedPathTemp = newTemp;
				iShowBaseDirCtrl->SetTextL(&iSelectedPathTemp);
				iShowBaseDirCtrl->HandleTextChangedL();
			}
			return EFalse;
		}
	}
	return ETrue;
}

void CPodcastClientSettingsDlg::RefreshUpdateOptions()
{
	switch(iAutoUpdateCtrl->CurrentItem())
	{
	case ENoAutoDownload:
		{
			SetLineDimmedNow(EPodcastSettingUpdateInterval, ETrue);
			SetLineDimmedNow(EPodcastSettingUpdateTime, ETrue);
			//MakeWholeLineVisible(EPodcastSettingIAPList, ETrue);
		}
		break;
	case EPeriodically:
		{
			SetLineDimmedNow(EPodcastSettingUpdateInterval, EFalse);
			SetLineDimmedNow(EPodcastSettingUpdateTime, ETrue);
		}break;
	default:
		{
			SetLineDimmedNow(EPodcastSettingUpdateInterval, ETrue);
			SetLineDimmedNow(EPodcastSettingUpdateTime, EFalse);
			//MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
		}break;
	}
}


void CPodcastClientSettingsDlg::HandleControlStateChangeL(TInt aControlId)
{
	CEikDialog::HandleControlStateChangeL(aControlId);

	if(EPodcastSettingConnection == aControlId)
	{
		switch(iConnectionCtrl->CurrentItem())
		{
		case EUseSpecifiedIAP:
			{
			SetLineDimmedNow(EPodcastSettingIAPList, EFalse);
			SetLineDimmedNow(EPodcastSettingAutoDownload, EFalse);
			SetLineDimmedNow(EPodcastSettingAutoUpdate, EFalse);
			SetLineDimmedNow(EPodcastSettingUpdateInterval, EFalse);
			//MakeWholeLineVisible(EPodcastSettingIAPList, ETrue);
			}
			break;
		case EUseAskMeAccount: 
			{
			// we don't allow auto update or download when user wants IAP selection dialog
			SetLineDimmedNow(EPodcastSettingAutoUpdate, ETrue);
			SetLineDimmedNow(EPodcastSettingAutoDownload, ETrue);
			SetLineDimmedNow(EPodcastSettingUpdateInterval, ETrue);
			iAutoUpdateCtrl->SetCurrentItem(0);
			iAutoDownloadCtrl->SetState(CEikButtonBase::EClear);
			} break;
		default:
			{
			SetLineDimmedNow(EPodcastSettingIAPList, ETrue);
			SetLineDimmedNow(EPodcastSettingAutoDownload, EFalse);
			SetLineDimmedNow(EPodcastSettingAutoUpdate, EFalse);
			SetLineDimmedNow(EPodcastSettingUpdateInterval, EFalse);
			//MakeWholeLineVisible(EPodcastSettingIAPList, EFalse);
			}break;
		}
	} else if (EPodcastSettingAutoUpdate == aControlId) {
		RefreshUpdateOptions();
	}
}

void CPodcastClientSettingsDlg::LineChangedL(TInt aControlId)
{
	CEikDialog::LineChangedL(aControlId);

	CQikCommandManager& comMan = CQikCommandManager::Static();

	if(aControlId == EPodcastSettingShowDir)
	{
		comMan.SetAvailable(*this, EPodcastSelectShowDir, ETrue);
	}
	else
	{
		comMan.SetAvailable(*this, EPodcastSelectShowDir, EFalse);
	}

}

