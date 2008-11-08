/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastListView.h"

#include <aknnavide.h> 
#include <podcast.rsg>
#include <aknlists.h> 
#include <aknviewappui.h>

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

	protected:
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
		CEikFormattedCellListBox * iListbox;

	};

CPodcastListContainer::CPodcastListContainer()
{
}

void CPodcastListContainer::ConstructL( const TRect& aRect )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    iListbox =static_cast<CEikFormattedCellListBox*>( new (ELeave) CAknDoubleStyleListBox);
	iListbox->SetMopParent( this );
	iListbox->SetContainerWindowL(*this);
	iListbox->ConstructL(this, 0);
	iListbox->CreateScrollBarFrameL(ETrue);
	iListbox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto );    
	iListbox->SetSize(aRect.Size());
	CDesCArray* items = new (ELeave) CDesCArrayFlat(5);
	items->AppendL(_L("\tLine one\tLine two"));
	iListbox->Model()->SetItemTextArray(items);
	iListbox->HandleItemAdditionL();
	iListbox->MakeVisible(ETrue);
    MakeVisible(EFalse);
	// Activate the window, which makes it ready to be drawn
    ActivateL();   
}

TInt CPodcastListContainer::CountComponentControls() const
    {
    return 1; // return number of controls inside this container
    }

CCoeControl* CPodcastListContainer::ComponentControl(TInt aIndex) const
    {
    switch ( aIndex )
        {
        case 0:
            return iListbox;
        default:
            return NULL;
        }
    }

void CPodcastListContainer::HandleResourceChange(TInt aType)
{
		switch( aType )
    	{
	    case KEikDynamicLayoutVariantSwitch:
		    SetRect(iEikonEnv->EikAppUi()->ClientRect());
		    break;
	    }
}


void CPodcastListContainer::SizeChanged()
{
	if(iListbox != NULL)
	{
		iListbox->SetSize(Size());
	}
}


CPodcastListContainer::~CPodcastListContainer()
{
	delete iNaviDecorator;
}

CPodcastListView::CPodcastListView()
{
}

void CPodcastListView::ConstructL()
{
	iListContainer = new (ELeave) CPodcastListContainer;
	iListContainer->SetMopParent( this );
	iListContainer->ConstructL(ClientRect());
}

void CPodcastListView::HandleViewRectChange()
{
    if ( iListContainer )
	{
        iListContainer->SetRect( ClientRect() );
	}
}

void CPodcastListView::HandleStatusPaneSizeChange()
{
	if ( iListContainer )
	{
        iListContainer->SetRect( ClientRect() );
	}
}

    
CPodcastListView::~CPodcastListView()
    {
    delete iListContainer;    
    }

	
void CPodcastListView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
									  const TDesC8& aCustomMessage)
{
	if(iListContainer)
	{
		iListContainer->SetRect(ClientRect());
		AppUi()->AddToViewStackL( *this, iListContainer );	
		iListContainer->MakeVisible(ETrue);
		iListContainer->DrawNow();
	}
}

void CPodcastListView::DoDeactivate()
{
	if ( iListContainer )
	{
        AppUi()->RemoveFromViewStack( *this, iListContainer );
	}
}