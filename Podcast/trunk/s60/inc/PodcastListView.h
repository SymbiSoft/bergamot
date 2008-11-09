/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#ifndef PODCASTLISTVIEWH
#define PODCASTLISTVIEWH 

#include <aknview.h>
#include <aknlists.h> 

class CAknDoubleLargeStyleListBox;
class CEikFormattedCellListBox;
class CPodcastListContainer : public CCoeControl
    {
    public: 
		CPodcastListContainer();
		~CPodcastListContainer();
		void ConstructL( const TRect& aRect );
		void SizeChanged();
        TInt CountComponentControls() const;
        CCoeControl* ComponentControl( TInt aIndex ) const;
		void HandleResourceChange(TInt aType);
		virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
		CEikFormattedCellListBox* Listbox();
	protected:
		CEikFormattedCellListBox * iListbox;

	};


class CPodcastListView : public CAknView
    {
    public: 
        ~CPodcastListView();
	protected:
	    void ConstructL();
		CPodcastListView();	

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

		void HandleViewRectChange();
		
		/** 
		* Event handler for status pane size changes.
		* @c CAknView provides an empty implementation for sub classes that do 
		* not want to handle this event.
		*/
		void HandleStatusPaneSizeChange();

	protected:
		 CPodcastListContainer* iListContainer;
};
#endif // PODCASTBASEVIEWH

