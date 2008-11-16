/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#ifndef PODCASTPLAYVIEWH
#define PODCASTPLAYVIEWH 

#include <aknview.h>
#include "ShowEngineObserver.h"
#include "PodcastModel.h"

const TInt KActiveShowUIDCmd = 0x1000;

class CPodcastPlayContainer;
class CAknNavigationControlContainer;

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


