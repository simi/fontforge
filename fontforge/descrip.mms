# Makefile for OpenVMS
# Date : 8 May 2008

CFLAGS=/nowarn/incl=([-.inc])/name=(as_is,short)/define=(\
	"_STATIC_LIBFREETYPE=1","_STATIC_LIBPNG=1","HAVE_LIBINTL_H=1",\
	"_STATIC_LIBUNINAMESLIST=1","_STATIC_LIBXML=1","_NO_XINPUT=1",\
	"_STATIC_LIBUNGIF=1","_STATIC_LIBJPEG=1","_STATIC_LIBTIFF=1",\
	"_NO_PYTHON=1","_NO_LIBSPIRO=1","fork=vfork",\
        "FONTFORGE_CONFIG_DEVICETABLES=1","PLUGINDIR=""/FONTFORGE$PLUGINS""",\
	"HAVE_PTHREAD_H=1")

fontforge_LIBOBJECTS = asmfpst.obj,autohint.obj,autosave.obj,autotrace.obj,autowidth.obj,\
 bezctx_ff.obj,bitmapchar.obj,bitmapcontrol.obj,bvedit.obj,clipnoui.obj,crctab.obj,\
 cvexport.obj,cvimages.obj,cvundoes.obj,dumpbdf.obj,dumppfa.obj,effects.obj,encoding.obj

fontforge_LIBOBJECTS1=featurefile.obj,fontviewbase.obj,freetype.obj,fvcomposit.obj,fvfonts.obj,fvimportbdf.obj,\
 fvmetrics.obj,glyphcomp.obj,http.obj,ikarus.obj,lookups.obj,macbinary.obj

fontforge_LIBOBJECTS2=macenc.obj,mathconstants.obj,mm.obj,namelist.obj,nonlineartrans.obj,noprefs.obj,nouiutil.obj

fontforge_LIBOBJECTS3=nowakowskittfinstr.obj,ofl.obj,othersubrs.obj,palmfonts.obj,parsepdf.obj,parsepfa.obj,\
 parsettfatt.obj,parsettfbmf.obj,parsettf.obj,parsettfvar.obj,plugins.obj,print.obj

fontforge_LIBOBJECTS4=psread.obj,pua.obj,python.obj,savefont.obj,scripting.obj,scstyles.obj,search.obj

fontforge_LIBOBJECTS5=sfd1.obj,sfd.obj,sflayout.obj,spiro.obj,splinechar.obj,splinefill.obj,\
 splinefont.obj,splineorder2.obj,splineoverlap.obj,splinerefigure.obj,\
 splinesaveafm.obj,splinesave.obj,splinestroke.obj,splineutil2.obj,splineutil.obj

fontforge_LIBOBJECTS6=start.obj,stemdb.obj,svg.obj,tottfaat.obj,tottfgpos.obj,tottf.obj,\
 tottfvar.obj,ttfinstrs.obj,ttfspecial.obj,ufo.obj,unicoderange.obj,utils.obj,\
 winfonts.obj,zapfnomen.obj,groups.obj,langfreq.obj

fontforge_LIBOBJECTS7=libstamp.obj,exelibstamp.obj

