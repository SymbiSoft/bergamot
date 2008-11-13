#include "PodcastAppui.h"
#include <Podcast.rsg>
#include "Podcast.hrh"
#include "PodcastMainView.h"
#include "PodcastFeedView.h"
#include "PodcastShowsView.h"
#include "PodcastPlayView.h"
#include "PodcastSettingsView.h"
#include "ShowEngine.h"
#include "PodcastModel.h"

#include <avkon.hrh>

#include "sqlite3.h"
#include "debug.h"
#include "libc\string.h"
#include "libc\stdio.h"

void CPodcastAppUi::ConstructL()
    {
    BaseConstructL( CAknAppUi::EAknEnableSkin );
    iPodcastModel = CPodcastModel::NewL();

    iMainView = CPodcastMainView::NewL(*iPodcastModel);
	this->AddViewL(iMainView);

	iFeedView = CPodcastFeedView::NewL(*iPodcastModel);
	this->AddViewL(iFeedView);

	iShowsView = CPodcastShowsView::NewL(*iPodcastModel);
	this->AddViewL(iShowsView);

	iPlayView = CPodcastPlayView::NewL(*iPodcastModel);
	this->AddViewL(iPlayView);

	iSettingsView = CPodcastSettingsView::NewL(*iPodcastModel);
	this->AddViewL(iSettingsView);
	
	DBTest();
    }

CPodcastAppUi::~CPodcastAppUi()
    {	
	delete iPodcastModel;
    }

// -----------------------------------------------------------------------------
// CPodcastAppUi::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CPodcastAppUi::DynInitMenuPaneL(
    TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
    {
    // no implementation required 
    }

// -----------------------------------------------------------------------------
// CPodcastAppUi::HandleCommandL(TInt aCommand)
// takes care of command handling
// -----------------------------------------------------------------------------
//
void CPodcastAppUi::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EAknSoftkeyExit:
        case EEikCmdExit:
            {
            Exit();
            break;
            }
	
        default:
            break;      
        }
    }

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	  int i;
	  for(i=0; i<argc; i++){
	    DP2("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	  }
	  //printf("\n");
	  return 0;
	}


void insert_data(sqlite3 *db)
	{
	 char sql[1000];
	 char *error;
	  for (int i=0;i<100;i++) {
	  	sprintf(sql, "insert into test (id, msg) values(%d, \"msg %d\");", i, i);
	  	int rc = sqlite3_exec(db, (const char*) sql, NULL, (void*)NULL,&error);
	  	
	  	if (rc != SQLITE_OK) {
	  		TBuf8<100> error8;
	  		error8.Format(_L8("%d:Error: %s"), i, error);
	  		TBuf<100> buf;
	  		buf.Copy(error8);
	  		DP1("%S", &buf);
	  		//break;
	  	}
	  }
	}

void read_data(sqlite3 *db)
	{
	 sqlite3_stmt *st;
	  char *sqlstr =  "select * from test";
	  const char *tail;
	  int rc = sqlite3_prepare(db,(const char*)sqlstr , -1, &st, (const char**) &tail);
	  
	  if( rc==SQLITE_OK ){
	  	rc = sqlite3_step(st);
	  	
	  	while (rc == SQLITE_ROW) {
	  		const unsigned char* str = sqlite3_column_text(st, 0);
	  		int id = sqlite3_column_int(st, 0);
	  		
	  		TPtrC8 ptr((unsigned char*)str);
	  		TBuf<1000> buf;
	  		buf.Copy(ptr);
	  			  
	  		DP2("msg=%S, id=%d", &buf, id);
	  		rc = sqlite3_step(st);  	
	  	}
	  }
	  
	  sqlite3_finalize(st);

	}
void CPodcastAppUi::DBTest()
	{
	  sqlite3 *db;
	  char *zErrMsg = 0;
	  int rc;

	  rc = sqlite3_open("c:\\logs\\escarpod.sqlite", &db);
	  sqlite3_extended_result_codes(db, 1);

	  if( rc == SQLITE_OK){
		read_data(db);
		TTime beforeInsert;
		beforeInsert.UniversalTime();
		insert_data(db);
		TTime afterInsert;
		afterInsert.UniversalTime();
			
		TTimeIntervalSeconds interval;
		afterInsert.SecondsFrom(beforeInsert,interval);
			
		DP1("Interval: %d", interval.Int());
	
	  }
	  
	  
	  sqlite3_close(db);


	}
