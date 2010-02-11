ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=$(EPOCROOT)epoc32\data\z
endif

TARGETDIR=$(ZDIR)\resource\apps
ICONTARGETFILENAME=$(TARGETDIR)\Podcast.mif
HEADERFILE=$(EPOCROOT)epoc32\include\Podcast.mbg
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
		/c16,1 $(DPATH)\feed_new_40x40.bmp\
		/c16,1 $(DPATH)\feed_error_40x40.bmp\
		/c16 $(DPATH)\emptyimage.bmp
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing

