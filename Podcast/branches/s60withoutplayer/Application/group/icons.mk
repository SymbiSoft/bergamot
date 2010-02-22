ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=$(EPOCROOT)epoc32\data\z
endif

TARGETDIR=$(ZDIR)\resource\apps
ICONTARGETFILENAME=$(TARGETDIR)\Podcast.mif
HEADERFILE=$(EPOCROOT)epoc32\include\Podcast.mbg
DPATH=..\data\images
TPATH=$(DPATH)\toolbars
do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

RESOURCE :	
		mifconv $(ICONTARGETFILENAME) /h$(HEADERFILE) /c16,1\
		$(DPATH)\podcatcher_64px.svg\
		$(DPATH)\Audio.svg\
		$(DPATH)\Audio_blue.svg\
		$(DPATH)\Audio_green.svg\
		$(DPATH)\Audio_pink.svg\
		$(DPATH)\Feed.svg\
		$(DPATH)\Video.svg\
		$(DPATH)\Blue_left.svg\
		$(DPATH)\Blue_right.svg\
		$(DPATH)\Broken_left.svg\
		$(DPATH)\Broken_right.svg\
		$(DPATH)\Download_left.svg\
		$(DPATH)\Download_right.svg\
		$(DPATH)\Green_left.svg\
		$(DPATH)\Green_right.svg\
		$(DPATH)\New_showstate_left.svg\
		$(DPATH)\New_showstate_right.svg\
		$(DPATH)\Not_downloaded_left.svg\
		$(DPATH)\Not_downloaded_right.svg\
		$(DPATH)\Queue_downloaded_left.svg\
		$(DPATH)\Queue_downloaded_right.svg\
		$(DPATH)\Red_left.svg\
		$(DPATH)\Red_right.svg\
		$(TPATH)\podcast_tab_feeds.svg\
		$(TPATH)\podcast_tab_queue.svg\
		$(TPATH)\podcast_tab_search.svg\
		$(TPATH)\podcast_tb_addfeed.svg\
		$(TPATH)\podcast_tb_clear.svg\
		$(TPATH)\podcast_tb_delete.svg\
		$(TPATH)\podcast_tb_download.svg\
		$(TPATH)\podcast_tb_marknew.svg\
		$(TPATH)\podcast_tb_markold.svg\
		$(TPATH)\podcast_tb_remove.svg\
		$(TPATH)\podcast_tb_resume.svg\
		$(TPATH)\podcast_tb_search.svg\
		$(TPATH)\podcast_tb_settings.svg\
		$(TPATH)\podcast_tb_suspend.svg\
		$(TPATH)\podcast_tb_update.svg\
		$(TPATH)\podcast_tb_updateall.svg\
		$(TPATH)\podcast_tb_cancel.svg\
		/c16,1 $(DPATH)\feed_new_40x40.bmp\
		/c16,1 $(DPATH)\feed_error_40x40.bmp\
		/c16 $(DPATH)\emptyimage.bmp
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing

