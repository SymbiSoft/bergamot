#ifndef PODCASTCLIENTSETTINGSDLGH
#define PODCASTCLIENTSETTINGSDLGH
#include <eikdialg.h>

#include "FeedInfo.h"
#include "ShowInfo.h"

class CPodcastModel;
class CEikEdwin;
class CEikCheckBox;
class CEikChoiceList;
class CQikNumberEditor;
class CQikSlider;
class CQikTimeEditor;
class CQikListBox;

class CPodcastClientSetAudioBookOrderDlg: public CEikDialog
{
public:
		CPodcastClientSetAudioBookOrderDlg(CPodcastModel& aPodcastModel, TUint aFeedId);
		~CPodcastClientSetAudioBookOrderDlg();
protected:
		TBool OkToExitL(TInt aCommandId);
		void PreLayoutDynInitL();
		void PopulateListboxL();
		static TInt CompareShowsByTrack(const CShowInfo &a, const CShowInfo &b);

private:
		CPodcastModel& iPodcastModel;
		TInt iFeedId;
		TBool iSwapSet;
		TInt iSwapIndex;
		CEikLabel* iSetLabel;
		CQikListBox* iListbox;
		RShowInfoArray iShowInfoArray;
};

/**
 * Dialog (old style) used to do volumd
 */
class CPodcastClientVolumeDlg: public CEikDialog
{
public:
	CPodcastClientVolumeDlg(CPodcastModel& aPodcastModel);
	~CPodcastClientVolumeDlg();
protected:
	TBool OkToExitL(TInt aCommandId);
	void PreLayoutDynInitL();
	
	void HandleControlStateChangeL(TInt aControlId);
private:
	/** Reference to the podcast application model */
	CPodcastModel& iPodcastModel;
	CQikSlider* iVolumeSlider;
};


/**
 * Dialog (old style) used to do volumd
 */
class CPodcastClientIAPDlg: public CEikDialog
{
public:
	CPodcastClientIAPDlg(CPodcastModel& aPodcastModel, TInt& aSelectedIAP);
	~CPodcastClientIAPDlg();
protected:
	TBool OkToExitL(TInt aCommandId);
	void PreLayoutDynInitL();
private:
	/** Reference to the podcast application model */
	CPodcastModel& iPodcastModel;
	CEikChoiceList* iIAPListCtrl;
	TInt& iSelectedIAP;
};



/**
 * Dialog (old style) used to do settings
 */
class CPodcastClientSettingsDlg: public CEikDialog
{
public:
	CPodcastClientSettingsDlg(CPodcastModel& aPodcastModel);
	~CPodcastClientSettingsDlg();
protected:
	TBool OkToExitL(TInt aCommandId);
	 void PreLayoutDynInitL();
	
	 void HandleControlStateChangeL(TInt aControlId);
	 void LineChangedL(TInt aControlId);

	 void RefreshUpdateOptions();
private:
	/** Reference to the podcast application model */
	CPodcastModel& iPodcastModel;

	CEikEdwin* iShowBaseDirCtrl;
	CEikChoiceList* iAutoUpdateCtrl;
	CEikCheckBox* iAutoDownloadCtrl;
	CQikNumberEditor* iUpdateIntervalCtrl;
//	CQikNumberEditor* iMaxSimDlsCtrl;
	CQikTimeEditor* iUpdateAtCtrl;
	CEikChoiceList* iConnectionCtrl;
	CEikChoiceList* iIAPListCtrl;

	TFileName iSelectedPathTemp;
};

#endif
