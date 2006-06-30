/* Copyright (C) 2000-2006 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _GGADGET_H
#define _GGADGET_H

#include "gdraw.h"
#include "intl.h"
struct giocontrol;

typedef struct gtextinfo {
    unichar_t *text;
    GImage *image;
    Color fg;
    Color bg;
    void *userdata;
    GFont *font;
    unsigned int disabled: 1;
    unsigned int image_precedes: 1;
    unsigned int checkable: 1;			/* Only for menus */
    unsigned int checked: 1;			/* Only for menus */
    unsigned int selected: 1;			/* Only for lists (used internally for menu(bar)s, when cursor is on the line) */
    unsigned int line: 1;			/* Only for menus */
    unsigned int text_is_1byte: 1;		/* If passed in as 1byte (ie. iso-8859-1) text, will be converted */
    unsigned int text_in_resource: 1;		/* the text field is actually an index into the string resource table */
    unsigned int changed: 1;			/* If a row/column widget changed this */
    unichar_t mnemonic;				/* Only for menus and menubars */
						/* should really be in menuitem, but that wastes space and complicates GTextInfoDraw */
} GTextInfo;

typedef struct gmenuitem {
    GTextInfo ti;
    unichar_t shortcut;
    short short_mask;
    struct gmenuitem *sub;
    void (*moveto)(struct gwindow *base,struct gmenuitem *mi,GEvent *);	/* called before creating submenu */
    void (*invoke)(struct gwindow *base,struct gmenuitem *mi,GEvent *);	/* called on mouse release */
    int mid;
} GMenuItem;

typedef struct tabinfo {
    unichar_t *text;
    struct ggadgetcreatedata *gcd;
    unsigned int disabled: 1;
    unsigned int selected: 1;
    unsigned int text_is_1byte: 1;		/* If passed in as 1byte (ie. iso-8859-1) text, will be converted */
    unsigned int text_in_resource: 1;		/* the text field is actually an index into the string resource table */
} GTabInfo;

enum border_type { bt_none, bt_box, bt_raised, bt_lowered, bt_engraved,
	    bt_embossed, bt_double };
enum border_shape { bs_rect, bs_roundrect, bs_elipse, bs_diamond };
enum box_flags {
    box_foreground_border_inner = 1,	/* 1 point line */
    box_foreground_border_outer = 2,	/* 1 point line */
    box_active_border_inner = 4,		/* 1 point line */
    box_foreground_shadow_outer = 8,	/* 1 point line, bottom&right */
    box_do_depressed_background = 0x10,
    box_draw_default = 0x20,	/* if a default button draw a depressed rect around button */
    box_generate_colors = 0x40	/* use border_brightest to compute other border cols */
    };
typedef struct gbox {
    unsigned char border_type;	
    unsigned char border_shape;	
    unsigned char border_width;	/* In points */
    unsigned char padding;	/* In points */
    unsigned char rr_radius;	/* In points */
    unsigned char flags;
    Color border_brightest;		/* used for left upper part of elipse */
    Color border_brighter;
    Color border_darkest;		/* used for right lower part of elipse */
    Color border_darker;
    Color main_background;
    Color main_foreground;
    Color disabled_background;
    Color disabled_foreground;
    Color active_border;
    Color depressed_background;
} GBox;

typedef struct ggadget GGadget;
typedef struct ggadget *GGadgetSet;

enum sb_type { sb_upline, sb_downline, sb_uppage, sb_downpage, sb_track, sb_trackrelease };

typedef int (*GGadgetHandler)(GGadget *,GEvent *);

typedef struct ggadgetdata {
    GRect pos;
    GBox *box;
    unichar_t mnemonic;
    unichar_t shortcut;
    uint8 short_mask;
    uint8 cols;			/* for rowcol */
    short cid;
    GTextInfo *label;
    union {
	GTextInfo *list;	/* for List Widgets (and ListButtons, RowCols etc) */
	GTabInfo *tabs;		/* for Tab Widgets */
	GMenuItem *menu;	/* for menus */
    } u;
    enum gg_flags { gg_visible=1, gg_enabled=2, gg_pos_in_pixels=4,
	gg_sb_vert=8, gg_line_vert=gg_sb_vert,
	gg_but_default=0x10, gg_but_cancel=0x20,
	gg_cb_on=0x40, gg_rad_startnew=0x80,
	gg_list_alphabetic=0x100, gg_list_multiplesel=0x200,
	gg_list_exactlyone=0x400, gg_list_internal=0x800,
	gg_group_prevlabel=0x1000, gg_group_end=0x2000,
	gg_textarea_wrap=0x4000,
	gg_tabset_scroll=0x8000, gg_tabset_filllines=0x10000, gg_tabset_fill1line = 0x20000,
	gg_rowcol_alphabetic=gg_list_alphabetic,
	gg_rowcol_vrules=0x40000, gg_rowcol_hrules=0x800000,
	gg_rowcol_displayonly=0x1000000,
	gg_dontcopybox=0x10000000,
	gg_pos_use0=0x20000000, gg_pos_under=0x40000000,
	gg_pos_newline = (int) 0x80000000,
	/* Reuse some flag values for different widgets */
	gg_file_pulldown=gg_sb_vert, gg_file_multiple = gg_list_multiplesel,
	gg_text_xim = gg_tabset_scroll,
	gg_tabset_vert = gg_sb_vert,
	gg_utf8_popup = gg_rowcol_displayonly
	} flags;
    const unichar_t *popup_msg;		/* Brief help message */
    GGadgetHandler handle_controlevent;
} GGadgetData;

typedef struct ggadgetcreatedata {
    GGadget *(*creator)(struct gwindow *base, GGadgetData *gd,void *data);
    GGadgetData gd;
    void *data;
    GGadget *ret;
} GGadgetCreateData;

enum editor_commands { ec_cut, ec_clear, ec_copy, ec_paste, ec_undo, ec_redo,
	ec_selectall, ec_search, ec_backsearch, ec_backword, ec_deleteword,
	ec_max };

    /* return values from file chooser filter functions */
enum fchooserret { fc_hide, fc_show, fc_showdisabled };

struct gdirentry;
typedef enum fchooserret (*GFileChooserFilterType)(GGadget *g,struct gdirentry *ent,
	const unichar_t *dir);

#define _STR_NULL	(-1)		/* Null string resource */
#define _STR_Language	0
#define _STR_OK		1
#define _STR_Cancel	2
#define _STR_Open	3
#define _STR_Save	4
#define _STR_Filter	5
#define _STR_New	6
#define _STR_Replace	7
#define _STR_Fileexists	8
#define _STR_Fileexistspre	9
#define _STR_Fileexistspost	10
#define _STR_Createdir	11
#define _STR_Dirname	12
#define _STR_Couldntcreatedir	13
#define _STR_SelectAll	14
#define _STR_None	15
#define __STR_LastStd	15

#define _NUM_Buttonsize		0
#define _NUM_ScaleFactor	1
#define __NUM_LastStd		1

extern void GTextInfoFree(GTextInfo *ti);
extern void GTextInfoListFree(GTextInfo *ti);
extern void GTextInfoArrayFree(GTextInfo **ti);
extern GTextInfo **GTextInfoFromChars(char **array, int len);
extern const unichar_t *GStringGetResource(int index,unichar_t *mnemonic);
extern int GGadgetScale(int xpos);
extern int GIntGetResource(int index);
extern int GStringSetResourceFileV(char *filename,uint32 checksum);
extern int GStringSetResourceFile(char *filename);	/* returns 1 for success, 0 for failure */
/* fallback string arrays are null terminated. mnemonics is same length as string */
/* fallback integer arrays are terminated by 0x80000000 (negative infinity) */
extern void GStringSetFallbackArray(const unichar_t **array,const unichar_t *mn,
	const int *ires);
unichar_t *GStringFileGetResource(char *filename, int index,unichar_t *mnemonic);
extern void GResourceUseGetText(void);

void GGadgetDestroy(GGadget *g);
void GGadgetSetVisible(GGadget *g,int visible);
void GGadgetSetEnabled(GGadget *g,int enabled);
GWindow GGadgetGetWindow(GGadget *g);
void *GGadgetGetUserData(GGadget *g);
void GGadgetSetUserData(GGadget *g, void *d);
void GGadgetSetPopupMsg(GGadget *g, const unichar_t *msg);
GRect *GGadgetGetInnerSize(GGadget *g,GRect *rct);
GRect *GGadgetGetSize(GGadget *g,GRect *rct);
int GGadgetGetCid(GGadget *g);
void GGadgetResize(GGadget *g,int32 width, int32 height );
void GGadgetMove(GGadget *g,int32 x, int32 y );
void GGadgetRedraw(GGadget *g);
void GGadgetsCreate(GWindow base, GGadgetCreateData *gcd);

void GGadgetSetTitle(GGadget *g,const unichar_t *title);
void GGadgetSetTitle8(GGadget *g,const char *title);
const unichar_t *_GGadgetGetTitle(GGadget *g);	/* Do not free!!! */
unichar_t *GGadgetGetTitle(GGadget *g);		/* Free the return */
char *GGadgetGetTitle8(GGadget *g);		/* Free the return (utf8) */
void GGadgetSetFont(GGadget *g,GFont *font);
GFont *GGadgetGetFont(GGadget *g);
int GGadgetEditCmd(GGadget *g,enum editor_commands cmd);
int GGadgetActiveGadgetEditCmd(GWindow gw,enum editor_commands cmd);
void GGadgetSetHandler(GGadget *g, GGadgetHandler handler);
GGadgetHandler GGadgetGetHandler(GGadget *g);

void GTextFieldSelect(GGadget *g,int sel_start, int sel_end);
void GTextFieldShow(GGadget *g,int pos);
void GTextFieldReplace(GGadget *g,const unichar_t *txt);
void GGadgetClearList(GGadget *g);
void GGadgetSetList(GGadget *g, GTextInfo **ti, int32 copyit);
GTextInfo **GGadgetGetList(GGadget *g,int32 *len);	/* Do not free!!! */
GTextInfo *GGadgetGetListItem(GGadget *g,int32 pos);
GTextInfo *GGadgetGetListItemSelected(GGadget *g);
void GGadgetSelectListItem(GGadget *g,int32 pos,int32 sel);
void GGadgetSelectOneListItem(GGadget *g,int32 pos);
int32 GGadgetIsListItemSelected(GGadget *g,int32 pos);
int32 GGadgetGetFirstListSelectedItem(GGadget *g);
void GGadgetScrollListToPos(GGadget *g,int32 pos);
void GGadgetScrollListToText(GGadget *g,const unichar_t *lab,int32 sel);
void GGadgetSetListOrderer(GGadget *g,int (*orderer)(const void *, const void *));

void GGadgetSetChecked(GGadget *g, int ison);
int GGadgetIsChecked(GGadget *g);

int GListIndexFromY(GGadget *g,int y);

int GTabSetGetSel(GGadget *g);
void GTabSetSetSel(GGadget *g,int sel);
void GTabSetSetEnabled(GGadget *g,int pos, int enabled);
GWindow GTabSetGetSubwindow(GGadget *g,int pos);
int GTabSetGetTabLines(GGadget *g);
void GTabSetSetNestedExpose(GGadget *g, void (*)(GWindow,GGadget *,GEvent *));
void GTabSetSetNestedMouse(GGadget *g, int (*)(GGadget *,GEvent *));

int32 GScrollBarGetPos(GGadget *g);
int32 GScrollBarSetPos(GGadget *g,int32 pos);
void GScrollBarSetMustShow(GGadget *g, int32 sb_min, int32 sb_max, int32 sb_pagesize,
	int32 sb_mustshow);
void GScrollBarSetBounds(GGadget *g, int32 sb_min, int32 sb_max, int32 sb_pagesize );
void GScrollBarGetBounds(GGadget *g, int32 *sb_min, int32 *sb_max, int32 *sb_pagesize );

void GMenuBarSetItemChecked(GGadget *g, int mid, int check);
void GMenuBarSetItemEnabled(GGadget *g, int mid, int enabled);
void GMenuBarSetItemName(GGadget *g, int mid, const unichar_t *name);