fontforge_UIOBJECTS = alignment.obj,anchorsaway.obj,autowidthdlg.obj,basedlg.obj,\
 bdfinfo.obj,bitmapdlg.obj,bitmapview.obj,charinfo.obj,charview.obj,clipui.obj,\
 combinations.obj,contextchain.obj,cursors.obj,cvaddpoints.obj,cvdebug.obj,cvdgloss.obj,\
 cvexportdlg.obj,cvfreehand.obj,cvgetinfo.obj,cvgridfit.obj,cvhand.obj,cvhints.obj,\
 cvimportdlg.obj,cvknife.obj,cvpalettes.obj,cvpointer.obj,cvruler.obj,cvshapes.obj,\
 cvstroke.obj,cvtranstools.obj,displayfonts.obj,effectsui.obj,encodingui.obj,\
 fontinfo.obj,fontview.obj,freetypeui.obj,fvfontsdlg.obj,fvmetricsdlg.obj,gotodlg.obj,\
 groupsdlg.obj,histograms.obj,images.obj,kernclass.obj,layer2layer.obj,lookupui.obj,\
 macencui.obj,math.obj,metricsview.obj,mmdlg.obj,nonlineartransui.obj,openfontdlg.obj,\
 prefs.obj,problems.obj,pythonui.obj,savefontdlg.obj,scriptingdlg.obj,scstylesui.obj,\
 searchview.obj,sftextfield.obj,showatt.obj,simplifydlg.obj,splashimage.obj,stamp.obj,\
 startui.obj,statemachine.obj,tilepath.obj,transform.obj,ttfinstrsui.obj,uiutil.obj,\
 windowmenu.obj

fontforge.exe : $(fontforge_UIOBJECTS) lff.opt xlib.opt\
	[-.libs]libfontforge.exe [-.libs]LIBGDRAW.olb
	library/create tmp.olb $(fontforge_UIOBJECTS)
        link/exec=fontforge.exe startui.obj,tmp/lib,[-.libs]LIBGDRAW/lib,\
	[]lff/opt,xlib.opt/opt
	delete tmp.olb;*

[-.libs]libfontforge.exe : $(fontforge_LIBOBJECTS) $(fontforge_LIBOBJECTS1)\
	$(fontforge_LIBOBJECTS2) $(fontforge_LIBOBJECTS3)\
	$(fontforge_LIBOBJECTS4) $(fontforge_LIBOBJECTS5)\
	$(fontforge_LIBOBJECTS6) $(fontforge_LIBOBJECTS7) [-.libs]LIBGUTIL.olb\
	[-.libs]LIBGUNICODE.olb
	@ WRITE_ SYS$OUTPUT "  generating lff1.opt"
	@ OPEN_/WRITE FILE  lff1.opt
	@ WRITE_ FILE "!"
	@ WRITE_ FILE "! lff1.opt generated by DESCRIP.$(MMS_EXT)" 
	@ WRITE_ FILE "!"
	@ WRITE_ FILE "IDENTIFICATION=""lff"""
	@ WRITE_ FILE "GSMATCH=LEQUAL,1,0
	@ WRITE_ FILE "$(fontforge_LIBOBJECTS)"
	@ WRITE_ FILE "$(fontforge_LIBOBJECTS1)"
	@ WRITE_ FILE "$(fontforge_LIBOBJECTS2)"
	@ WRITE_ FILE "$(fontforge_LIBOBJECTS3)"
	@ WRITE_ FILE "$(fontforge_LIBOBJECTS4)"
	@ WRITE_ FILE "$(fontforge_LIBOBJECTS5)"
	@ WRITE_ FILE "$(fontforge_LIBOBJECTS6)"
	@ WRITE_ FILE "$(fontforge_LIBOBJECTS7)"
	@ librar/extract=* [-.libs]libgutil
	@ write_ file "libgutil.obj"
	@ librar/extract=* [-.libs]libgunicode
	@ write_ file "libgunicode.obj"
	@ CLOSE_ FILE
	@ $(MMS)$(MMSQUALIFIERS)/ignore=warning lff_vms
	@ WRITE_ SYS$OUTPUT "  linking libfontforge.exe ..."
	@ LINK_/NODEB/SHARE=[-.libs]libfontforge.exe/MAP=lff.map/FULL \
	lff1.opt/opt,lff_vms.opt/opt,[-.fontforge]xlib.opt/opt
	@ delete libgunicode.obj;*,libgutil.obj;*
	library/create [-.libs]libfontforge.olb $(fontforge_OBJECTS)
	library [-.libs]libfontforge.olb $(fontforge_LIBOBJECTS1)
	library [-.libs]libfontforge.olb $(fontforge_LIBOBJECTS2)
	library [-.libs]libfontforge.olb $(fontforge_LIBOBJECTS3)
	library [-.libs]libfontforge.olb $(fontforge_LIBOBJECTS4)
	library [-.libs]libfontforge.olb $(fontforge_LIBOBJECTS5)
	library [-.libs]libfontforge.olb $(fontforge_LIBOBJECTS6)
	library [-.libs]libfontforge.olb $(fontforge_LIBOBJECTS7)

