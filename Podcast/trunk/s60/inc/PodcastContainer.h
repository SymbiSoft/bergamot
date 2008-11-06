/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */


#ifndef PODCASTCONTAINER_H
#define PODCASTCONTAINER_H

#include <coecntrl.h>
#include <eiklbo.h> // MEikListBoxObserver
#include <akngrid.h> // CAknGrid
#include <aknnavide.h> 

_LIT(KIconName, "\\resource\\apps\\podcast.mif");
const TInt KLengthOfItemResourceString = 40;

class CPodcastContainer : public CCoeControl, MEikListBoxObserver
    {
    public: 
        static CPodcastContainer* NewL( const TRect& aRect );
        static CPodcastContainer* NewLC( const TRect& aRect );
        ~CPodcastContainer();

    public:
        void SetGridInitialParametersL();
        void CreateGridResourceL();
        void LoadGraphicsL();
        void AddDataL();
        void ApplySelGridGraphicStyle();
		void SetIconSizes( CArrayPtr<CGulIcon>* aIconArray );

    private: // Functions from base classes
        void HandleListBoxEventL( CEikListBox* aListBox, 
                TListBoxEvent aListBoxEvent );
        void ConstructL( const TRect& aRect );
        void SizeChanged();
        void Draw( const TRect& aRect ) const;
        TInt CountComponentControls() const;
        CCoeControl* ComponentControl( TInt aIndex ) const;
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,
                TEventCode aType );
    private:  // New functions
        void SetNaviPaneTextL();

    private: 
        CAknGrid* iGrid;
        CAknGridM* iGridM;
        TBool iVerticalOrientation;
        TBool iLeftToRight;
        TBool iTopToBottom;
        TInt iNumOfColumns;
        TInt iNumOfRows;
        TInt iContentType;
        TSize iSizeOfCell;
        TInt iNumOfItems;

        CAknGridView::TScrollingType iVerticalScrollingType;
        CAknGridView::TScrollingType iHorizontalScrollingType;

        CFbsBitmap* iBitmap;
        CFbsBitmap* iMask;

        CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
    };

#endif

// End of File