void GFileChooserPopupCheck(GGadget *g,GEvent *e);
void GFileChooserFilterIt(GGadget *g);
void GFileChooserRefreshList(GGadget *g);
int GFileChooserFilterEh(GGadget *g,GEvent *e);
void GFileChooserConnectButtons(GGadget *g,GGadget *ok, GGadget *filter);
void GFileChooserSetFilterText(GGadget *g,const unichar_t *filter);
void GFileChooserSetFilterFunc(GGadget *g,GFileChooserFilterType filter);
void GFileChooserSetDir(GGadget *g,unichar_t *dir);
struct giocontrol *GFileChooserReplaceIO(GGadget *g,struct giocontrol *gc);
unichar_t *GFileChooserGetDir(GGadget *g);
unichar_t *GFileChooserGetFilterText(GGadget *g);
GFileChooserFilterType GFileChooserGetFilterFunc(GGadget *g);
void GFileChooserSetMimetypes(GGadget *g,unichar_t **mimetypes);
unichar_t **GFileChooserGetMimetypes(GGadget *g);
void GFileChooserGetChildren(GGadget *g,GGadget **pulldown, GGadget **list, GGadget **tf);
int GFileChooserPosIsDir(GGadget *g, int pos);
unichar_t *GFileChooserFileNameOfPos(GGadget *g, int pos);

extern void GGadgetPreparePopup(GWindow base,const unichar_t *msg);
extern void GGadgetPreparePopupR(GWindow base,int msg);
extern void GGadgetPreparePopup8(GWindow base,char *msg);
extern void GGadgetEndPopup(void);
extern void GGadgetPopupExternalEvent(GEvent *e);

extern int GGadgetDispatchEvent(GGadget *g,GEvent *e);

/* Handles *?{}[] wildcards */
int GGadgetWildMatch(unichar_t *pattern, unichar_t *name,int ignorecase);
enum fchooserret GFileChooserDefFilter(GGadget *g,struct gdirentry *ent,
	const unichar_t *dir);

GWindow GMenuCreatePopupMenu(GWindow owner,GEvent *event, GMenuItem *mi);

GGadget *GLineCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GGroupCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GLabelCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GButtonCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GImageButtonCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GListButtonCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GRadioCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GCheckBoxCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GScrollBarCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GListCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GTextFieldCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GPasswordCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GTextAreaCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GListFieldCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GMenuBarCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GTabSetCreate(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *GFileChooserCreate(struct gwindow *base, GGadgetData *gd,void *data);

GGadget *CreateSlider(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *CreateFileChooser(struct gwindow *base, GGadgetData *gd,void *data);
GGadget *CreateGadgets(struct gwindow *base, GGadgetCreateData *gcd);

GTextInfo **GTextInfoArrayFromList(GTextInfo *ti, uint16 *cnt);

extern unichar_t *utf82u_mncopy(const char *utf8buf,unichar_t *mn);

#if 0
void GadgetRedraw(Gadget *g);
void GadgetSetBox(Gadget *g,Box *b);
Box *GadgetGetBox(Gadget *g);
Rect *GadgetGetPos(Gadget *g);
void GadgetMove(Gadget *g, int x, int y);
void GadgetResize(Gadget *g, int width, int height);
void GadgetReposition(Gadget *g, Rect *r);
void GadgetSetVisible(Gadget *g, int visible);
int GadgetIsVisible(Gadget *g);
void GadgetSetEnabled(Gadget *g, int enabled);
int GadgetIsEnabled(Gadget *g);
void GadgetSetFocus(Gadget *g);
int GadgetHasFocus(Gadget *g);
void GadgetSetMnemonic(Gadget *g, int ch);
int GadgetGetMnemonic(Gadget *g);
void GadgetSetAccel(Gadget *g, int ch, short mask);
int GadgetGetAccelChar(Gadget *g);
short GadgetGetAccelMask(Gadget *g);

void GadgetSetTitle(Gadget *g,const unichar_t *title);
unichar_t *GadgetGetTitle(Gadget *g);
void GadgetSetFont(Gadget *g,GFont *font);
GFont *GadgetGetFont(Gadget *g);

void GadgetSetCheck(Gadget *g,int checked);
int GadgetGetChecked(Gadget *g);

void GadgetClearList(Gadget *g);
void GadgetSetListTI(Gadget *g,GTextInfo *ti);
void GadgetSetList(Gadget *g,unichar_t **list);
void GadgetPrependToList(Gadget *g,unichar_t *

void GadgetSetMenuItem(Gadget *,MenuItem *m);
MenuItem *GadgetGetMenuItem(Gadget *);
#endif

#endif