lff_vms :
	@ WRITE_ SYS$OUTPUT "  generating lff.map ..."
	@ LINK_/NODEB/NOSHARE/NOEXE/MAP=lff.map/FULL lff1.opt/OPT
	@ WRITE_ SYS$OUTPUT "  analyzing lff.map ..."
	@ @[-.plugins]ANALYZE_MAP.COM lff.map lff_vms.opt

alignment.obj : alignment.c
autohint.obj : autohint.c
autosave.obj : autosave.c
autowidth.obj : autowidth.c
bitmapdlg.obj : bitmapdlg.c
scstyles.obj : scstyles.c
parsettfbmf.obj : parsettfbmf.c
bitmapview.obj : bitmapview.c
bvedit.obj : bvedit.c
charview.obj : charview.c
cursors.obj : cursors.c
cvaddpoints.obj : cvaddpoints.c
cvexport.obj : cvexport.c
cvgetinfo.obj : cvgetinfo.c
cvhints.obj : cvhints.c
cvimages.obj : cvimages.c
cvknife.obj : cvknife.c
cvpalettes.obj : cvpalettes.c
cvpointer.obj : cvpointer.c
cvruler.obj : cvruler.c
cvshapes.obj : cvshapes.c
cvstroke.obj : cvstroke.c
cvtranstools. : cvtranstools.c
cvundoes.obj : cvundoes.c
dumpbdf.obj : dumpbdf.c
dumppfa.obj : dumppfa.c
fontinfo.obj : fontinfo.c
fontview.obj : fontview.c
fvcomposit.obj : fvcomposit.c
fvfonts.obj : fvfonts.c
fvimportbdf.obj : fvimportbdf.c
fvmetrics.obj : fvmetrics.c
images.obj : images.c
metricsview.obj : metricsview.c
parsepfa.obj : parsepfa.c
parsettf.obj : parsettf.c
prefs.obj : prefs.c
psread.obj : psread.c
namelist.obj : namelist.c
savefontdlg.ob : savefontdlg.c
sfd.obj : sfd.c
splashimage.obj : splashimage.c
splinefill.obj : splinefill.c
splineoverlap.obj : splineoverlap.c
splinesave.obj : splinesave.c
splinesaveafm.obj : splinesaveafm.c
splinestroke.obj : splinestroke.c
splineutil.obj : splineutil.c
splineutil2.obj : splineutil2.c
stamp.obj : stamp.c
start.obj : start.c
tottf.obj : tottf.c
          $(CC) $(CFLAGS)/noop tottf
transform.obj : transform.c
uiutil.obj : uiutil.c
utils.obj : utils.c
windowmenu.obj : windowmenu.c
zapfnomen.obj : zapfnomen.c
othersubrs.obj : othersubrs.c
autotrace.obj : autotrace.c
openfontdlg.obj : openfontdlg.c
encoding.obj : encoding.c
problems.obj : problems.c
crctab.obj : crctab.c
macbinary.obj : macbinary.c
scripting.obj : scripting.c
displayfonts.obj : displayfonts.c
combinations.obj : combinations.c
sftextfield.obj : sftextfield.c
ikarus.obj : ikarus.c
cvfreehand.obj : cvfreehand.c
cvhand.obj : cvhand.c
simplifydlg.obj : simplifydlg.c
winfonts.obj : winfonts.c
freetype.obj : freetype.c
gotodlg.obj : gotodlg.c
search.obj : search.c
tottfgpos.obj : tottfgpos.c
charinfo.obj : charinfo.c
tottfaat.obj : tottfaat.c
          $(CC) $(CFLAGS)/noop tottfaat
