/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */


#ifndef PODCASTSHOWSVIEWH
#define PODCASTSHOWSVIEWH 

#include <aknview.h>

class CPodcastShowsContainer;

class CPodcastShowsView : public CAknView
    {
    public: 
        static CPodcastShowsView* NewL( const TRect& aRect );
        static CPodcastShowsView* NewLC( const TRect& aRect );
        ~CPodcastShowsView();
	protected:
	    void ConstructL( const TRect& aRect );
		CPodcastShowsView();

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
		CPodcastShowsContainer* iShowsContainer;
};

#endif // PODCASTSHOWSVIEWH


