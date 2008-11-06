/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#ifndef PODCASTFEEDVIEWH
#define PODCASTFEEDVIEWH 

#include <aknview.h>

class CPodcastFeedContainer;

class CPodcastFeedView : public CAknView
    {
    public: 
        static CPodcastFeedView* NewL( const TRect& aRect );
        static CPodcastFeedView* NewLC( const TRect& aRect );
        ~CPodcastFeedView();
	protected:
	    void ConstructL( const TRect& aRect );
		CPodcastFeedView();

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
	private:
		CPodcastFeedContainer* iFeedContainer;
};

#endif // PODCASTFEEDVIEWH