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
		$(DPATH)\podcatcher_64px.svg\
		$(DPATH)\Audio.svg\
		$(DPATH)\Audio_blue.svg\
		$(DPATH)\Audio_green.svg\
		$(DPATH)\Audio_pink.svg\
		$(DPATH)\Feed.svg\
		$(DPATH)\Video.svg
	
		
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing

