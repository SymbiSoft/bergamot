/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#ifndef PODCASTPLAYVIEWH
#define PODCASTPLAYVIEWH 

#include <aknview.h>
#include <aknnavide.h> 
#include <eikprogi.h>
#include <eikimage.h> 
#include <eikedwin.h>
#include <eiklabel.h>
#include <imageconversion.h>
#include <bitmaptransforms.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include "ShowEngineObserver.h"
#include "PodcastModel.h"
#include "SoundEngine.h"

const TInt KActiveShowUIDCmd = 0x1000;

class CPodcastPlayContainer;
class CAknNavigationControlContainer;

class CImageWaiter:public CActive
	{
	public:
	CImageWaiter(CEikImage* aImageCtrl, CFbsBitmap* aBitmap);
	~CImageWaiter();
	void Start();
	void RunL();
	void DoCancel();

	private:
	CEikImage* iImageCtrl;
	CFbsBitmap* iBitmap;
	TBool iScaling;
	CBitmapScaler* iBitmapScaler;
	};

class CMyEdwin : public CEikEdwin
	{
public:
	void SetBackgroundColor(const TRgb& aColor);
	void SetTextColor(const TRgb& aColor);
	};

class CVolumeTimer : public CTimer
	{
	public:
		CVolumeTimer(CPodcastPlayContainer* aContainer);
		~CVolumeTimer();
		void CountDown();
		void RunL();
		
	private:
		CPodcastPlayContainer* iContainer;
	};

enum TNaviType {
	ENaviNone,
	ENaviTabs,
	ENaviVolume
};

class CPodcastPlayContainer : public CCoeControl, public MSoundEngineObserver, public MShowEngineObserver
    {
    public: 
		CPodcastPlayContainer(CPodcastModel& aPodcastModel, CAknNavigationControlContainer* aNaviPane);
		~CPodcastPlayContainer();
		void ConstructL( const TRect& aRect );

		void ViewActivatedL(TInt aCurrentShowUid);
    	void Draw( const TRect& aRect ) const;
 	    void UpdateViewL();
		CShowInfo* ShowInfo()
		{
			return iShowInfo;
		}
		void PlayShow();
		void NaviShowTabGroupL();
		void NaviShowVolumeL(TUint aVolume);
		void NaviClear();
				
	protected:
		void UpdateControlVisibility();
		void ShowListUpdated(){};  
		void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);
		void DownloadQueueUpdated(TInt /*aDownloadingShows*/, TInt /*aQueuedShows*/) {}

		static TInt PlayingUpdateStaticCallbackL(TAny* aPlayView);
		void UpdatePlayStatusL();

		void PlaybackInitializedL();
		void PlaybackStartedL();
		void PlaybackStoppedL();
		void VolumeChanged(TUint aVolume, TUint aMaxVolume);
		void UpdateMaxProgressValueL(TInt aDuration);
		
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
		void HandleResourceChange(TInt aType);
		void SizeChanged();
		TInt CountComponentControls() const;
		CCoeControl* ComponentControl(TInt aIndex) const;

		
        /**
        * From CCoeControl, MopSupplyObject.
        */
        TTypeUid::Ptr MopSupplyObject( TTypeUid aId );
	protected:
		CAknNavigationDecorator* iTabNaviDecorator;
		CAknNavigationDecorator* iVolumeNaviDecorator;
				
		CPeriodic* iPlaybackTicker;
		CEikImage* iCoverImageCtrl;
		CFbsBitmap* iCurrentCoverImage;
		CEikProgressInfo* iPlayProgressbar;
		CEikProgressInfo* iDownloadProgressInfo;
		CEikLabel* iShowInfoTitle;
		CMyEdwin* iShowInfoLabel;
		CEikLabel* iTimeLabel;
	
		CShowInfo* iShowInfo;
		CPodcastModel& iPodcastModel;
		TUint iMaxProgressValue;
		TUint iBytesDownloaded;
		CAknNavigationControlContainer* iNaviPane;
		CAknTabGroup* iTabGroup;
		TFileName iLastImageFileName;
        CAknsBasicBackgroundControlContext* iBgContext;
        CImageDecoder* iBitmapConverter;        
        CImageWaiter* iImageWaiter;
        CVolumeTimer* iVolumeTimer;
        TNaviType iNaviType;
	};

class CPodcastPlayView : public CAknView
    {
    public: 
        static CPodcastPlayView* NewL(CPodcastModel& aPodcastModel);
        static CPodcastPlayView* NewLC(CPodcastModel& aPodcastModel);
        ~CPodcastPlayView();
	protected:
	    void ConstructL();
		CPodcastPlayView(CPodcastModel& aPodcastModel);

		/**
		 * Returns views id, intended for overriding by sub classes.
		 * @return id for this view.
		 */
		TUid Id() const;

		/** 
		 * Handles a view activation and passes the message of type 
		 * @c aCustomMessageId. This function is intended for overriding by 
		 * sub classes. This function is called by @c AknViewActivatedL().
		 * @param aPrevViewId Specifies the view previously active.
		 * @param aCustomMessageId Specifies the message type.
		 * @param aCustomMessage The activation message.
		 */
		void DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage);

	    /** 
		 * View deactivation function intended for overriding by sub classes. 
		 * This function is called by @c AknViewDeactivated().
		 */
		void DoDeactivate();
				
		/** 
		* Command handling function intended for overriding by sub classes. 
		* Default implementation is empty.  
		* @param aCommand ID of the command to respond to. 
		*/
		void HandleCommandL(TInt aCommand);
		
		void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);
	private:
        CAknNavigationControlContainer* iNaviPane;
		CPodcastPlayContainer* iPlayContainer;
		CPodcastModel& iPodcastModel;
		/** Previous activated view */
		TVwsViewId iPreviousView;
		TInt iCurrentViewShowUid;
};

#endif // PODCASTPLAYVIEWH