splineorder2.obj : splineorder2.c
ttfinstrs.obj : ttfinstrs.c
cvgridfit.obj : cvgridfit.c
cvdebug.obj : cvdebug.c
showatt.obj : showatt.c
kernclass.obj : kernclass.c
nonlineartrans.obj : nonlineartrans.c
effects.obj : effects.c
histograms.obj : histograms.c
ttfspecial.obj : ttfspecial.c
svg.obj : svg.c
parsettfatt.obj : parsettfatt.c
contextchain.obj : contextchain.c
macenc.obj : macenc.c
statemachine.obj : statemachine.c
splinerefigure.obj : splinerefigure.c
mm.obj : mm.c
parsettfvar.obj : parsettfvar.c
tottfvar.obj : tottfvar.c
pua.obj : pua.c
stemdb.obj : stemdb.c
anchorsaway.obj : anchorsaway.c
palmfonts.obj : palmfonts.c
cvdgloss.obj : cvdgloss.c
groups.obj : groups.c
parsepdf.obj : parsepdf.c
plugins.obj : plugins.c
startui.obj : startui.c
bdfinfo.obj : bdfinfo.c
glyphcomp.obj : glyphcomp.c
unicoderange.obj : unicoderange.c
ufo.obj : ufo.c
ofl.obj : ofl.c
lookups.obj : lookups.c
sfd1.obj : sfd1.c
python.obj : python.c
featurefile.obj : featurefile.c
math.obj : math.c
nowakowskittfinstr.obj : nowakowskittfinstr.c
http.obj : http.c
spiro.obj : spiro.c
bezctx_ff.obj : bezctx_ff.c
scriptingdlg.obj : scriptingdlg.c
fvfontsdlg.obj : fvfontsdlg.c
splinefont.obj : splinefont.c
splinechar.obj : splinechar.c
cvexportdlg.obj : cvexportdlg.c
cvimportdlg.obj : cvimportdlg.c
encodingui.obj : encodingui.c
bitmapchar.obj : bitmapchar.c
lookupui.obj : lookupui.c
nouiutil.obj : nouiutil.c
noprefs.obj : noprefs.c
bitmapcontrol.obj : bitmapcontrol.c
fontviewbase.obj : fontviewbase.c
mathconstants.obj : mathconstants.c
print.obj : print.c
asmfpst.obj : asmfpst.c
sflayout.obj : sflayout.c
searchview.obj : searchview.c
nonlineartransui.obj : nonlineartransui.c
scstylesui.obj : scstylesui.c
groupsdlg.obj : groupsdlg.c
fvmetricsdlg.obj : fvmetricsdlg.c
clipnoui.obj : clipnoui.c
autowidthdlg.obj : autowidthdlg.c
macencui.obj : macencui.c
savefont.obj : savefont.c
mmdlg.obj : mmdlg.c
effectsui.obj : effectsui.c
langfreq.obj :langfreq.c
ttfinstrsui.obj : ttfinstrsui.c
libstamp.obj : libstamp.pre
	pipe gsed -e "s/REPLACE_ME_WITH_MAJOR_VERSION/1/"\
	-e "s/REPLACE_ME_WITH_MINOR_VERSION/0/" libstamp.pre > libstamp.c
	cc $(CFLAGS) libstamp.c
	delete libstamp.c;*
exelibstamp.obj : exelibstamp.pre
	pipe gsed -e "s/REPLACE_ME_WITH_MAJOR_VERSION/1/"\
	-e "s/REPLACE_ME_WITH_MINOR_VERSION/0/" exelibstamp.pre > exelibstamp.c
	cc $(CFLAGS) exelibstamp.c
	delete exelibstamp.c;*
clipui.obj : clipui.c
layer2layer.obj : layer2layer.c
basedlg.obj : basedlg.c
