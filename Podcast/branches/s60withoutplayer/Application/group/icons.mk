ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=$(EPOCROOT)epoc32\data\z
endif

TARGETDIR=$(ZDIR)\resource\apps
ICONTARGETFILENAME=$(TARGETDIR)\Podcastsvg.mif
HEADERFILE=$(EPOCROOT)epoc32\include\Podcastsvg.mbg
DPATH=..\data\images

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

RESOURCE :	
		mifconv $(ICONTARGETFILENAME) /h$(HEADERFILE) /c16,1\
		 .\podcast.svg /c16 $(DPATH)\PodcastClient_64x64.bmp\
		  /8 $(DPATH)\PodcastClient_64x64_mask.bmp\
		  /c16 $(DPATH)\PodcastClient_40x40.bmp\
		/8 $(DPATH)\PodcastClient_40x40_mask.bmp\
	/c16 $(DPATH)\PodcastClient_18x18.bmp\
	/8 $(DPATH)\PodcastClient_18x18_mask.bmp\
	/c16 $(DPATH)\feeds_40x40.bmp\
	/8 $(DPATH)\feeds_40x40m.bmp\
	/c16 $(DPATH)\feed_40x40.bmp\
	/8 $(DPATH)\feed_40x40m.bmp\
	/c16 $(DPATH)\play_40x40.bmp\
	/8 $(DPATH)\play_40x40m.bmp\
	/c16 $(DPATH)\downloaded_view_40x40.bmp\
	/8 $(DPATH)\downloaded_view_40x40m.bmp\
	/c16 $(DPATH)\pending_view_40x40.bmp\
	/8 $(DPATH)\pending_view_40x40m.bmp\
	/c16 $(DPATH)\show_40x40.bmp\
	/8 $(DPATH)\show_40x40m.bmp\
	/c16 $(DPATH)\show_playing_40x40.bmp\
	/8 $(DPATH)\show_playing_40x40m.bmp\
	/c16 $(DPATH)\downloading_40x40.bmp\
	/8 $(DPATH)\downloading_40x40m.bmp
	
		
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing

