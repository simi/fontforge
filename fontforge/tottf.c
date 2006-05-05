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
#include "pfaedit.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <utype.h>
#include <ustring.h>
#include <chardata.h>
#include <gwidget.h>

#ifdef __CygWin
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif

#include "ttf.h"

/* This file produces a ttf file given a splinefont. */

/* ************************************************************************** */

/* Required tables:
	cmap		encoding
	head		header data
	hhea		horizontal metrics header data
	hmtx		horizontal metrics (widths, lsidebearing)
	maxp		various maxima in the font
	name		various names associated with the font
	post		postscript names and other stuff
Required by windows but not mac
	OS/2		bleah. 
Required for TrueType
	loca		pointers to the glyphs
	glyf		character shapes
Required for OpenType (Postscript)
	CFF 		A complete postscript CFF font here with all its internal tables
Required for bitmaps
	bdat/EBDT	bitmap data
	bloc/EBLC	pointers to bitmaps
	bhed		for apple bitmap only fonts, replaces head
Optional for bitmaps
	EBSC		bitmap scaling table (used in windows "bitmap-only" fonts)
"Advanced Typograpy"
  Apple
	feat		(mapping between morx features and 'name' names)
	kern		(if data are present)
	lcar		(ligature caret, if data present)
	morx		(substitutions, if data present)
	prop		(glyph properties, if data present)
	opbd		(optical bounds, if data present)
  OpenType
	GPOS		(opentype, if kern,anchor data are present)
	GSUB		(opentype, if ligature (other subs) data are present)
	GDEF		(opentype, if anchor data are present)
Apple variation tables (for distortable (multiple master type) fonts)
	fvar		(font variations)
	gvar		(glyph variations)
	cvar		(cvt variations)
	avar		(axis variations)
additional tables
	cvt		for hinting
	gasp		to control when things should be hinted
	fpgm		for hinting (currently only copied and dumped verbatim)
	prep		for hinting (currently only copied and dumped verbatim)
FontForge
	PfEd		My own table
TeX
	TeX		TeX specific info (stuff that used to live in tfm files)
*/

const char *ttfstandardnames[258] = {
".notdef",
".null",
"nonmarkingreturn",
"space",
"exclam",
"quotedbl",
"numbersign",
"dollar",
"percent",
"ampersand",
"quotesingle",
"parenleft",
"parenright",
"asterisk",
"plus",
"comma",
"hyphen",
"period",
"slash",
"zero",
"one",
"two",
"three",
"four",
"five",
"six",
"seven",
"eight",
"nine",
"colon",
"semicolon",
"less",
"equal",
"greater",
"question",
"at",
"A",
"B",
"C",
"D",
"E",
"F",
"G",
"H",
"I",
"J",
"K",
"L",
"M",
"N",
"O",
"P",
"Q",
"R",
"S",
"T",
"U",
"V",
"W",
"X",
"Y",
"Z",
"bracketleft",
"backslash",
"bracketright",
"asciicircum",
"underscore",
"grave",
"a",
"b",
"c",
"d",
"e",
"f",
"g",
"h",
"i",
"j",
"k",
"l",
"m",
"n",
"o",
"p",
"q",
"r",
"s",
"t",
"u",
"v",
"w",
"x",
"y",
"z",
"braceleft",
"bar",
"braceright",
"asciitilde",
"Adieresis",
"Aring",
"Ccedilla",
"Eacute",
"Ntilde",
"Odieresis",
"Udieresis",
"aacute",
"agrave",
"acircumflex",
"adieresis",
"atilde",
"aring",
"ccedilla",
"eacute",
"egrave",
"ecircumflex",
"edieresis",
"iacute",
"igrave",
"icircumflex",
"idieresis",
"ntilde",
"oacute",
"ograve",
"ocircumflex",
"odieresis",
"otilde",
"uacute",
"ugrave",
"ucircumflex",
"udieresis",
"dagger",
"degree",
"cent",
"sterling",
"section",
"bullet",
"paragraph",
"germandbls",
"registered",
"copyright",
"trademark",
"acute",
"dieresis",
"notequal",
"AE",
"Oslash",
"infinity",
"plusminus",
"lessequal",
"greaterequal",
"yen",
"mu",
"partialdiff",
"summation",
"product",
"pi",
"integral",
"ordfeminine",
"ordmasculine",
"Omega",
"ae",
"oslash",
"questiondown",
"exclamdown",
"logicalnot",
"radical",
"florin",
"approxequal",
"Delta",
"guillemotleft",
"guillemotright",
"ellipsis",
"nonbreakingspace",
"Agrave",
"Atilde",
"Otilde",
"OE",
"oe",
"endash",
"emdash",
"quotedblleft",
"quotedblright",
"quoteleft",
"quoteright",
"divide",
"lozenge",
"ydieresis",
"Ydieresis",
"fraction",
"currency",
"guilsinglleft",
"guilsinglright",
"fi",
"fl",
"daggerdbl",
"periodcentered",
"quotesinglbase",
"quotedblbase",
"perthousand",
"Acircumflex",
"Ecircumflex",
"Aacute",
"Edieresis",
"Egrave",
"Iacute",
"Icircumflex",
"Idieresis",
"Igrave",
"Oacute",
"Ocircumflex",
"apple",
"Ograve",
"Uacute",
"Ucircumflex",
"Ugrave",
"dotlessi",
"circumflex",
"tilde",
"macron",
"breve",
"dotaccent",
"ring",
"cedilla",
"hungarumlaut",
"ogonek",
"caron",
"Lslash",
"lslash",
"Scaron",
"scaron",
"Zcaron",
"zcaron",
"brokenbar",
"Eth",
"eth",
"Yacute",
"yacute",
"Thorn",
"thorn",
"minus",
"multiply",
"onesuperior",
"twosuperior",
"threesuperior",
"onehalf",
"onequarter",
"threequarters",
"franc",
"Gbreve",
"gbreve",
"Idotaccent",
"Scedilla",
"scedilla",
"Cacute",
"cacute",
"Ccaron",
"ccaron",
"dcroat"
};

static int uniranges[][3] = {
    { 0x20, 0x7e, 0 },		/* Basic Latin */
    { 0xa0, 0xff, 1 },		/* Latin-1 Supplement */
    { 0x100, 0x17f, 2 },	/* Latin Extended-A */
    { 0x180, 0x24f, 3 },	/* Latin Extended-B */
    { 0x250, 0x2af, 4 },	/* IPA Extensions */
    { 0x2b0, 0x2ff, 5 },	/* Spacing Modifier Letters */
    { 0x300, 0x36f, 6 },	/* Combining Diacritical Marks */
    { 0x370, 0x3ff, 7 },	/* Greek */
    { 0x400, 0x52f, 9 },	/* Cyrillic / Cyrillic Supplement */
    { 0x530, 0x58f, 10 },	/* Armenian */
    { 0x590, 0x5ff, 11 },	/* Hebrew */
    { 0x600, 0x6ff, 13 },	/* Arabic */
    { 0x700, 0x74f, 71 },	/* Syriac */
    { 0x780, 0x7bf, 72 },	/* Thaana */
    { 0x900, 0x97f, 15 },	/* Devanagari */
    { 0x980, 0x9ff, 16 },	/* Bengali */
    { 0xa00, 0xa7f, 17 },	/* Gurmukhi */
    { 0xa80, 0xaff, 18 },	/* Gujarati */
    { 0xb00, 0xb7f, 19 },	/* Oriya */
    { 0xb80, 0xbff, 20 },	/* Tamil */
    { 0xc00, 0xc7f, 21 },	/* Telugu */
    { 0xc80, 0xcff, 22 },	/* Kannada */
    { 0xd00, 0xd7f, 23 },	/* Malayalam */
    { 0xd80, 0xdff, 73 },	/* Sinhala */
    { 0xe00, 0xe7f, 24 },	/* Thai */
    { 0xe80, 0xeff, 25 },	/* Lao */
    { 0xf00, 0xfbf, 70 },	/* Tibetan */
    { 0x1000, 0x109f, 74 },	/* Myanmar */
    { 0x10a0, 0x10ff, 26 },	/* Georgian */
    { 0x1100, 0x11ff, 28 },	/* Hangul Jamo */
    { 0x1200, 0x137f, 75 },	/* Ethiopic */
    { 0x13a0, 0x13ff, 76 },	/* Cherokee */
    { 0x1400, 0x167f, 77 },	/* Unified Canadian Aboriginal Symbols */
    { 0x1680, 0x169f, 78 },	/* Ogham */
    { 0x16a0, 0x16ff, 79 },	/* Runic */
    { 0x1700, 0x177f, 84 },	/* Tagalog / Harunoo / Buhid / Tagbanwa */
    { 0x1780, 0x17ff, 80 },	/* Khmer */
    { 0x1800, 0x18af, 81 },	/* Mongolian */
    /* { 0x1900, 0x194f, },	   Limbu */
    /* { 0x1950, 0x197f, },	   Tai le */
    /* { 0x19e0, 0x19ff, },	   Khmer Symbols */
    /* { 0x1d00, 0x1d7f, },	   Phonetic Extensions */
    { 0x1e00, 0x1eff, 29 },	/* Latin Extended Additional */
    { 0x1f00, 0x1fff, 30 },	/* Greek Extended */
    { 0x2000, 0x206f, 31 },	/* General Punctuation */
    { 0x2070, 0x209f, 32 },	/* Superscripts and Subscripts */
    { 0x20a0, 0x20cf, 33 },	/* Currency Symbols */
    { 0x20d0, 0x20ff, 34 },	/* Combining Marks for Symbols */
    { 0x2100, 0x214f, 35 },	/* Letterlike Symbols */
    { 0x2150, 0x218f, 36 },	/* Number Forms */
    { 0x2190, 0x21ff, 37 },	/* Arrows */
    { 0x2200, 0x22ff, 38 },	/* Mathematical Operators */
    { 0x2300, 0x237f, 39 },	/* Miscellaneous Technical */
    { 0x2400, 0x243f, 40 },	/* Control Pictures */
    { 0x2440, 0x245f, 41 },	/* Optical Character Recognition */
    { 0x2460, 0x24ff, 42 },	/* Enclosed Alphanumerics */
    { 0x2500, 0x257f, 43 },	/* Box Drawing */
    { 0x2580, 0x259f, 44 },	/* Block Elements */
    { 0x25a0, 0x25ff, 45 },	/* Geometric Shapes */
    { 0x2600, 0x267f, 46 },	/* Miscellaneous Symbols */
    { 0x2700, 0x27bf, 47 },	/* Dingbats */
    { 0x27c0, 0x27ef, 38 },	/* Miscellaneous Mathematical Symbols-A */
    { 0x27f0, 0x27ff, 37 },	/* Supplementary Arrows-A */
    { 0x2800, 0x28ff, 82 },	/* Braille Patterns */
    { 0x2900, 0x297f, 37 },	/* Supplementary Arrows-B */
    { 0x2980, 0x2aff, 38 },	/* Miscellaneous Mathematical Symbols-B /
				   Supplemental Mathematical Operators */
    { 0x2e80, 0x2fff, 59 },	/* CJK Radicals Supplement / Kangxi Radicals /
				   Ideographic Description Characters */
    { 0x3000, 0x303f, 48 },	/* CJK Symbols and Punctuation */
    { 0x3040, 0x309f, 49 },	/* Hiragana */
    { 0x30a0, 0x30ff, 50 },	/* Katakana */
    { 0x3100, 0x312f, 51 },	/* Bopomofo */
    { 0x3130, 0x318f, 52 },	/* Hangul Compatibility Jamo */
    { 0x3190, 0x319f, 59 },	/* Kanbun */
    { 0x31a0, 0x31bf, 51 },	/* Bopomofo Extended */
    { 0x31f0, 0x31ff, 50 },	/* Katakana Phonetic Extensions */
    { 0x3200, 0x32ff, 54 },	/* Enclosed CJK Letters and Months */
    { 0x3300, 0x33ff, 55 },	/* CJK compatability */
    { 0x3400, 0x4dbf, 59 },	/* CJK Unified Ideographs Extension A */
    /* { 0x4dc0, 0x4dff, },	   Yijing Hexagram Symbols */
    { 0x4e00, 0x9fff, 59 },	/* CJK Unified Ideographs */
    { 0xa000, 0xa4cf, 81 },	/* Yi Syllables / Yi Radicals */
    { 0xac00, 0xd7af, 56 },	/* Hangul */
    { 0xe000, 0xf8ff, 60 },	/* Private Use Area */

    { 0xf900, 0xfaff, 61 },	/* CJK Compatibility Ideographs */
    /* 12 ideographs in The IBM 32 Compatibility Additions are CJK unified
       ideographs despite their names: see The Unicode Standard 4.0, p.475 */
    { 0xfa0e, 0xfa0f, 59 },
    { 0xfa10, 0xfa10, 61 },
    { 0xfa11, 0xfa11, 59 },
    { 0xfa12, 0xfa12, 61 },
    { 0xfa13, 0xfa14, 59 },
    { 0xfa15, 0xfa1e, 61 },
    { 0xfa1f, 0xfa1f, 59 },
    { 0xfa20, 0xfa20, 61 },
    { 0xfa21, 0xfa21, 59 },
    { 0xfa22, 0xfa22, 61 },
    { 0xfa23, 0xfa24, 59 },
    { 0xfa25, 0xfa26, 61 },
    { 0xfa27, 0xfa29, 59 },
    { 0xfa2a, 0xfaff, 61 },	/* CJK Compatibility Ideographs */

    { 0xfb00, 0xfb4f, 62 },	/* Alphabetic Presentation Forms */
    { 0xfb50, 0xfdff, 63 },	/* Arabic Presentation Forms-A */
    { 0xfe00, 0xfe0f, 91 },	/* Variation Selectors */
    { 0xfe20, 0xfe2f, 64 },	/* Combining Half Marks */
    { 0xfe30, 0xfe4f, 65 },	/* CJK Compatibility Forms */
    { 0xfe50, 0xfe6f, 66 },	/* Small Form Variants */
    { 0xfe70, 0xfeef, 67 },	/* Arabic Presentation Forms-B */
    { 0xff00, 0xffef, 68 },	/* Halfwidth and Fullwidth Forms */
    { 0xfff0, 0xffff, 69 },	/* Specials */

    /* { 0x10000, 0x1007f, },   Linear B Syllabary */
    /* { 0x10080, 0x100ff, },   Linear B Ideograms */
    /* { 0x10100, 0x1013f, },   Aegean Numbers */
    { 0x10300, 0x1032f, 85 },	/* Old Italic */
    { 0x10330, 0x1034f, 86 },	/* Gothic */
    { 0x10400, 0x1044f, 87 },	/* Deseret */
    /* { 0x10450, 0x1047f, },   Shavian */
    /* { 0x10480, 0x104af, },   Osmanya */
    /* { 0x10800, 0x1083f, },   Cypriot Syllabary */
    { 0x1d000, 0x1d1ff, 88 },	/* Byzantine Musical Symbols / Musical Symbols */
    /* { 0x1d300, 0x1d35f, },   Tai Xuan Jing Symbols */
    { 0x1d400, 0x1d7ff, 89 },	/* Mathematical Alphanumeric Symbols */
    { 0x20000, 0x2a6df, 59 },	/* CJK Unified Ideographs Extension B */
    { 0x2f800, 0x2fa1f, 61 },	/* CJK Compatibility Ideographs Supplement */
    { 0xe0000, 0xe007f, 92 },	/* Tags */
    { 0xe0100, 0xe01ef, 91 },	/* Variation Selectors Supplement */
    { 0xf0000, 0xffffd, 90 },	/* Supplementary Private Use Area-A */
    { 0x100000, 0x10fffd, 90 },	/* Supplementary Private Use Area-B */
};

static int32 getuint32(FILE *ttf) {
    int ch1 = getc(ttf);
    int ch2 = getc(ttf);
    int ch3 = getc(ttf);
    int ch4 = getc(ttf);
    if ( ch4==EOF )
return( EOF );
return( (ch1<<24)|(ch2<<16)|(ch3<<8)|ch4 );
}

void putshort(FILE *file,int sval) {
    putc((sval>>8)&0xff,file);
    putc(sval&0xff,file);
}

void putlong(FILE *file,int val) {
    putc((val>>24)&0xff,file);
    putc((val>>16)&0xff,file);
    putc((val>>8)&0xff,file);
    putc(val&0xff,file);
}
#define dumpabsoffset	putlong

static void dumpoffset(FILE *file,int offsize,int val) {
    if ( offsize==1 )
	putc(val,file);
    else if ( offsize==2 )
	putshort(file,val);
    else if ( offsize==3 ) {
	putc((val>>16)&0xff,file);
	putc((val>>8)&0xff,file);
	putc(val&0xff,file);
    } else
	putlong(file,val);
}

static void put2d14(FILE *file,real dval) {
    int val;
    int mant;

    val = floor(dval);
    mant = floor(16384.*(dval-val));
    val = (val<<14) | mant;
    putshort(file,val);
}

void putfixed(FILE *file,real dval) {
    int val;
    int mant;

    val = floor(dval);
    mant = floor(65536.*(dval-val));
    val = (val<<16) | mant;
    putlong(file,val);
}

int ttfcopyfile(FILE *ttf, FILE *other, int pos, char *tab_name) {
    int ch;
    int ret = 1;

    if ( ferror(ttf) || ferror(other)) {
	IError("Disk error of some nature. Perhaps no space on device?\nGenerated font will be unusable" );
    } else if ( pos!=ftell(ttf)) {
	IError("File Offset wrong for ttf table (%s), %d expected %d", tab_name, ftell(ttf), pos );
    }
    rewind(other);
    while (( ch = getc(other))!=EOF )
	putc(ch,ttf);
    if ( ferror(other)) ret = 0;
    if ( fclose(other)) ret = 0;
return( ret );
}

static void FigureFullMetricsEnd(SplineFont *sf,struct glyphinfo *gi) {
    /* We can reduce the size of the width array by removing a run at the end */
    /*  of the same width. So start at the end, find the width of the last */
    /*  character we'll output, then run backwards as long as we've got the */
    /*  same width */
    /* (do same thing for vertical metrics too */
    int i, lasti, lastv;
    int width, vwidth;

    lasti = lastv = gi->gcnt-1;
    for ( i=gi->gcnt-1; i>3 && gi->bygid[i]==-1; --i );
    if ( i>2 ) {
	width = sf->glyphs[gi->bygid[i]]->width;
	vwidth = sf->glyphs[gi->bygid[i]]->vwidth;
	lasti = lastv = i;
	for ( i=lasti-1; i>3; --i ) {
	    if ( SCWorthOutputting(sf->glyphs[gi->bygid[i]]) ) {
		if ( sf->glyphs[gi->bygid[i]]->width!=width )
	break;
		else
		    lasti = i;
	    }
	}
	gi->lasthwidth = lasti;
	if ( sf->hasvmetrics ) {
	    for ( i=lastv-1; i>3; --i ) {
		if ( SCWorthOutputting(sf->glyphs[gi->bygid[i]]) ) {
		    if ( sf->glyphs[gi->bygid[i]]->vwidth!=vwidth )
	    break;
		    else
			lastv = i;
		}
	    }
	    gi->lastvwidth = lastv;
	}
    } else {
	gi->lasthwidth = 0;
	gi->lastvwidth = 0;
    }
}

static void dumpghstruct(struct glyphinfo *gi,struct glyphhead *gh) {

    putshort(gi->glyphs,gh->numContours);
    putshort(gi->glyphs,gh->xmin);
    putshort(gi->glyphs,gh->ymin);
    putshort(gi->glyphs,gh->xmax);
    putshort(gi->glyphs,gh->ymax);
    if ( gh->xmin<gi->xmin ) gi->xmin = gh->xmin;
    if ( gh->ymin<gi->ymin ) gi->ymin = gh->ymin;
    if ( gh->xmax>gi->xmax ) gi->xmax = gh->xmax;
    if ( gh->ymax>gi->ymax ) gi->ymax = gh->ymax;
}

static void ttfdumpmetrics(SplineChar *sc,struct glyphinfo *gi,DBounds *b) {

    if ( sc->ttf_glyph<=gi->lasthwidth )
	putshort(gi->hmtx,sc->width);
    putshort(gi->hmtx,b->minx);
    if ( sc->parent->hasvmetrics ) {
	if ( sc->ttf_glyph<=gi->lastvwidth )
	    putshort(gi->vmtx,sc->vwidth);
	putshort(gi->vmtx,sc->parent->vertical_origin-b->maxy);
    }
    if ( sc->ttf_glyph==gi->lasthwidth )
	gi->hfullcnt = sc->ttf_glyph+1;
    if ( sc->ttf_glyph==gi->lastvwidth )
	gi->vfullcnt = sc->ttf_glyph+1;
}

static SplineSet *SCttfApprox(SplineChar *sc) {
    SplineSet *head=NULL, *last, *ss, *tss;
    RefChar *ref;

    for ( ss=sc->layers[ly_fore].splines; ss!=NULL; ss=ss->next ) {
	tss = sc->parent->order2 ? SplinePointListCopy1(ss) : SSttfApprox(ss);
	if ( head==NULL ) head = tss;
	else last->next = tss;
	last = tss;
    }
    for ( ref=sc->layers[ly_fore].refs; ref!=NULL; ref=ref->next ) {
	for ( ss=ref->layers[0].splines; ss!=NULL; ss=ss->next ) {
	    tss = sc->parent->order2 ? SplinePointListCopy1(ss) : SSttfApprox(ss);
	    if ( head==NULL ) head = tss;
	    else last->next = tss;
	    last = tss;
	}
    }
return( head );
}

#define _On_Curve	1
#define _X_Short	2
#define _Y_Short	4
#define _Repeat		8
#define _X_Same		0x10
#define _Y_Same		0x20

int SSAddPoints(SplineSet *ss,int ptcnt,BasePoint *bp, char *flags) {
    SplinePoint *sp, *first, *nextsp;
    int startcnt = ptcnt;

    if ( ss->first->prev!=NULL &&
	    ss->first->prev->from->nextcpindex==startcnt ) {
	if ( flags!=NULL ) flags[ptcnt] = 0;
	bp[ptcnt].x = rint(ss->first->prevcp.x);
	bp[ptcnt++].y = rint(ss->first->prevcp.y);
    } else if ( ss->first->ttfindex!=ptcnt && ss->first->ttfindex!=0xfffe )
	IError("Unexpected point count in SSAddPoints" );

    first = NULL;
    for ( sp=ss->first; sp!=first ; ) {
	if ( sp->ttfindex!=0xffff ) {
	    if ( flags!=NULL ) flags[ptcnt] = _On_Curve;
	    bp[ptcnt].x = rint(sp->me.x);
	    bp[ptcnt].y = rint(sp->me.y);
	    sp->ttfindex = ptcnt++;
	} else if ( !SPInterpolate( sp ) ) {
	    /* If an on curve point is midway between two off curve points*/
	    /*  it may be omitted and will be interpolated on read in */
	    if ( flags!=NULL ) flags[ptcnt] = _On_Curve;
	    bp[ptcnt].x = rint(sp->me.x);
	    bp[ptcnt].y = rint(sp->me.y);
	    sp->ttfindex = ptcnt++;
	}
	nextsp = sp->next!=NULL ? sp->next->to : NULL;
	if ( sp->nextcpindex == startcnt )
	    /* This control point is actually our first point, not our last */
    break;
	if ( (sp->nextcpindex !=0xffff && sp->nextcpindex!=0xfffe ) ||
		!sp->nonextcp ) {
	    if ( flags!=NULL ) flags[ptcnt] = 0;
	    bp[ptcnt].x = rint(sp->nextcp.x);
	    bp[ptcnt++].y = rint(sp->nextcp.y);
	}
	if ( nextsp==NULL )
    break;
	if ( first==NULL ) first = sp;
	sp = nextsp;
    }
return( ptcnt );
}

static void dumppointarrays(struct glyphinfo *gi,BasePoint *bp, char *fs, int pc) {
    BasePoint last;
    int i,flags;
    int lastflag, flagcnt;

    if ( gi->maxp->maxPoints<pc )
	gi->maxp->maxPoints = pc;

	/* flags */
    last.x = last.y = 0;
    lastflag = -1; flagcnt = 0;
    for ( i=0; i<pc; ++i ) {
	flags = 0;
	if ( fs==NULL || fs[i] )
	    flags = _On_Curve;		/* points are on curve */
	if ( last.x==bp[i].x )
	    flags |= _X_Same;
	else if ( bp[i].x-last.x>-256 && bp[i].x-last.x<255 ) {
	    flags |= _X_Short;
	    if ( bp[i].x>=last.x )
		flags |= _X_Same;		/* In this context it means positive */
	}
	if ( last.y==bp[i].y )
	    flags |= _Y_Same;
	else if ( bp[i].y-last.y>-256 && bp[i].y-last.y<255 ) {
	    flags |= _Y_Short;
	    if ( bp[i].y>=last.y )
		flags |= _Y_Same;		/* In this context it means positive */	
	}
	last = bp[i];
	if ( lastflag==-1 ) {
	    lastflag = flags;
	    flagcnt = 0;
	} else if ( flags!=lastflag ) {
	    if ( flagcnt!=0 )
		lastflag |= _Repeat;
	    putc(lastflag,gi->glyphs);
	    if ( flagcnt!=0 )
		putc(flagcnt,gi->glyphs);
	    lastflag = flags;
	    flagcnt = 0;
	} else {
	    if ( ++flagcnt == 255 ) {
		putc(lastflag|_Repeat,gi->glyphs);
		putc(255,gi->glyphs);
		lastflag = -1;
		flagcnt = 0;
	    }
	}
    }
    if ( lastflag!=-1 ) {
	if ( flagcnt!=0 )
	    lastflag |= _Repeat;
	putc(lastflag,gi->glyphs);
	if ( flagcnt!=0 )
	    putc(flagcnt,gi->glyphs);
    }

	/* xcoords */
    last.x = 0;
    for ( i=0; i<pc; ++i ) {
	if ( last.x==bp[i].x )
	    /* Do Nothing */;
	else if ( bp[i].x-last.x>-256 && bp[i].x-last.x<255 ) {
	    if ( bp[i].x>=last.x )
		putc(bp[i].x-last.x,gi->glyphs);
	    else
		putc(last.x-bp[i].x,gi->glyphs);
	} else
	    putshort(gi->glyphs,bp[i].x-last.x);
	last.x = bp[i].x;
    }
	/* ycoords */
    last.y = 0;
    for ( i=0; i<pc; ++i ) {
	if ( last.y==bp[i].y )
	    /* Do Nothing */;
	else if ( bp[i].y-last.y>-256 && bp[i].y-last.y<255 ) {
	    if ( bp[i].y>=last.y )
		putc(bp[i].y-last.y,gi->glyphs);
	    else
		putc(last.y-bp[i].y,gi->glyphs);
	} else
	    putshort(gi->glyphs,bp[i].y-last.y);
	last.y = bp[i].y;
    }
    if ( ftell(gi->glyphs)&1 )		/* Pad the file so that the next glyph */
	putc('\0',gi->glyphs);		/* on a word boundary */
}

static void dumpinstrs(struct glyphinfo *gi,uint8 *instrs,int cnt) {
    int i;

    if ( (gi->flags&ttf_flag_nohints) ) {
	putshort(gi->glyphs,0);
return;
    }
    /* Do we ever want to call AutoHint and AutoInst here? I think not. */

    if ( gi->maxp->maxglyphInstr<cnt ) gi->maxp->maxglyphInstr=cnt;
    putshort(gi->glyphs,cnt);
    for ( i=0; i<cnt; ++i )
	putc( instrs[i],gi->glyphs );
}

static void dumpmissingglyph(SplineFont *sf,struct glyphinfo *gi,int fixedwidth) {
    struct glyphhead gh;
    BasePoint bp[10];
    uint8 instrs[50];
    int stemcvt, stem;

    gi->pointcounts[gi->next_glyph] = 8;
    gi->loca[gi->next_glyph++] = ftell(gi->glyphs);
    gi->maxp->maxContours = 2;

    gh.numContours = 2;
    gh.ymin = 0;
    gh.ymax = 2*(sf->ascent+sf->descent)/3;
    gh.xmax = (sf->ascent+sf->descent)/3;
    gh.xmin = stem = gh.xmax/10;
    gh.xmax += stem;
    if ( gh.ymax>sf->ascent ) gh.ymax = sf->ascent;
    dumpghstruct(gi,&gh);

    stemcvt = TTF_getcvtval(gi->sf,stem);

    bp[0].x = stem;		bp[0].y = 0;
    bp[1].x = stem;		bp[1].y = gh.ymax;
    bp[2].x = gh.xmax;		bp[2].y = gh.ymax;
    bp[3].x = gh.xmax;		bp[3].y = 0;

    bp[4].x = 2*stem;		bp[4].y = stem;
    bp[5].x = gh.xmax-stem;	bp[5].y = stem;
    bp[6].x = gh.xmax-stem;	bp[6].y = gh.ymax-stem;
    bp[7].x = 2*stem;		bp[7].y = gh.ymax-stem;

    instrs[0] = 0xb1;			/* Pushb, 2byte */
    instrs[1] = 1;			/* Point 1 */
    instrs[2] = 0;			/* Point 0 */
    instrs[3] = 0x2f;			/* MDAP, rounded (pt0) */
    instrs[4] = 0x3c;			/* ALIGNRP, (pt1 same pos as pt0)*/
    instrs[5] = 0xb2;			/* Pushb, 3byte */
    instrs[6] = 7;			/* Point 7 */
    instrs[7] = 4;			/* Point 4 */
    instrs[8] = stemcvt;		/* CVT entry for our stem width */
    instrs[9] = 0xe0+0x0d;		/* MIRP, don't set rp0, minimum, rounded, black */
    instrs[10] = 0x32;			/* SHP[rp2] (pt7 same pos as pt4) */
    instrs[11] = 0xb1;			/* Pushb, 2byte */
    instrs[12] = 6;			/* Point 6 */
    instrs[13] = 5;			/* Point 5 */
    instrs[14] = 0xc0+0x1c;		/* MDRP, set rp0, minimum, rounded, grey */
    instrs[15] = 0x3c;			/* ALIGNRP, (pt6 same pos as pt5)*/
    instrs[16] = 0xb2;			/* Pushb, 3byte */
    instrs[17] = 3;			/* Point 3 */
    instrs[18] = 2;			/* Point 2 */
    instrs[19] = stemcvt;		/* CVT entry for our stem width */
    instrs[20] = 0xe0+0x0d;		/* MIRP, dont set rp0, minimum, rounded, black */
    instrs[21] = 0x32;			/* SHP[rp2] (pt3 same pos as pt2) */

    instrs[22] = 0x00;			/* SVTCA, y axis */

    instrs[23] = 0xb1;			/* Pushb, 2byte */
    instrs[24] = 3;			/* Point 3 */
    instrs[25] = 0;			/* Point 0 */
    instrs[26] = 0x2f;			/* MDAP, rounded */
    instrs[27] = 0x3c;			/* ALIGNRP, (pt3 same height as pt0)*/
    instrs[28] = 0xb2;			/* Pushb, 3byte */
    instrs[29] = 5;			/* Point 5 */
    instrs[30] = 4;			/* Point 4 */
    instrs[31] = stemcvt;		/* CVT entry for our stem width */
    instrs[32] = 0xe0+0x0d;		/* MIRP, don't set rp0, minimum, rounded, black */
    instrs[33] = 0x32;			/* SHP[rp2] (pt5 same height as pt4) */
    instrs[34] = 0xb2;			/* Pushb, 3byte */
    instrs[35] = 7;			/* Point 7 */
    instrs[36] = 6;			/* Point 6 */
    instrs[37] = TTF_getcvtval(gi->sf,bp[6].y);	/* CVT entry for top height */
    instrs[38] = 0xe0+0x1c;		/* MIRP, set rp0, minimum, rounded, grey */
    instrs[39] = 0x3c;			/* ALIGNRP (pt7 same height as pt6) */
    instrs[40] = 0xb2;			/* Pushb, 3byte */
    instrs[41] = 1;			/* Point 1 */
    instrs[42] = 2;			/* Point 2 */
    instrs[43] = stemcvt;		/* CVT entry for our stem width */
    instrs[44] = 0xe0+0x0d;		/* MIRP, dont set rp0, minimum, rounded, black */
    instrs[45] = 0x32;			/* SHP[rp2] (pt1 same height as pt2) */

	/* We've touched all points in all dimensions */
	/* Don't need any IUP */

	/* end contours array */
    putshort(gi->glyphs,4-1);
    putshort(gi->glyphs,8-1);
	/* instruction length&instructions */
    dumpinstrs(gi,instrs,46);

    dumppointarrays(gi,bp,NULL,8);

    if ( fixedwidth==-1 )
	putshort(gi->hmtx,gh.xmax + 2*stem);
    else
	putshort(gi->hmtx,fixedwidth);
    putshort(gi->hmtx,stem);
    if ( sf->hasvmetrics ) {
	putshort(gi->vmtx,sf->ascent+sf->descent);
	putshort(gi->vmtx,sf->vertical_origin-gh.ymax);
    }
}

static void dumpblankglyph(struct glyphinfo *gi,SplineFont *sf,int fixedwidth) {
    int advance = gi->next_glyph==1?0:fixedwidth==-1?(sf->ascent+sf->descent)/3:
	    fixedwidth;
    /* For reasons quite obscure to me, glyph 1 has an advance width of 0 */
    /* even in a mono-spaced font like CourierNew.ttf */

    /* These don't get a glyph header, because there are no contours */
    gi->pointcounts[gi->next_glyph] = 0;
    gi->loca[gi->next_glyph++] = ftell(gi->glyphs);
    putshort(gi->hmtx,advance);
    putshort(gi->hmtx,0);
    if ( sf->hasvmetrics ) {
	putshort(gi->vmtx,gi->next_glyph==2?0:(sf->ascent+sf->descent));
	putshort(gi->vmtx,0);
    }
}

static void dumpspace(SplineChar *sc, struct glyphinfo *gi) {
    /* These don't get a glyph header, because there are no contours */
    DBounds b;
    gi->pointcounts[gi->next_glyph] = 0;
    gi->loca[gi->next_glyph++] = ftell(gi->glyphs);
    memset(&b,0,sizeof(b));
    ttfdumpmetrics(sc,gi,&b);
}

static int IsTTFRefable(SplineChar *sc) {
    RefChar *ref;

    if ( sc->layers[ly_fore].refs==NULL || sc->layers[ly_fore].splines!=NULL )
return( false );

    for ( ref=sc->layers[ly_fore].refs; ref!=NULL; ref=ref->next ) {
	if ( ref->transform[0]<-2 || ref->transform[0]>1.999939 ||
		ref->transform[1]<-2 || ref->transform[1]>1.999939 ||
		ref->transform[2]<-2 || ref->transform[2]>1.999939 ||
		ref->transform[3]<-2 || ref->transform[3]>1.999939 )
return( false );
    }
return( true );
}

static int RefDepth(RefChar *ref) {
    int rd, temp;
    SplineChar *sc = ref->sc;

    if ( sc->layers[ly_fore].refs==NULL || sc->layers[ly_fore].splines!=NULL )
return( 1 );
    rd = 0;
    for ( ref = sc->layers[ly_fore].refs; ref!=NULL; ref=ref->next ) {
	if ( ref->transform[0]>=-2 || ref->transform[0]<=1.999939 ||
		ref->transform[1]>=-2 || ref->transform[1]<=1.999939 ||
		ref->transform[2]>=-2 || ref->transform[2]<=1.999939 ||
		ref->transform[3]>=-2 || ref->transform[3]<=1.999939 ) {
	    temp = RefDepth(ref);
	    if ( temp>rd ) rd = temp;
	}
    }
return( rd+1 );
}

static void CountCompositeMaxPts(SplineChar *sc,struct glyphinfo *gi) {
    RefChar *ref;
    int ptcnt = 0, index;

    for ( ref=sc->layers[ly_fore].refs; ref!=NULL; ref=ref->next ) {
	if ( ref->sc->ttf_glyph==-1 )
    continue;
	index = ref->sc->ttf_glyph;
	if ( gi->pointcounts[index]==-1 )
	    CountCompositeMaxPts(ref->sc,gi);
	ptcnt += gi->pointcounts[index];
    }
    gi->pointcounts[sc->ttf_glyph] = ptcnt;
    if ( gi->maxp->maxCompositPts<ptcnt ) gi->maxp->maxCompositPts=ptcnt;
}

/* In order3 fonts we figure out the composite point counts at the end */
/*  when we know how many points are in each sub-glyph */
static void RefigureCompositeMaxPts(SplineFont *sf,struct glyphinfo *gi) {
    int i;

    for ( i=0; i<gi->gcnt; ++i ) if ( gi->bygid[i]!=-1 && sf->glyphs[gi->bygid[i]]->ttf_glyph!=-1 ) {
	if ( sf->glyphs[gi->bygid[i]]->layers[ly_fore].splines==NULL &&
		sf->glyphs[gi->bygid[i]]->layers[ly_fore].refs!=NULL &&
		gi->pointcounts[i]== -1 )
	    CountCompositeMaxPts(sf->glyphs[gi->bygid[i]],gi);
    }
}

static void dumpcomposite(SplineChar *sc, struct glyphinfo *gi) {
    struct glyphhead gh;
    DBounds bb;
    int i, ptcnt, ctcnt, flags, sptcnt, rd;
    SplineSet *ss;
    RefChar *ref;
    SplineChar *isc = sc->ttf_instrs==NULL && sc->parent->mm!=NULL && sc->parent->mm->apple ?
		sc->parent->mm->normal->glyphs[sc->orig_pos] : sc;
    int arg1, arg2;

#if 0
    if ( autohint_before_generate && sc->changedsincelasthinted &&
	    !sc->manualhints )
	if ( !(gi->flags&ttf_flag_nohints) )
	    SplineCharAutoHint(sc,NULL);
#endif

    gi->loca[gi->next_glyph] = ftell(gi->glyphs);

    SplineCharFindBounds(sc,&bb);
    gh.numContours = -1;
    gh.xmin = floor(bb.minx); gh.ymin = floor(bb.miny);
    gh.xmax = ceil(bb.maxx); gh.ymax = ceil(bb.maxy);
    dumpghstruct(gi,&gh);

    i=ptcnt=ctcnt=0;
    for ( ref=sc->layers[ly_fore].refs; ref!=NULL; ref=ref->next, ++i ) {
	if ( ref->sc->ttf_glyph==-1 ) {
	    /*if ( sc->layers[ly_fore].refs->next==NULL || any )*/
    continue;
	}
	flags = 0;
	if ( ref->round_translation_to_grid )
	    flags |= _ROUND;
	if ( ref->use_my_metrics )
	    flags |= _USE_MY_METRICS;
	if ( ref->next!=NULL )
	    flags |= _MORE;		/* More components */
	else if ( isc->ttf_instrs_len!=0 )	/* Composits also inherit instructions */
	    flags |= _INSTR;		/* Instructions appear after last ref */
	if ( ref->transform[1]!=0 || ref->transform[2]!=0 )
	    flags |= _MATRIX;		/* Need a full matrix */
	else if ( ref->transform[0]!=ref->transform[3] )
	    flags |= _XY_SCALE;		/* different xy scales */
	else if ( ref->transform[0]!=1. )
	    flags |= _SCALE;		/* xy scale is same */
	if ( ref->point_match ) {
	    arg1 = ref->match_pt_base;
	    arg2 = ref->match_pt_ref;
	} else {
	    arg1 = rint(ref->transform[4]);
	    arg2 = rint(ref->transform[5]);
	    flags |= _ARGS_ARE_XY|_UNSCALED_OFFSETS;
	    /* The values I output are the values I want to see */
	    /* There is some very strange stuff wrongly-documented on the apple*/
	    /*  site about how these should be interpretted when there are */
	    /*  scale factors, or rotations */
	    /* That description does not match the behavior of their rasterizer*/
	    /*  I've reverse engineered something else (see parsettf.c) */
	    /*  http://fonts.apple.com/TTRefMan/RM06/Chap6glyf.html */
	    /* Adobe says that setting bit 12 means that this will not happen */
	    /*  Apple doesn't mention bit 12 though...(but they do support it) */
	}
	if ( arg1<-128 || arg1>127 ||
		arg2<-128 || arg2>127 )
	    flags |= _ARGS_ARE_WORDS;
	putshort(gi->glyphs,flags);
	putshort(gi->glyphs,ref->sc->ttf_glyph==-1?0:ref->sc->ttf_glyph);
	if ( flags&_ARGS_ARE_WORDS ) {
	    putshort(gi->glyphs,(short)arg1);
	    putshort(gi->glyphs,(short)arg2);
	} else {
	    putc((char) arg1,gi->glyphs);
	    putc((char) arg2,gi->glyphs);
	}
	if ( flags&_MATRIX ) {
	    put2d14(gi->glyphs,ref->transform[0]);
	    put2d14(gi->glyphs,ref->transform[1]);
	    put2d14(gi->glyphs,ref->transform[2]);
	    put2d14(gi->glyphs,ref->transform[3]);
	} else if ( flags&_XY_SCALE ) {
	    put2d14(gi->glyphs,ref->transform[0]);
	    put2d14(gi->glyphs,ref->transform[3]);
	} else if ( flags&_SCALE ) {
	    put2d14(gi->glyphs,ref->transform[0]);
	}
	sptcnt = SSTtfNumberPoints(ref->layers[0].splines);
	for ( ss=ref->layers[0].splines; ss!=NULL ; ss=ss->next ) {
	    ++ctcnt;
	}
	if ( sc->parent->order2 )
	    ptcnt += sptcnt;
	else if ( ptcnt>=0 && gi->pointcounts[ref->sc->ttf_glyph==-1?0:ref->sc->ttf_glyph]>=0 )
	    ptcnt += gi->pointcounts[ref->sc->ttf_glyph==-1?0:ref->sc->ttf_glyph];
	else
	    ptcnt = -1;
	rd = RefDepth(ref);
	if ( rd>gi->maxp->maxcomponentdepth )
	    gi->maxp->maxcomponentdepth = rd;
    }

    if ( isc->ttf_instrs_len!=0 )
	dumpinstrs(gi,isc->ttf_instrs,isc->ttf_instrs_len);

    gi->pointcounts[gi->next_glyph++] = ptcnt;
    if ( gi->maxp->maxnumcomponents<i ) gi->maxp->maxnumcomponents = i;
    if ( gi->maxp->maxCompositPts<ptcnt ) gi->maxp->maxCompositPts=ptcnt;
    if ( gi->maxp->maxCompositCtrs<ctcnt ) gi->maxp->maxCompositCtrs=ctcnt;

    ttfdumpmetrics(sc,gi,&bb);
    if ( ftell(gi->glyphs)&1 )		/* Pad the file so that the next glyph */
	putc('\0',gi->glyphs);		/* on a word boundary, can only happen if odd number of instrs */
}

static void dumpglyph(SplineChar *sc, struct glyphinfo *gi) {
    struct glyphhead gh;
    DBounds bb;
    SplineSet *ss, *ttfss;
    int contourcnt, ptcnt, origptcnt;
    BasePoint *bp;
    char *fs;
    SplineChar *isc = sc->ttf_instrs==NULL && sc->parent->mm!=NULL && sc->parent->mm->apple ?
		sc->parent->mm->normal->glyphs[sc->orig_pos] : sc;

/* I haven't seen this documented, but ttf rasterizers are unhappy with a */
/*  glyph that consists of a single point. Glyphs containing two single points*/
/*  are ok, glyphs with a single point and anything else are ok, glyphs with */
/*  a line are ok. But a single point is not ok. Dunno why */
    if (( sc->layers[ly_fore].splines==NULL && sc->layers[ly_fore].refs==NULL ) ||
	    ( sc->layers[ly_fore].refs==NULL &&
	     (sc->layers[ly_fore].splines->first->next==NULL ||
	      sc->layers[ly_fore].splines->first->next->to == sc->layers[ly_fore].splines->first) &&
	     sc->layers[ly_fore].splines->next==NULL) ) {
	dumpspace(sc,gi);
return;
    }

    gi->loca[gi->next_glyph] = ftell(gi->glyphs);

    if ( isc->ttf_instrs!=NULL )
	initforinstrs(sc);

    ttfss = SCttfApprox(sc);
    ptcnt = SSTtfNumberPoints(ttfss);
    for ( ss=ttfss, contourcnt=0; ss!=NULL; ss=ss->next ) {
	++contourcnt;
    }
    origptcnt = ptcnt;

    SplineCharFindBounds(sc,&bb);
	/* MicroSoft's font validator has a bug. It only looks at the points */
	/*  when calculating the bounding box, and complains when I look at */
	/*  the splines for internal extrema. I presume it does this because */
	/*  all the extrema are supposed to be points (and it complains about */
	/*  that error too), but it would lead to rasterization problems if */
	/*  we did what they want */
    gh.numContours = contourcnt;
    gh.xmin = floor(bb.minx); gh.ymin = floor(bb.miny);
    gh.xmax = ceil(bb.maxx); gh.ymax = ceil(bb.maxy);
    dumpghstruct(gi,&gh);
    if ( contourcnt>gi->maxp->maxContours ) gi->maxp->maxContours = contourcnt;
    if ( ptcnt>gi->maxp->maxPoints ) gi->maxp->maxPoints = ptcnt;

    bp = galloc(ptcnt*sizeof(BasePoint));
    fs = galloc(ptcnt);
    ptcnt = contourcnt = 0;
    for ( ss=ttfss; ss!=NULL; ss=ss->next ) {
	ptcnt = SSAddPoints(ss,ptcnt,bp,fs);
	putshort(gi->glyphs,ptcnt-1);
    }
    if ( ptcnt!=origptcnt )
	IError( "Point count wrong calculated=%d, actual=%d in %.20s", origptcnt, ptcnt, sc->name );
    gi->pointcounts[gi->next_glyph++] = ptcnt;

    dumpinstrs(gi,isc->ttf_instrs,isc->ttf_instrs_len);
	
    dumppointarrays(gi,bp,fs,ptcnt);
    SplinePointListsFree(ttfss);
    free(bp);
    free(fs);

    ttfdumpmetrics(sc,gi,&bb);
}

void SFDummyUpCIDs(struct glyphinfo *gi,SplineFont *sf) {
    int i,j,k,max;
    int *bygid;

    max = 0;
    for ( k=0; k<sf->subfontcnt; ++k )
	if ( sf->subfonts[k]->glyphcnt>max ) max = sf->subfonts[k]->glyphcnt;
    if ( max == 0 )
return;

    sf->glyphs = gcalloc(max,sizeof(SplineChar *));
    sf->glyphcnt = sf->glyphmax = max;
    for ( k=0; k<sf->subfontcnt; ++k )
	for ( i=0; i<sf->subfonts[k]->glyphcnt; ++i ) if ( sf->subfonts[k]->glyphs[i]!=NULL )
	    sf->glyphs[i] = sf->subfonts[k]->glyphs[i];

    if ( gi==NULL )
return;

    bygid = galloc((sf->glyphcnt+3)*sizeof(int));
    memset(bygid,0xff, (sf->glyphcnt+3)*sizeof(int));

    j=1;
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( bygid[0]== -1 && strcmp(sf->glyphs[i]->name,".notdef")==0 ) {
	    sf->glyphs[i]->ttf_glyph = 0;
	    bygid[0] = i;
	} else if ( SCWorthOutputting(sf->glyphs[i])) {
	    sf->glyphs[i]->ttf_glyph = j;
	    bygid[j++] = i;
	}
    }
    gi->bygid = bygid;
    gi->gcnt = j;
}

static void AssignNotdefNull(SplineFont *sf,int *bygid, int iscff) {
    int i;

    /* The first three glyphs are magic, glyph 0 is .notdef */
    /*  glyph 1 is .null and glyph 2 is nonmarking return */
    /*  We may generate them automagically */

    bygid[0] = bygid[1] = bygid[2] = -1;
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( bygid[0]== -1 && strcmp(sf->glyphs[i]->name,".notdef")==0 ) {
	    sf->glyphs[i]->ttf_glyph = 0;
	    bygid[0] = i;
	} else if ( !iscff && bygid[1]== -1 &&
		(strcmp(sf->glyphs[i]->name,".null")==0 ||
		 (i==1 && strcmp(sf->glyphs[1]->name,"glyph1")==0)) ) {
	    sf->glyphs[i]->ttf_glyph = 1;
	    bygid[1] = i;
	} else if ( !iscff && bygid[2]== -1 &&
		(strcmp(sf->glyphs[i]->name,"nonmarkingreturn")==0 ||
		 (i==2 && strcmp(sf->glyphs[2]->name,"glyph2")==0)) ) {
	    sf->glyphs[i]->ttf_glyph = 2;
	    bygid[2] = i;
	}
    }
}

static int AssignTTFGlyph(struct glyphinfo *gi,SplineFont *sf,EncMap *map,int iscff) {
    int *bygid = galloc((sf->glyphcnt+3)*sizeof(int));
    int i,j;

    memset(bygid,0xff, (sf->glyphcnt+3)*sizeof(int));

    AssignNotdefNull(sf,bygid,iscff);

    j = iscff ? 1 : 3;
    for ( i=0; i<map->enccount; ++i ) if ( map->map[i]!=-1 ) {
	SplineChar *sc = sf->glyphs[map->map[i]];
	if ( SCWorthOutputting(sc) && sc->ttf_glyph==-1
#if HANYANG
		&& (!iscff || !sc->compositionunit)
#endif
	) {
	    sc->ttf_glyph = j;
	    bygid[j++] = sc->orig_pos;
	}
    }

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	SplineChar *sc = sf->glyphs[i];
	if ( SCWorthOutputting(sc) && sc->ttf_glyph==-1 
#if HANYANG
		&& (!iscff || !sc->compositionunit)
#endif
	) {
	    sc->ttf_glyph = j;
	    bygid[j++] = i;
	}
    }
    gi->bygid = bygid;
    gi->gcnt = j;
return j;
}

static int AssignTTFBitGlyph(struct glyphinfo *gi,SplineFont *sf,EncMap *map,int32 *bsizes) {
    int i, j;
    BDFFont *bdf;
    int *bygid = galloc((sf->glyphcnt+3)*sizeof(int));

    memset(bygid,0xff, (sf->glyphcnt+3)*sizeof(int));

    AssignNotdefNull(sf,bygid,false);

    for ( bdf = sf->bitmaps; bdf!=NULL; bdf=bdf->next ) {
	for ( j=0; bsizes[j]!=0 && ((bsizes[j]&0xffff)!=bdf->pixelsize || (bsizes[j]>>16)!=BDFDepth(bdf)); ++j );
	if ( bsizes[j]==0 )
    continue;
	for ( i=0; i<bdf->glyphcnt; ++i ) if ( !IsntBDFChar(bdf->glyphs[i]) )
	    sf->glyphs[i]->ttf_glyph = -2;
    }

    j = 3;
    for ( i=0; i<map->enccount; ++i ) if ( map->map[i]!=-1 ) {
	SplineChar *sc = sf->glyphs[map->map[i]];
	if ( sc->ttf_glyph==-2 ) {
	    sc->ttf_glyph = j;
	    bygid[j++] = sc->orig_pos;
	}
    }

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	SplineChar *sc = sf->glyphs[i];
	if ( sc->ttf_glyph==-2 ) {
	    sc->ttf_glyph = j;
	    bygid[j++] = i;
	}
    }

    gi->bygid = bygid;
    gi->gcnt = j;
return j;
}

static int dumpglyphs(SplineFont *sf,struct glyphinfo *gi) {
    int i, cnt;
    int fixed = gi->fixed_width;

#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
    gwwv_progress_change_stages(2+gi->strikecnt);
#endif
    QuickBlues(sf,&gi->bd);
    /*FindBlues(sf,gi->blues,NULL);*/
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
    gwwv_progress_next_stage();
#endif

    if ( !gi->onlybitmaps ) {
	if ( sf->order2 )
	    for ( i=0; i<sf->glyphcnt; ++i ) {
		SplineChar *sc = sf->glyphs[i];
		if ( SCWorthOutputting(sc) )
		    if ( !SCPointsNumberedProperly(sc)) {
			free(sc->ttf_instrs); sc->ttf_instrs = NULL;
			sc->ttf_instrs_len = 0;
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
			SCMarkInstrDlgAsChanged(sc);
#endif		/* FONTFORGE_CONFIG_NO_WINDOWING_UI */
			SCNumberPoints(sc);
		    }
	    }
    }

    gi->maxp->numGlyphs = cnt = gi->gcnt;
    gi->loca = galloc((gi->maxp->numGlyphs+1)*sizeof(uint32));
    gi->pointcounts = galloc((gi->maxp->numGlyphs+1)*sizeof(int32));
    memset(gi->pointcounts,-1,(gi->maxp->numGlyphs+1)*sizeof(int32));
    gi->next_glyph = 0;
    gi->glyphs = tmpfile();
    gi->hmtx = tmpfile();
    if ( sf->hasvmetrics )
	gi->vmtx = tmpfile();
    FigureFullMetricsEnd(sf,gi);

    if ( fixed!=-1 ) {
	gi->lasthwidth = 3;
	gi->hfullcnt = 3;
    }
    for ( i=0; i<gi->gcnt; ++i ) {
	if ( i==0 ) {
	    if ( gi->bygid[0]!=-1 && (fixed==-1 || sf->glyphs[gi->bygid[0]]->width==fixed))
		dumpglyph(sf->glyphs[gi->bygid[0]],gi);
	    else
		dumpmissingglyph(sf,gi,fixed);
	} else if ( i<=2 && gi->bygid[i]==-1 )
	    dumpblankglyph(gi,sf,fixed);
	else if ( gi->onlybitmaps ) {
	    if ( gi->bygid[i]!=-1 && sf->glyphs[gi->bygid[i]]->ttf_glyph>0 )
		dumpspace(sf->glyphs[gi->bygid[i]],gi);
	} else {
	    if ( gi->bygid[i]!=-1 && sf->glyphs[gi->bygid[i]]->ttf_glyph>0 ) {
		if ( IsTTFRefable(sf->glyphs[gi->bygid[i]]) )
		    dumpcomposite(sf->glyphs[gi->bygid[i]],gi);
		else
		    dumpglyph(sf->glyphs[gi->bygid[i]],gi);
	    }
	}
	if ( (ftell(gi->glyphs)&3) != 0 ) {
	    /* Apple says glyphs must be 16bit aligned */
	    if ( ftell(gi->glyphs)&1 )
		putc('\0',gi->glyphs);
	    /* MS says glyphs should be 32bit aligned */
	    if ( ftell(gi->glyphs)&2 )
		putshort(gi->glyphs,0);
	}
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
	if ( !gwwv_progress_next())
return( false );
#endif		/* FONTFORGE_CONFIG_NO_WINDOWING_UI */
    }

    /* extra location entry points to end of last glyph */
    gi->loca[gi->next_glyph] = ftell(gi->glyphs);
    /* Microsoft's Font Validator wants the last loca entry to point into the */
    /*  glyph table. I think that's an error on their part, but it's so easy */
    /*  to fix, I might as well (instead of pointing to right after the table)*/
    /* Sigh. But if I do that, it complains that there's extra stuff in the */
    /*  glyph table. There's just no pleasing them */
    /* putlong(gi->glyphs,0);*/
    gi->glyph_len = ftell(gi->glyphs);
    gi->hmtxlen = ftell(gi->hmtx);
    /* pad out to four bytes */
    if ( gi->hmtxlen&2 ) putshort(gi->hmtx,0);
    if ( gi->loca[gi->next_glyph]&3 ) {
	for ( i=4-(gi->loca[gi->next_glyph]&3); i>0; --i )
	    putc('\0',gi->glyphs);
    }
    if ( sf->hasvmetrics ) {
	gi->vmtxlen = ftell(gi->vmtx);
	if ( gi->vmtxlen&2 ) putshort(gi->vmtx,0);
    }
    if ( !sf->order2 )
	RefigureCompositeMaxPts(sf,gi);
    free(gi->pointcounts);

return( true );
}

/* Generate a null glyf and loca table for X opentype bitmaps */
static int dumpnoglyphs(SplineFont *sf,struct glyphinfo *gi) {

    gi->glyphs = tmpfile();
    gi->glyph_len = 0;
    /* loca gets built in dummyloca */
return( true );
}

/* Standard names for cff */
extern const char *cffnames[];
extern const int nStdStrings;

static int storesid(struct alltabs *at,char *str) {
    int i;
    FILE *news;
    char *pt;
    long pos;

    if ( str!=NULL ) {			/* NULL is the magic string at end of array */
	for ( i=0; cffnames[i]!=NULL; ++i ) {
	    if ( strcmp(cffnames[i],str)==0 )
return( i );
	}
    }

    pos = ftell(at->sidf)+1;
    if ( pos>=65536 && !at->sidlongoffset ) {
	at->sidlongoffset = true;
	news = tmpfile();
	rewind(at->sidh);
	for ( i=0; i<at->sidcnt; ++i )
	    putlong(news,getushort(at->sidh));
	fclose(at->sidh);
	at->sidh = news;
    }
    if ( at->sidlongoffset )
	putlong(at->sidh,pos);
    else
	putshort(at->sidh,pos);

    if ( str!=NULL ) {
	for ( pt=str; *pt; ++pt )
	    putc(*pt,at->sidf);
    }
return( at->sidcnt++ + nStdStrings );
}

static void dumpint(FILE *cfff,int num) {

    if ( num>=-107 && num<=107 )
	putc(num+139,cfff);
    else if ( num>=108 && num<=1131 ) {
	num -= 108;
	putc((num>>8)+247,cfff);
	putc(num&0xff,cfff);
    } else if ( num>=-1131 && num<=-108 ) {
	num = -num;
	num -= 108;
	putc((num>>8)+251,cfff);
	putc(num&0xff,cfff);
    } else if ( num>=-32768 && num<32768 ) {
	putc(28,cfff);
	putc(num>>8,cfff);
	putc(num&0xff,cfff);
    } else {		/* In dict data we have 4 byte ints, in type2 strings we don't */
	putc(29,cfff);
	putc((num>>24)&0xff,cfff);
	putc((num>>16)&0xff,cfff);
	putc((num>>8)&0xff,cfff);
	putc(num&0xff,cfff);
    }
}

static void dumpdbl(FILE *cfff,double d) {
    if ( d-rint(d)>-.00001 && d-rint(d)<.00001 )
	dumpint(cfff,(int) d);
    else {
	/* The type2 strings have a fixed format, but the dict data does not */
	char buffer[20], *pt;
	int sofar,n,odd;
	sprintf( buffer, "%g", d);
	sofar = 0; odd=true;
	putc(30,cfff);		/* Start a double */
	for ( pt=buffer; *pt; ++pt ) {
	    if ( isdigit(*pt) )
		n = *pt-'0';
	    else if ( *pt=='.' )
		n = 0xa;
	    else if ( *pt=='-' )
		n = 0xe;
	    else if (( *pt=='E' || *pt=='e') && pt[1]=='-' ) {
		n = 0xc;
		++pt;
	    } else if ( *pt=='E' || *pt=='e')
		n = 0xb;
	    else
		n = 0;		/* Should never happen */
	    if ( odd ) {
		sofar = n<<4;
		odd = false;
	    } else {
		putc(sofar|n,cfff);
		sofar=0;
		odd = true;
	    }
	}
	if ( sofar==0 )
	    putc(0xff,cfff);
	else
	    putc(sofar|0xf,cfff);
    }
}

static void dumpoper(FILE *cfff,int oper ) {
    if ( oper!=-1 ) {
	if ( oper>=256 )
	    putc(oper>>8,cfff);
	putc(oper&0xff,cfff);
    }
}

static void dumpdbloper(FILE *cfff,double d, int oper ) {
    dumpdbl(cfff,d);
    dumpoper(cfff,oper);
}

static void dumpintoper(FILE *cfff,int v, int oper ) {
    dumpint(cfff,v);
    dumpoper(cfff,oper);
}

static void dumpsizedint(FILE *cfff,int big,int num, int oper ) {
    if ( big ) {
	putc(29,cfff);
	putc((num>>24)&0xff,cfff);
	putc((num>>16)&0xff,cfff);
	putc((num>>8)&0xff,cfff);
	putc(num&0xff,cfff);
    } else {
	putc(28,cfff);
	putc(num>>8,cfff);
	putc(num&0xff,cfff);
    }
    dumpoper(cfff,oper);
}

static void dumpsid(FILE *cfff,struct alltabs *at,char *str,int oper) {
    if ( str==NULL )
return;
    dumpint(cfff,storesid(at,str));
    dumpoper(cfff,oper);
}

static void DumpStrDouble(char *pt,FILE *cfff,int oper) {
    real d;
    if ( *pt=='[' ) ++pt;		/* For StdHW, StdVW */
    d = strtod(pt,NULL);
    dumpdbloper(cfff,d,oper);
}

static void DumpDblArray(real *arr,int n,FILE *cfff, int oper) {
    int mi,i;

    for ( mi=n-1; mi>=0 && arr[mi]==0; --mi );
    if ( mi<0 )
return;
    dumpdbl(cfff,arr[0]);
    for ( i=1; i<=mi; ++i )
	dumpdbl(cfff,arr[i]-arr[i-1]);
    dumpoper(cfff,oper);
}

static void DumpStrArray(char *pt,FILE *cfff,int oper) {
    real d, last=0;
    char *end;

    while ( *pt==' ' ) ++pt;
    if ( *pt=='\0' )
return;
    if ( *pt=='[' ) ++pt;
    while ( *pt==' ' ) ++pt;
    while ( *pt!=']' && *pt!='\0' ) {
	d = strtod(pt,&end);
	if ( pt==end )		/* User screwed up. Should be a number */
    break;
	dumpdbl(cfff,d-last);
	last = d;
	pt = end;
	while ( *pt==' ' ) ++pt;
    }
    dumpoper(cfff,oper);
}

static void dumpcffheader(SplineFont *sf,FILE *cfff) {
    putc('\1',cfff);		/* Major version: 1 */
    putc('\0',cfff);		/* Minor version: 0 */
    putc('\4',cfff);		/* Header size in bytes */
    putc('\4',cfff);		/* Absolute Offset size. */
	/* I don't think there are any absolute offsets that aren't encoded */
	/*  in a dict as numbers (ie. inherently variable sized items) */
}

static void dumpcffnames(SplineFont *sf,FILE *cfff) {
    char *pt;

    putshort(cfff,1);		/* One font name */
    putc('\1',cfff);		/* Offset size */
    putc('\1',cfff);		/* Offset to first name */
    putc('\1'+strlen(sf->fontname),cfff);
    for ( pt=sf->fontname; *pt; ++pt )
	putc(*pt,cfff);
}

static void dumpcffcharset(SplineFont *sf,struct alltabs *at) {
    int i;

    at->gn_sid = gcalloc(at->gi.gcnt,sizeof(uint32));
    putc(0,at->charset);
    /* I always use a format 0 charset. ie. an array of SIDs in random order */

    /* First element must be ".notdef" and is omitted */

    for ( i=1; i<at->gi.gcnt; ++i )
	if ( at->gi.bygid[i]!=-1 && SCWorthOutputting(sf->glyphs[at->gi.bygid[i]])) {
	    at->gn_sid[i] = storesid(at,sf->glyphs[at->gi.bygid[i]]->name);
	    putshort(at->charset,at->gn_sid[i]);
	}
}

static void dumpcffcidset(SplineFont *sf,struct alltabs *at) {
    int gid, start;

    putc(2,at->charset);

    start = -1;			/* Glyph 0 always maps to CID 0, and is omitted */
    for ( gid = 1; gid<at->gi.gcnt; ++gid ) {
	if ( start==-1 )
	    start = gid;
	else if ( at->gi.bygid[gid]-at->gi.bygid[start]!=gid-start ) {
	    putshort(at->charset,at->gi.bygid[start]);
	    putshort(at->charset,at->gi.bygid[gid-1]-at->gi.bygid[start]);
	    start = gid;
	}
    }
    if ( start!=-1 ) {
	putshort(at->charset,at->gi.bygid[start]);
	putshort(at->charset,at->gi.bygid[gid-1]-at->gi.bygid[start]);
    }
}

static void dumpcfffdselect(SplineFont *sf,struct alltabs *at) {
    int cid, k, lastfd, cnt;
    int gid;

    putc(3,at->fdselect);
    putshort(at->fdselect,0);		/* number of ranges, fill in later */

    for ( k=0; k<sf->subfontcnt; ++k )
	if ( SCWorthOutputting(sf->subfonts[k]->glyphs[0]))
    break;
    if ( k==sf->subfontcnt ) --k;	/* If CID 0 not defined, put it in last font */
    putshort(at->fdselect,0);
    putc(k,at->fdselect);
    lastfd = k;
    cnt = 1;
    for ( gid = 1; gid<at->gi.gcnt; ++gid ) {
	cid = at->gi.bygid[gid];
	for ( k=0; k<sf->subfontcnt; ++k ) {
	    if ( cid<sf->subfonts[k]->glyphcnt &&
		SCWorthOutputting(sf->subfonts[k]->glyphs[cid]) )
	break;
	}
	if ( k==sf->subfontcnt )
	    /* Doesn't map to a glyph, irrelevant */;
	else {
	    if ( k!=lastfd ) {
		putshort(at->fdselect,gid);
		putc(k,at->fdselect);
		lastfd = k;
		++cnt;
	    }
	}
    }
    putshort(at->fdselect,gid);
    fseek(at->fdselect,1,SEEK_SET);
    putshort(at->fdselect,cnt);
    fseek(at->fdselect,0,SEEK_END);
}

static void dumpcffencoding(SplineFont *sf,struct alltabs *at) {
    int i, cnt, anydups;
    uint32 start_pos = ftell(at->encoding);
    SplineChar *sc;
    EncMap *map = at->map;

    putc(0,at->encoding);
    /* I always use a format 0 encoding. ie. an array of glyph indexes */
    putc(0xff,at->encoding);		/* fixup later */

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	sf->glyphs[i]->ticked = false;

    cnt = 0;
    anydups = 0;
    for ( i=0; i<256 && i<map->enccount; ++i ) if ( map->map[i]!=-1 && (sc=sf->glyphs[map->map[i]])!=NULL ) {
	if ( sc->ttf_glyph>255 )
    continue;
	if ( sc->ticked ) {
	    ++anydups;
	} else if ( sc->ttf_glyph>0 ) {
	    if ( cnt>=255 )
    break;
	    putc(i,at->encoding);
	    ++cnt;
	    sc->ticked = true;
	}
    }
    if ( anydups ) {
	fseek(at->encoding,start_pos,SEEK_SET);
	putc(0x80,at->encoding);
	putc(cnt,at->encoding);
	fseek(at->encoding,0,SEEK_END);
	putc(anydups,at->encoding);

	for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	    sf->glyphs[i]->ticked = false;
	for ( i=0; i<256 && i<map->enccount; ++i ) if ( map->map[i]!=-1 && (sc=sf->glyphs[map->map[i]])!=NULL ) {
	    if ( sc->ttf_glyph>255 )
    continue;
	    if ( sc->ticked ) {
		putc(i,at->encoding);
		putshort(at->encoding,at->gn_sid[sc->ttf_glyph]);
	    }
	    sc->ticked = true;
	}
    } else {
	fseek(at->encoding,start_pos+1,SEEK_SET);
	putc(cnt,at->encoding);
	fseek(at->encoding,0,SEEK_END);
    }
    free( at->gn_sid );
    at->gn_sid = NULL;
}

static void _dumpcffstrings(FILE *file, struct pschars *strs) {
    int i, len, offsize;

    /* First figure out the offset size */
    len = 1;
    for ( i=0; i<strs->next; ++i )
	len += strs->lens[i];

    /* Then output the index size and offsets */
    putshort( file, strs->next );
    if ( strs->next!=0 ) {
	/* presumably offsets are unsigned. But the docs don't state this in the obvious place */
	offsize = len<=255?1:len<=65535?2:len<=0xffffff?3:4;
	putc(offsize,file);
	len = 1;
	for ( i=0; i<strs->next; ++i ) {
	    dumpoffset(file,offsize,len);
	    len += strs->lens[i];
	}
	dumpoffset(file,offsize,len);

	/* last of all the strings */
	for ( i=0; i<strs->next; ++i ) {
	    uint8 *pt = strs->values[i], *end = pt+strs->lens[i];
	    while ( pt<end )
		putc( *pt++, file );
	}
    }
}

static FILE *dumpcffstrings(struct pschars *strs) {
    FILE *file = tmpfile();
    _dumpcffstrings(file,strs);
    PSCharsFree(strs);
return( file );
}

int SFFigureDefWidth(SplineFont *sf, int *_nomwid) {
    uint16 *widths; uint32 *cumwid;
    int nomwid, defwid, i, sameval=(int) 0x80000000, maxw=0, allsame=true;
    int cnt,j;

    for ( i=0; i<sf->glyphcnt; ++i )
	if ( SCWorthOutputting(sf->glyphs[i]) ) {
	    if ( maxw<sf->glyphs[i]->width ) maxw = sf->glyphs[i]->width;
	    if ( sameval == 0x8000000 )
		sameval = sf->glyphs[i]->width;
	    else if ( sameval!=sf->glyphs[i]->width )
		allsame = false;
	}
    if ( allsame ) {
	nomwid = defwid = sameval;
    } else {
	++maxw;
	if ( maxw>65535 ) maxw = 3*(sf->ascent+sf->descent);
	widths = gcalloc(maxw,sizeof(uint16));
	cumwid = gcalloc(maxw,sizeof(uint32));
	defwid = 0; cnt=0;
	for ( i=0; i<sf->glyphcnt; ++i )
	    if ( SCWorthOutputting(sf->glyphs[i]) &&
		    sf->glyphs[i]->width>=0 &&
		    sf->glyphs[i]->width<maxw )
		if ( ++widths[sf->glyphs[i]->width] > cnt ) {
		    defwid = sf->glyphs[i]->width;
		    cnt = widths[defwid];
		}
	widths[defwid] = 0;
	for ( i=0; i<maxw; ++i )
		for ( j=-107; j<=107; ++j )
		    if ( i+j>=0 && i+j<maxw )
			cumwid[i] += widths[i+j];
	cnt = 0; nomwid = 0;
	for ( i=0; i<maxw; ++i )
	    if ( cnt<cumwid[i] ) {
		cnt = cumwid[i];
		nomwid = i;
	    }
	free(widths); free(cumwid);
    }
    if ( _nomwid!=NULL )
	*_nomwid = nomwid;
return( defwid );
}

static void ATFigureDefWidth(SplineFont *sf, struct alltabs *at, int subfont) {
    int nomwid, defwid;

    defwid = SFFigureDefWidth(sf,&nomwid);
    if ( subfont==-1 )
	at->defwid = defwid;
    else
	at->fds[subfont].defwid = defwid;
    if ( subfont==-1 )
	at->nomwid = nomwid;
    else
	at->fds[subfont].nomwid = nomwid;
}

static void dumpcffprivate(SplineFont *sf,struct alltabs *at,int subfont,
	int subrcnt) {
    char *pt;
    FILE *private = subfont==-1?at->private:at->fds[subfont].private;
    int mi,i;
    real bluevalues[14], otherblues[10];
    real snapcnt[12];
    real stemsnaph[12], stemsnapv[12];
    real stdhw[1], stdvw[1];
    int hasblue=0, hash=0, hasv=0, bs;
    int nomwid, defwid;
    EncMap *map = at->map;

    /* The private dict is not in an index, so no index header. Just the data */

    if ( subfont==-1 )
	defwid = at->defwid;
    else
	defwid = at->fds[subfont].defwid;
    dumpintoper(private,defwid,20);		/* Default Width */
    if ( subfont==-1 )
	nomwid = at->nomwid;
    else
	nomwid = at->fds[subfont].nomwid;
    dumpintoper(private,nomwid,21);		/* Nominative Width */

    bs = SplineFontIsFlexible(sf,at->gi.flags);
    hasblue = PSDictHasEntry(sf->private,"BlueValues")!=NULL;
    hash = PSDictHasEntry(sf->private,"StdHW")!=NULL;
    hasv = PSDictHasEntry(sf->private,"StdVW")!=NULL;
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
    gwwv_progress_change_stages(2+autohint_before_generate+!hasblue);
#endif
    if ( autohint_before_generate ) {
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
	gwwv_progress_change_line1(_("Auto Hinting Font..."));
#endif
	SplineFontAutoHint(sf);
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
	gwwv_progress_next_stage();
#endif
    }

    if ( !hasblue ) {
	FindBlues(sf,bluevalues,otherblues);
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
	gwwv_progress_next_stage();
#endif
    }

    stdhw[0] = stdvw[0] = 0;
    if ( !hash ) {
	FindHStems(sf,stemsnaph,snapcnt);
	mi = -1;
	for ( i=0; stemsnaph[i]!=0 && i<12; ++i )
	    if ( mi==-1 ) mi = i;
	    else if ( snapcnt[i]>snapcnt[mi] ) mi = i;
	if ( mi!=-1 ) stdhw[0] = stemsnaph[mi];
    }

    if ( !hasv ) {
	FindVStems(sf,stemsnapv,snapcnt);
	mi = -1;
	for ( i=0; stemsnapv[i]!=0 && i<12; ++i )
	    if ( mi==-1 ) mi = i;
	    else if ( snapcnt[i]>snapcnt[mi] ) mi = i;
	if ( mi!=-1 ) stdvw[0] = stemsnapv[mi];
    }
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
    gwwv_progress_change_line1(_("Saving OpenType Font"));
#endif

    if ( hasblue )
	DumpStrArray(PSDictHasEntry(sf->private,"BlueValues"),private,6);
    else
	DumpDblArray(bluevalues,sizeof(bluevalues)/sizeof(bluevalues[0]),private,6);
    if ( (pt=PSDictHasEntry(sf->private,"OtherBlues"))!=NULL )
	DumpStrArray(pt,private,7);
    else if ( !hasblue )
	DumpDblArray(otherblues,sizeof(otherblues)/sizeof(otherblues[0]),private,7);
    if ( (pt=PSDictHasEntry(sf->private,"FamilyBlues"))!=NULL )
	DumpStrArray(pt,private,8);
    if ( (pt=PSDictHasEntry(sf->private,"FamilyOtherBlues"))!=NULL )
	DumpStrArray(pt,private,9);
    if ( (pt=PSDictHasEntry(sf->private,"BlueScale"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+9);
    if ( (pt=PSDictHasEntry(sf->private,"BlueShift"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+10);
    else
	dumpintoper(private,bs,(12<<8)+10);
    if ( (pt=PSDictHasEntry(sf->private,"BlueFuzz"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+11);
    if ( hash ) {
	DumpStrDouble(PSDictHasEntry(sf->private,"StdHW"),private,10);
	if ( (pt=PSDictHasEntry(sf->private,"StemSnapH"))!=NULL )
	    DumpStrArray(pt,private,(12<<8)|12);
    } else {
	if ( stdhw[0]!=0 )
	    dumpdbloper(private,stdhw[0],10);
	DumpDblArray(stemsnaph,sizeof(stemsnaph)/sizeof(stemsnaph[0]),private,(12<<8)|12);
    }
    if ( hasv ) {
	DumpStrDouble(PSDictHasEntry(sf->private,"StdVW"),private,11);
	if ( (pt=PSDictHasEntry(sf->private,"StemSnapV"))!=NULL )
	    DumpStrArray(pt,private,(12<<8)|13);
    } else {
	if ( stdvw[0]!=0 )
	    dumpdbloper(private,stdvw[0],11);
	DumpDblArray(stemsnapv,sizeof(stemsnapv)/sizeof(stemsnapv[0]),private,(12<<8)|13);
    }
    if ( (pt=PSDictHasEntry(sf->private,"ForceBold"))!=NULL ) {
	dumpintoper(private,*pt=='t'||*pt=='T',(12<<8)|14);
    } else if ( sf->weight!=NULL &&
	    (strstrmatch(sf->weight,"Bold")!=NULL ||
	     strstrmatch(sf->weight,"Demi")!=NULL ||
	     strstrmatch(sf->weight,"Fett")!=NULL ||
	     strstrmatch(sf->weight,"Gras")!=NULL ||
	     strstrmatch(sf->weight,"Heavy")!=NULL ||
	     strstrmatch(sf->weight,"Black")!=NULL))
	dumpintoper(private,1,(12<<8)|14);
    if ( (pt=PSDictHasEntry(sf->private,"LanguageGroup"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+17);
    else if ( map->enc->is_japanese ||
	      map->enc->is_korean ||
	      map->enc->is_tradchinese ||
	      map->enc->is_simplechinese )
	dumpintoper(private,1,(12<<8)|17);
    if ( (pt=PSDictHasEntry(sf->private,"ExpansionFactor"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+18);
    if ( subrcnt!=0 )
	dumpsizedint(private,false,ftell(private)+3+1,19);	/* Subrs */

    if ( subfont==-1 )
	at->privatelen = ftell(private);
    else
	at->fds[subfont].privatelen = ftell(private);
}

/* When we exit this the topdict is not complete, we still need to fill in */
/*  values for charset,encoding,charstrings and private. Then we need to go */
/*  back and fill in the table length (at lenpos) */
static void dumpcfftopdict(SplineFont *sf,struct alltabs *at) {
    char *pt, *end;
    FILE *cfff = at->cfff;
    DBounds b;

    putshort(cfff,1);		/* One top dict */
    putc('\2',cfff);		/* Offset size */
    putshort(cfff,1);		/* Offset to topdict */
    at->lenpos = ftell(cfff);
    putshort(cfff,0);		/* placeholder for final position (final offset in index points beyond last element) */
    dumpsid(cfff,at,sf->version,0);
    dumpsid(cfff,at,sf->copyright,1);
    dumpsid(cfff,at,sf->fullname?sf->fullname:sf->fontname,2);
    dumpsid(cfff,at,sf->familyname,3);
    dumpsid(cfff,at,sf->weight,4);
    if ( at->gi.fixed_width!=-1 ) dumpintoper(cfff,1,(12<<8)|1);
    if ( sf->italicangle!=0 ) dumpdbloper(cfff,sf->italicangle,(12<<8)|2);
    if ( sf->upos!=-100 ) dumpdbloper(cfff,sf->upos,(12<<8)|3);
    if ( sf->uwidth!=50 ) dumpdbloper(cfff,sf->uwidth,(12<<8)|4);
    if ( sf->strokedfont ) {
	dumpintoper(cfff,2,(12<<8)|5);
	dumpdbloper(cfff,sf->strokewidth,(12<<8)|8);
    }
    /* We'll never set CharstringType */
    if ( sf->ascent+sf->descent!=1000 ) {
	dumpdbl(cfff,1.0/(sf->ascent+sf->descent));
	dumpint(cfff,0);
	dumpint(cfff,0);
	dumpdbl(cfff,1.0/(sf->ascent+sf->descent));
	dumpint(cfff,0);
	dumpintoper(cfff,0,(12<<8)|7);
    }
    if ( sf->uniqueid!=-1 )
	dumpintoper(cfff, sf->uniqueid?sf->uniqueid:4000000 + (rand()&0x3ffff), 13 );
    SplineFontFindBounds(sf,&b);
    at->gi.xmin = b.minx;
    at->gi.ymin = b.miny;
    at->gi.xmax = b.maxx;
    at->gi.ymax = b.maxy;
    dumpdbl(cfff,floor(b.minx));
    dumpdbl(cfff,floor(b.miny));
    dumpdbl(cfff,ceil(b.maxx));
    dumpdbloper(cfff,ceil(b.maxy),5);
    /* We'll never set StrokeWidth */
    if ( sf->xuid!=NULL ) {
	pt = sf->xuid; if ( *pt=='[' ) ++pt;
	while ( *pt && *pt!=']' ) {
	    dumpint(cfff,strtol(pt,&end,10));
	    for ( pt = end; *pt==' '; ++pt );
	}
	putc(14,cfff);
	if ( sf->changed_since_xuidchanged )
	    SFIncrementXUID(sf);
    }
    /* Offset to charset (oper=15) needed here */
    /* Offset to encoding (oper=16) needed here (not for CID )*/
    /* Offset to charstrings (oper=17) needed here */
    /* Length of, and Offset to private (oper=18) needed here (not for CID )*/
}

static int dumpcffdict(SplineFont *sf,struct alltabs *at) {
    FILE *fdarray = at->fdarray;
    int pstart;
    /* according to the PSRef Man v3, only fontname, fontmatrix and private */
    /*  appear in this dictionary */

    dumpsid(fdarray,at,sf->fontname,(12<<8)|38);
    if ( sf->ascent+sf->descent!=1000 ) {
	dumpdbl(fdarray,1.0/(sf->ascent+sf->descent));
	dumpint(fdarray,0);
	dumpint(fdarray,0);
	dumpdbl(fdarray,1.0/(sf->ascent+sf->descent));
	dumpint(fdarray,0);
	dumpintoper(fdarray,0,(12<<8)|7);
    }
    pstart = ftell(fdarray);
    dumpsizedint(fdarray,false,0,-1);	/* private length */
    dumpsizedint(fdarray,true,0,18);	/* private offset */
return( pstart );
}

static void dumpcffdictindex(SplineFont *sf,struct alltabs *at) {
    int i;
    int pos;

    putshort(at->fdarray,sf->subfontcnt);
    putc('\2',at->fdarray);		/* DICTs aren't very big, and there are at most 255 */
    putshort(at->fdarray,1);		/* Offset to first dict */
    for ( i=0; i<sf->subfontcnt; ++i )
	putshort(at->fdarray,0);	/* Dump offset placeholders (note there's one extra to mark the end) */
    pos = ftell(at->fdarray)-1;
    for ( i=0; i<sf->subfontcnt; ++i ) {
	at->fds[i].fillindictmark = dumpcffdict(sf->subfonts[i],at);
	at->fds[i].eodictmark = ftell(at->fdarray);
	if ( at->fds[i].eodictmark>65536 )
	    IError("The DICT INDEX got too big, result won't work");
    }
    fseek(at->fdarray,2*sizeof(short)+sizeof(char),SEEK_SET);
    for ( i=0; i<sf->subfontcnt; ++i )
	putshort(at->fdarray,at->fds[i].eodictmark-pos);
    fseek(at->fdarray,0,SEEK_END);
}

static void dumpcffcidtopdict(SplineFont *sf,struct alltabs *at) {
    char *pt, *end;
    FILE *cfff = at->cfff;
    DBounds b;
    int cidcnt=0, k;

    for ( k=0; k<sf->subfontcnt; ++k )
	if ( sf->subfonts[k]->glyphcnt>cidcnt ) cidcnt = sf->subfonts[k]->glyphcnt;

    putshort(cfff,1);		/* One top dict */
    putc('\2',cfff);		/* Offset size */
    putshort(cfff,1);		/* Offset to topdict */
    at->lenpos = ftell(cfff);
    putshort(cfff,0);		/* placeholder for final position */
    dumpsid(cfff,at,sf->cidregistry,-1);
    dumpsid(cfff,at,sf->ordering,-1);
    dumpintoper(cfff,sf->supplement,(12<<8)|30);		/* ROS operator must be first */
    dumpdbloper(cfff,sf->cidversion,(12<<8)|31);
    dumpintoper(cfff,cidcnt,(12<<8)|34);
    dumpintoper(cfff, sf->uniqueid?sf->uniqueid:4000000 + (rand()&0x3ffff), (12<<8)|35 );

    dumpsid(cfff,at,sf->copyright,1);
    dumpsid(cfff,at,sf->fullname?sf->fullname:sf->fontname,2);
    dumpsid(cfff,at,sf->familyname,3);
    dumpsid(cfff,at,sf->weight,4);
    /* FontMatrix  (identity here, real ones in sub fonts)*/
    /* Actually there is no fontmatrix in the adobe cid font I'm looking at */
    /*  which means it should default to [.001...] but it doesn't so the */
    /*  docs aren't completely accurate */
    /* I now see I've no idea what the FontMatrix means in a CID keyed font */
    /*  it seems to be ignored everywhere */
#if 0
    dumpdbl(cfff,1.0);
    dumpint(cfff,0);
    dumpint(cfff,0);
    dumpdbl(cfff,1.0);
    dumpint(cfff,0);
    dumpintoper(cfff,0,(12<<8)|7);
#endif

    CIDFindBounds(sf,&b);
    at->gi.xmin = b.minx;
    at->gi.ymin = b.miny;
    at->gi.xmax = b.maxx;
    at->gi.ymax = b.maxy;
    dumpdbl(cfff,floor(b.minx));
    dumpdbl(cfff,floor(b.miny));
    dumpdbl(cfff,ceil(b.maxx));
    dumpdbloper(cfff,ceil(b.maxy),5);
    /* We'll never set StrokeWidth */
    if ( sf->xuid!=NULL ) {
	pt = sf->xuid; if ( *pt=='[' ) ++pt;
	while ( *pt && *pt!=']' ) {
	    dumpint(cfff,strtol(pt,&end,10));
	    for ( pt = end; *pt==' '; ++pt );
	}
	putc(14,cfff);
	if ( sf->changed_since_xuidchanged )
	    SFIncrementXUID(sf);
    }
#if 0
    /* Acrobat doesn't seem to care about a private dict here. Ghostscript */
    /*  dies.  Tech Note: 5176.CFF.PDF, top of page 23 says:		   */
    /*		A Private DICT is required, but may be specified as having */
    /*		a length of 0 if there are no non-default values to be stored*/
    /* No indication >where< it is required. I assumed everywhere. Perhaps */
    /*  just in basefonts? 						   */
    dumpint(cfff,0);			/* Docs say a private dict is required and they don't specifically omit CID top dicts */
    dumpintoper(cfff,0,18);		/* But they do say it can be zero */
#endif
    /* Offset to charset (oper=15) needed here */
    /* Offset to charstrings (oper=17) needed here */
    /* Offset to FDArray (oper=12,36) needed here */
    /* Offset to FDSelect (oper=12,37) needed here */
}

static int isStdEncoding(SplineFont *sf,EncMap *map) {
    int i;

    for ( i=0; i<256 && i<map->enccount; ++i ) if ( map->map[i]!=-1 && sf->glyphs[map->map[i]]!=NULL )
	if ( sf->glyphs[map->map[i]]->unicodeenc!=-1 )
	    if ( sf->glyphs[map->map[i]]->unicodeenc!=unicode_from_adobestd[i] )
return( 0 );

return( 1 );
}

static void finishup(SplineFont *sf,struct alltabs *at) {
    int strlen, shlen, glen,enclen,csetlen,cstrlen,prvlen;
    int base, eotop, strhead;
    int output_enc = ( at->format==ff_cff && !isStdEncoding(sf,at->map));

    storesid(at,NULL);		/* end the strings index */
    strlen = ftell(at->sidf) + (shlen = ftell(at->sidh));
    glen = sizeof(short);	/* Single entry: 0, no globals */
    enclen = ftell(at->encoding);
    csetlen = ftell(at->charset);
    cstrlen = ftell(at->charstrings);
    prvlen = ftell(at->private);
    base = ftell(at->cfff);
    if ( base+6*3+strlen+glen+enclen+csetlen+cstrlen+prvlen > 32767 ) {
	at->cfflongoffset = true;
	base += 5*5+4;
    } else
	base += 5*3+4;
    strhead = 2+(at->sidcnt>1);
    base += strhead;

    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen,15);		/* Charset */
    if ( output_enc )					/* encoding offset */
	dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen,16);	/* encoding offset */
    else {
	dumpsizedint(at->cfff,at->cfflongoffset,0,16);
	enclen = 0;
    }
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen+enclen,17);/* charstrings */
    dumpsizedint(at->cfff,at->cfflongoffset,at->privatelen,-1);
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen+enclen+cstrlen,18); /* private size */
    eotop = base-strhead-at->lenpos-1;
    if ( at->cfflongoffset ) {
	fseek(at->cfff,3,SEEK_SET);
	putc(4,at->cfff);
    }
    fseek(at->cfff,at->lenpos,SEEK_SET);
    putshort(at->cfff,eotop);
    fseek(at->cfff,0,SEEK_END);

    /* String Index */
    putshort(at->cfff,at->sidcnt-1);
    if ( at->sidcnt!=1 ) {		/* Everybody gets an added NULL */
	putc(at->sidlongoffset?4:2,at->cfff);
	if ( !ttfcopyfile(at->cfff,at->sidh,base,"CFF-StringBase")) at->error = true;
	if ( !ttfcopyfile(at->cfff,at->sidf,base+shlen,"CFF-StringData")) at->error = true;
    }

    /* Global Subrs */
    putshort(at->cfff,0);

    /* Charset */
    if ( !ttfcopyfile(at->cfff,at->charset,base+strlen+glen,"CFF-Charset")) at->error = true;

    /* Encoding */
    if ( !ttfcopyfile(at->cfff,at->encoding,base+strlen+glen+csetlen,"CFF-Encoding")) at->error = true;

    /* Char Strings */
    if ( !ttfcopyfile(at->cfff,at->charstrings,base+strlen+glen+csetlen+enclen,"CFF-CharStrings")) at->error = true;

    /* Private & Subrs */
    if ( !ttfcopyfile(at->cfff,at->private,base+strlen+glen+csetlen+enclen+cstrlen,"CFF-Private")) at->error = true;
}

static void finishupcid(SplineFont *sf,struct alltabs *at) {
    int strlen, shlen, glen,csetlen,cstrlen,fdsellen,fdarrlen,prvlen;
    int base, eotop, strhead;
    int i;

    storesid(at,NULL);		/* end the strings index */
    strlen = ftell(at->sidf) + (shlen = ftell(at->sidh));
    glen = ftell(at->globalsubrs);
    /* No encodings */
    csetlen = ftell(at->charset);
    fdsellen = ftell(at->fdselect);
    cstrlen = ftell(at->charstrings);
    fdarrlen = ftell(at->fdarray);
    base = ftell(at->cfff);

    at->cfflongoffset = true;
    base += 5*4+4+2;		/* two of the opers below are two byte opers */
    strhead = 2+(at->sidcnt>1);
    base += strhead;

    prvlen = 0;
    for ( i=0; i<sf->subfontcnt; ++i ) {
	fseek(at->fdarray,at->fds[i].fillindictmark,SEEK_SET);
	dumpsizedint(at->fdarray,false,at->fds[i].privatelen,-1);	/* Private len */
	dumpsizedint(at->fdarray,true,base+strlen+glen+csetlen+fdsellen+cstrlen+fdarrlen+prvlen,18);	/* Private offset */
	prvlen += ftell(at->fds[i].private);	/* private & subrs */
    }

    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen,15);	/* charset */
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen,(12<<8)|37);	/* fdselect */
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen+fdsellen,17);	/* charstrings */
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen+fdsellen+cstrlen,(12<<8)|36);	/* fdarray */
    eotop = base-strhead-at->lenpos-1;
    fseek(at->cfff,at->lenpos,SEEK_SET);
    putshort(at->cfff,eotop);
    fseek(at->cfff,0,SEEK_END);

    /* String Index */
    putshort(at->cfff,at->sidcnt-1);
    if ( at->sidcnt!=1 ) {		/* Everybody gets an added NULL */
	putc(at->sidlongoffset?4:2,at->cfff);
	if ( !ttfcopyfile(at->cfff,at->sidh,base,"CFF-StringBase")) at->error = true;
	if ( !ttfcopyfile(at->cfff,at->sidf,base+shlen,"CFF-StringData")) at->error = true;
    }

    /* Global Subrs */
    if ( !ttfcopyfile(at->cfff,at->globalsubrs,base+strlen,"CFF-GlobalSubrs")) at->error = true;

    /* Charset */
    if ( !ttfcopyfile(at->cfff,at->charset,base+strlen+glen,"CFF-Charset")) at->error = true;

    /* FDSelect */
    if ( !ttfcopyfile(at->cfff,at->fdselect,base+strlen+glen+csetlen,"CFF-FDSelect")) at->error = true;

    /* Char Strings */
    if ( !ttfcopyfile(at->cfff,at->charstrings,base+strlen+glen+csetlen+fdsellen,"CFF-CharStrings")) at->error = true;

    /* FDArray (DICT Index) */
    if ( !ttfcopyfile(at->cfff,at->fdarray,base+strlen+glen+csetlen+fdsellen+cstrlen,"CFF-FDArray")) at->error = true;

    /* Private & Subrs */
    prvlen = 0;
    for ( i=0; i<sf->subfontcnt; ++i ) {
	int temp = ftell(at->fds[i].private);
	if ( !ttfcopyfile(at->cfff,at->fds[i].private,
		base+strlen+glen+csetlen+fdsellen+cstrlen+fdarrlen+prvlen,"CFF-PrivateSubrs")) at->error = true;
	prvlen += temp;
    }

    free(at->fds);
}

static int dumpcffhmtx(struct alltabs *at,SplineFont *sf,int bitmaps) {
    DBounds b;
    SplineChar *sc;
    int i,cnt;
    int dovmetrics = sf->hasvmetrics;
    int width = at->gi.fixed_width;

    at->gi.hmtx = tmpfile();
    if ( dovmetrics )
	at->gi.vmtx = tmpfile();
    FigureFullMetricsEnd(sf,&at->gi);
    if ( at->gi.bygid[0]!=-1 && (sf->glyphs[at->gi.bygid[0]]->width==width || width==-1 )) {
	putshort(at->gi.hmtx,sf->glyphs[at->gi.bygid[0]]->width);
	SplineCharFindBounds(sf->glyphs[at->gi.bygid[0]],&b);
	putshort(at->gi.hmtx,b.minx);
	if ( dovmetrics ) {
	    putshort(at->gi.vmtx,sf->glyphs[at->gi.bygid[0]]->vwidth);
	    putshort(at->gi.vmtx,sf->vertical_origin-b.miny);
	}
    } else {
	putshort(at->gi.hmtx,width==-1?sf->ascent+sf->descent:width);
	putshort(at->gi.hmtx,0);
	if ( dovmetrics ) {
	    putshort(at->gi.vmtx,sf->ascent+sf->descent);
	    putshort(at->gi.vmtx,0);
	}
    }
    cnt = 1;
    if ( bitmaps ) {
	if ( width==-1 ) width = (sf->ascent+sf->descent)/3;
	putshort(at->gi.hmtx,width);
	putshort(at->gi.hmtx,0);
	if ( dovmetrics ) {
	    putshort(at->gi.vmtx,sf->ascent+sf->descent);
	    putshort(at->gi.vmtx,0);
	}
	putshort(at->gi.hmtx,width);
	putshort(at->gi.hmtx,0);
	if ( dovmetrics ) {
	    putshort(at->gi.vmtx,sf->ascent+sf->descent);
	    putshort(at->gi.vmtx,0);
	}
	cnt = 3;
    }

    for ( i=cnt; i<at->gi.gcnt; ++i ) if ( at->gi.bygid[i]!=-1 ) {
	sc = sf->glyphs[at->gi.bygid[i]];
	if ( SCWorthOutputting(sc) ) {
	    if ( i<=at->gi.lasthwidth )
		putshort(at->gi.hmtx,sc->width);
	    SplineCharFindBounds(sc,&b);
	    putshort(at->gi.hmtx,b.minx);
	    if ( dovmetrics ) {
		if ( i<=at->gi.lastvwidth )
		    putshort(at->gi.vmtx,sc->vwidth);
		putshort(at->gi.vmtx,sf->vertical_origin-b.maxy);
	    }
	    ++cnt;
	    if ( i==at->gi.lasthwidth )
		at->gi.hfullcnt = cnt;
	    if ( i==at->gi.lastvwidth )
		at->gi.vfullcnt = cnt;
	}
    }
    at->gi.hmtxlen = ftell(at->gi.hmtx);
    if ( at->gi.hmtxlen&2 ) putshort(at->gi.hmtx,0);
    if ( dovmetrics ) {
	at->gi.vmtxlen = ftell(at->gi.vmtx);
	if ( at->gi.vmtxlen&2 ) putshort(at->gi.vmtx,0);
    }

    at->gi.maxp->numGlyphs = cnt;
return( true );
}

static void dumpcffcidhmtx(struct alltabs *at,SplineFont *_sf) {
    DBounds b;
    SplineChar *sc;
    int cid,i,cnt=0,max;
    SplineFont *sf;
    int dovmetrics = _sf->hasvmetrics;

    at->gi.hmtx = tmpfile();
    if ( dovmetrics )
	at->gi.vmtx = tmpfile();
    FigureFullMetricsEnd(_sf,&at->gi);

    max = 0;
    for ( i=0; i<_sf->subfontcnt; ++i )
	if ( max<_sf->subfonts[i]->glyphcnt )
	    max = _sf->subfonts[i]->glyphcnt;
    for ( cid = 0; cid<max; ++cid ) {
	for ( i=0; i<_sf->subfontcnt; ++i ) {
	    sf = _sf->subfonts[i];
	    if ( cid<sf->glyphcnt && SCWorthOutputting(sf->glyphs[cid]))
	break;
	}
	if ( i!=_sf->subfontcnt ) {
	    sc = sf->glyphs[cid];
	    if ( sc->ttf_glyph<=at->gi.lasthwidth )
		putshort(at->gi.hmtx,sc->width);
	    SplineCharFindBounds(sc,&b);
	    putshort(at->gi.hmtx,b.minx);
	    if ( dovmetrics ) {
		if ( sc->ttf_glyph<=at->gi.lastvwidth )
		    putshort(at->gi.vmtx,sc->vwidth);
		putshort(at->gi.vmtx,sf->vertical_origin-b.maxy);
	    }
	    ++cnt;
	    if ( sc->ttf_glyph==at->gi.lasthwidth )
		at->gi.hfullcnt = cnt;
	    if ( sc->ttf_glyph==at->gi.lastvwidth )
		at->gi.vfullcnt = cnt;
	} else if ( cid==0 ) {
	    /* Create a dummy entry for .notdef */
	    sf = _sf->subfonts[0];
	    putshort(at->gi.hmtx,sf->ascent+sf->descent);
	    putshort(at->gi.hmtx,0);
	    ++cnt;
	    if ( dovmetrics ) {
		putshort(at->gi.vmtx,sf->ascent+sf->descent);
		putshort(at->gi.vmtx,0);
	    }
	}
    }
    at->gi.hmtxlen = ftell(at->gi.hmtx);
    if ( at->gi.hmtxlen&2 ) putshort(at->gi.hmtx,0);
    if ( dovmetrics ) {
	at->gi.vmtxlen = ftell(at->gi.vmtx);
	if ( at->gi.vmtxlen&2 ) putshort(at->gi.vmtx,0);
    }

    at->gi.maxp->numGlyphs = cnt;
}

static int dumptype2glyphs(SplineFont *sf,struct alltabs *at) {
    int i;
    struct pschars *subrs, *chrs;

    at->cfff = tmpfile();
    at->sidf = tmpfile();
    at->sidh = tmpfile();
    at->charset = tmpfile();
    at->encoding = tmpfile();
    at->private = tmpfile();

    dumpcffheader(sf,at->cfff);
    dumpcffnames(sf,at->cfff);
    dumpcffcharset(sf,at);
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
    gwwv_progress_change_stages(2+at->gi.strikecnt);
#endif

    ATFigureDefWidth(sf,at,-1);
#if defined(FONTFORGE_CONFIG_OTF_USE_SUBRS)
    if ((chrs =SplineFont2ChrsSubrs2(sf,at->nomwid,at->defwid,at->gi.bygid,at->gi.gcnt,at->gi.flags,&subrs))==NULL )
return( false );
#else
    if ((subrs = SplineFont2Subrs2(sf,at->gi.flags))==NULL )
return( false );
#endif
    dumpcffprivate(sf,at,-1,subrs->next);
    if ( subrs->next!=0 )
	_dumpcffstrings(at->private,subrs);
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
    gwwv_progress_next_stage();
#endif
#if !defined(FONTFORGE_CONFIG_OTF_USE_SUBRS)
    chrs = SplineFont2Chrs2(sf,at->nomwid,at->defwid,subrs,at->gi.flags,at->gi.bygid,at->gi.gcnt);
#endif
    at->charstrings = dumpcffstrings(chrs);
    PSCharsFree(subrs);
    if ( at->charstrings == NULL )
return( false );
    if ( at->format==ff_cff && !isStdEncoding(sf,at->map))
	dumpcffencoding(sf,at);		/* Do this after we've assigned glyph ids */
    dumpcfftopdict(sf,at);
    finishup(sf,at);

    at->cfflen = ftell(at->cfff);
    if ( at->cfflen&3 ) {
	for ( i=4-(at->cfflen&3); i>0; --i )
	    putc('\0',at->cfff);
    }

    if ( at->format!=ff_cff )
	dumpcffhmtx(at,sf,false);
    free(at->gn_sid); at->gn_sid=NULL;
return( true );
}

static int dumpcidglyphs(SplineFont *sf,struct alltabs *at) {
    int i;
    struct pschars *glbls = NULL, *chrs;

    at->cfff = tmpfile();
    at->sidf = tmpfile();
    at->sidh = tmpfile();
    at->charset = tmpfile();
    at->fdselect = tmpfile();
    at->fdarray = tmpfile();
    at->globalsubrs = tmpfile();

    at->fds = gcalloc(sf->subfontcnt,sizeof(struct fd2data));
    for ( i=0; i<sf->subfontcnt; ++i ) {
	at->fds[i].private = tmpfile();
	ATFigureDefWidth(sf->subfonts[i],at,i);
    }
#if defined(FONTFORGE_CONFIG_OTF_USE_SUBRS)
    if ( (chrs = CID2ChrsSubrs2(sf,at->fds,at->gi.flags,&glbls))==NULL )
return( false );
    for ( i=0; i<sf->subfontcnt; ++i ) {
	dumpcffprivate(sf->subfonts[i],at,i,at->fds[i].subrs->next);
	if ( at->fds[i].subrs->next!=0 )
	    _dumpcffstrings(at->fds[i].private,at->fds[i].subrs);
	PSCharsFree(at->fds[i].subrs);
    }
    _dumpcffstrings(at->globalsubrs,glbls);
    PSCharsFree(glbls);
#else
    for ( i=0; i<sf->subfontcnt; ++i ) {
	if ( (at->fds[i].subrs = SplineFont2Subrs2(sf->subfonts[i],at->gi.flags))==NULL )
return( false );
	dumpcffprivate(sf->subfonts[i],at,i,at->fds[i].subrs->next);
	if ( at->fds[i].subrs->next!=0 )
	    _dumpcffstrings(at->fds[i].private,at->fds[i].subrs);
	PSCharsFree(at->fds[i].subrs);
    }
    putshort(at->globalsubrs,0);		/* No globals */
#endif

    dumpcffheader(sf,at->cfff);
    dumpcffnames(sf,at->cfff);
    dumpcffcidset(sf,at);
    dumpcfffdselect(sf,at);
    dumpcffdictindex(sf,at);
#if defined(FONTFORGE_CONFIG_OTF_USE_SUBRS)
    if ( (at->charstrings = dumpcffstrings(chrs))==NULL )
#else
    if ( (at->charstrings = dumpcffstrings(chrs = CID2Chrs2(sf,at->fds,at->gi.flags)))==NULL )
return( false );
#endif
    for ( i=0; i<sf->subfontcnt; ++i )
	PSCharsFree(at->fds[i].subrs);
    dumpcffcidtopdict(sf,at);
    finishupcid(sf,at);

    at->cfflen = ftell(at->cfff);
    if ( at->cfflen&3 ) {
	for ( i=4-(at->cfflen&3); i>0; --i )
	    putc('\0',at->cfff);
    }

    if ( at->format!=ff_cffcid )
	dumpcffcidhmtx(at,sf);
return( true );
}

static int AnyInstructions(SplineFont *sf) {
    int i;

    if ( sf->subfontcnt!=0 ) {
	for ( i=0; i<sf->subfontcnt; ++i )
	    if ( AnyInstructions(sf->subfonts[i]))
return( true );
    } else {
	for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	    if ( sf->glyphs[i]->ttf_instrs_len!=0 )
return( true );
	}
    }
return( false );
}

static void sethead(struct head *head,SplineFont *sf,struct alltabs *at) {
    time_t now;
    uint32 now1904[4];
    uint32 year[2];
    int i, lr, rl;

    head->version = 0x00010000;
    if ( sf->subfontcnt!=0 ) {
	int val, mant;
	val = floor(sf->cidversion);
	mant = floor(65536.*(sf->cidversion-val));
	head->revision = (val<<16) | mant;
    } else if ( sf->version!=NULL ) {
	char *pt=sf->version;
	double dval;
	int val, mant;
	while ( *pt && !isdigit(*pt) && *pt!='.' ) ++pt;
	if ( *pt ) {
	    dval = strtod(pt,NULL);
	    val = floor(dval);
	    mant = floor(65536.*(dval-val));
	    head->revision = (val<<16) | mant;
	}
    }
    head->checksumAdj = 0;
    head->magicNum = 0x5f0f3cf5;
    head->flags = 8|3;
    if ( AnyInstructions(sf))
	head->flags = 0x1b;		/* baseline at 0, lsbline at 0, round ppem, instructions change metrics */
    head->emunits = sf->ascent+sf->descent;
    head->macstyle = MacStyleCode(sf,NULL);
    head->lowestreadable = 8;
    head->locais32 = 1;
    lr = rl = 0;

    for ( i=0; i<at->gi.gcnt; ++i ) if ( at->gi.bygid[i]!=-1 ) {
	SplineChar *sc = sf->glyphs[at->gi.bygid[i]];
	int uni = sc->unicodeenc ;
	if ( SCRightToLeft(sc) )
	    rl = 1;
	else if (( uni!=-1 && uni<0x10000 && islefttoright(uni)) ||
		 (uni>=0x10300 && uni<0x107ff))
	    lr = 1;
    }

    /* I assume we've always got some neutrals (spaces, punctuation) */
    if ( lr && rl )
	head->dirhint = 0;
    else if ( rl )
	head->dirhint = -2;
    else
	head->dirhint = 2;
    if ( rl )
	head->flags |= (1<<9);		/* Apple documents this */
    /* if there are any indic characters, set bit 10 */

    time(&now);		/* seconds since 1970, need to convert to seconds since 1904 */
    now1904[0] = now1904[1] = now1904[2] = now1904[3] = 0;
    year[0] = 60*60*24*365;
    year[1] = year[0]>>16; year[0] &= 0xffff;
    for ( i=4; i<70; ++i ) {
	now1904[3] += year[0];
	now1904[2] += year[1];
	if ( (i&3)==0 )
	    now1904[3] += 60*60*24;
	now1904[2] += now1904[3]>>16;
	now1904[3] &= 0xffff;
	now1904[1] += now1904[2]>>16;
	now1904[2] &= 0xffff;
    }
    now1904[3] += now&0xffff;
    now1904[2] += now>>16;
    now1904[2] += now1904[3]>>16;
    now1904[3] &= 0xffff;
    now1904[1] += now1904[2]>>16;
    now1904[2] &= 0xffff;
    head->modtime[1] = head->createtime[1] = (now1904[2]<<16)|now1904[3];
    head->modtime[0] = head->createtime[0] = (now1904[0]<<16)|now1904[1];
}

static void sethhead(struct hhead *hhead,struct hhead *vhead,struct alltabs *at, SplineFont *sf) {
    int i, width, rbearing, height, bbearing;
    int ymax, ymin, xmax, xmin, off;
    DBounds bb;
    /* Might as well fill in the vhead even if we don't use it */
    /*  we just won't dump it out if we don't want it */

    width = 0x80000000; rbearing = 0x7fffffff; height = 0x80000000; bbearing=0x7fffffff;
    xmax = ymax = 0x80000000; xmin = ymin = 0x7fffffff;
    for ( i=0; i<at->gi.gcnt; ++i ) if ( at->gi.bygid[i]!=-1 ) {
	SplineChar *sc = sf->glyphs[at->gi.bygid[i]];
	SplineCharFindBounds(sc,&bb);
	if ( sc->width>width ) width = sc->width;
	if ( sc->vwidth>height ) height = sc->vwidth;
	if ( sc->width-bb.maxx < rbearing ) rbearing = sc->width-bb.maxx;
	if ( sc->vwidth-bb.maxy < bbearing ) bbearing = sc->vwidth-bb.maxy;
	if ( bb.maxy > ymax ) ymax = bb.maxy;
	if ( bb.miny < ymin ) ymin = bb.miny;
	if ( bb.maxx > xmax ) xmax = bb.maxx;
	if ( bb.minx < xmin ) xmin = bb.minx;
    }

    if ( at->head.ymax>ymax ) ymax = at->head.ymax;	/* If generated .notdef glyph is bigger than real glyphs */
    if ( at->head.ymin<ymin ) ymin = at->head.ymin;

    if ( ymax==0 && ymin==0 ) {
	/* this can happen in a bitmap only font */
	ymax = sf->ascent;
	ymin = -sf->descent;
    }
    hhead->version = 0x00010000;
    if ( sf->pfminfo.hheadascent_add )
	hhead->ascender = ymax + sf->pfminfo.hhead_ascent;
    else
	hhead->ascender = sf->pfminfo.hhead_ascent;
    if ( sf->pfminfo.hheadascent_add )
	hhead->descender = ymin + sf->pfminfo.hhead_descent;
    else
	hhead->descender = sf->pfminfo.hhead_descent;
    hhead->linegap = sf->pfminfo.linegap;

    vhead->version = 0x00011000;
    off = (sf->ascent+sf->descent)/2;
    vhead->ascender = xmax-off;
    vhead->descender = xmin-off;
    vhead->linegap = sf->pfminfo.linegap;

    at->isfixed = at->gi.fixed_width!=-1;
    hhead->maxwidth = width;
    hhead->minlsb = at->head.xmin;
    hhead->minrsb = rbearing;
    /* Apple's ftxvalidator says the the min sidebearing should be 0 even if it isn't */
    if ( hhead->minlsb>0 ) hhead->minlsb = 0;
    if ( hhead->minrsb>0 ) hhead->minrsb = 0;
    hhead->maxextent = at->head.xmax;
    if ( sf->italicangle==0 )
	hhead->caretSlopeRise = 1;
    else {
	hhead->caretSlopeRise = 100;
	hhead->caretSlopeRun = (int) rint(100*tan(-sf->italicangle*3.1415926535897/180.));
    }

    vhead->maxwidth = height;
    vhead->minlsb = at->head.ymin;
    vhead->minrsb = bbearing;
    vhead->maxextent = at->head.ymax;
    vhead->caretSlopeRise = 0;
    vhead->caretSlopeRun = 1;
	/* Are there vertical oblique fonts? */

    hhead->numMetrics = at->gi.hfullcnt;
    vhead->numMetrics = at->gi.vfullcnt;
}

static void setvorg(struct vorg *vorg, SplineFont *sf) {
    vorg->majorVersion = 1;
    vorg->minorVersion = 0;
    vorg->defaultVertOriginY = sf->vertical_origin;
    vorg->numVertOriginYMetrics = 0;
}

static void OS2WeightCheck(struct pfminfo *pfminfo,char *weight) {
    if ( weight==NULL ) {
	/* default it */
    } else if ( strstrmatch(weight,"medi")!=NULL ) {
	pfminfo->weight = 500;
	pfminfo->panose[2] = 6;
    } else if ( strstrmatch(weight,"demi")!=NULL ||
		strstrmatch(weight,"halb")!=NULL ||
		(strstrmatch(weight,"semi")!=NULL &&
		    strstrmatch(weight,"bold")!=NULL) ) {
	pfminfo->weight = 600;
	pfminfo->panose[2] = 7;
    } else if ( strstrmatch(weight,"bold")!=NULL ||
		strstrmatch(weight,"fett")!=NULL ||
		strstrmatch(weight,"gras")!=NULL ) {
	pfminfo->weight = 700;
	pfminfo->panose[2] = 8;
    } else if ( strstrmatch(weight,"heavy")!=NULL ) {
	pfminfo->weight = 800;
	pfminfo->panose[2] = 9;
    } else if ( strstrmatch(weight,"black")!=NULL ) {
	pfminfo->weight = 900;
	pfminfo->panose[2] = 10;
    } else if ( strstrmatch(weight,"nord")!=NULL ) {
	pfminfo->weight = 950;
	pfminfo->panose[2] = 11;
    } else if ( strstrmatch(weight,"thin")!=NULL ) {
	pfminfo->weight = 100;
	pfminfo->panose[2] = 2;
    } else if ( strstrmatch(weight,"extra")!=NULL ||
	    strstrmatch(weight,"light")!=NULL ) {
	pfminfo->weight = 200;
	pfminfo->panose[2] = 3;
    } else if ( strstrmatch(weight,"light")!=NULL ) {
	pfminfo->weight = 300;
	pfminfo->panose[2] = 4;
    }
}

void SFDefaultOS2Simple(struct pfminfo *pfminfo,SplineFont *sf) {
    pfminfo->pfmfamily = 0x11;
    pfminfo->panose[0] = 2;
    pfminfo->weight = 400;
    pfminfo->panose[2] = 5;
    pfminfo->width = 5;
    pfminfo->panose[3] = 3;
    pfminfo->winascent_add = pfminfo->windescent_add = true;
    pfminfo->hheadascent_add = pfminfo->hheaddescent_add = true;
    pfminfo->typoascent_add = pfminfo->typodescent_add = true;
    pfminfo->os2_winascent = pfminfo->os2_windescent = 0;

    if ( sf->subfonts!=NULL ) sf = sf->subfonts[0];
    pfminfo->linegap = pfminfo->vlinegap = pfminfo->os2_typolinegap = 
	    rint(.09*(sf->ascent+sf->descent));
}

void SFDefaultOS2SubSuper(struct pfminfo *pfminfo,int emsize) {
    pfminfo->os2_supysize = pfminfo->os2_subysize = .7*emsize;
    pfminfo->os2_supxsize = pfminfo->os2_subxsize = .65*emsize;
    pfminfo->os2_subyoff = .14*emsize;
    pfminfo->os2_subxoff = pfminfo->os2_supxoff = 0;
    pfminfo->os2_subyoff = .48*emsize;
    pfminfo->os2_strikeysize = 102*emsize/2048;
    pfminfo->os2_strikeypos = 530*emsize/2048;
}

void SFDefaultOS2Info(struct pfminfo *pfminfo,SplineFont *sf,char *fontname) {
    int samewid= -1;
    char *weight = sf->cidmaster==NULL ? sf->weight : sf->cidmaster->weight;

    if ( sf->pfminfo.pfmset ) {
	if ( pfminfo!=&sf->pfminfo )
	    *pfminfo = sf->pfminfo;
	if ( !pfminfo->panose_set ) {
	    struct pfminfo info;
	    memset(&info,0,sizeof(info));
	    sf->pfminfo.pfmset = false;
	    SFDefaultOS2Info(&info,sf,fontname);
	    sf->pfminfo.pfmset = true;
	    memcpy(pfminfo->panose,info.panose,sizeof(info.panose));
	}
    } else {
	memset(pfminfo,'\0',sizeof(*pfminfo));
	SFDefaultOS2Simple(pfminfo,sf);
	samewid = CIDOneWidth(sf);

	pfminfo->pfmfamily = 0x10;
	if ( samewid>0 ) {
	    pfminfo->pfmfamily = 0x30;
	    /* pfminfo->panose[3] = 9; */ /* This is done later */
	} else if ( strstrmatch(fontname,"sans")!=NULL )
	    pfminfo->pfmfamily = 0x20;
	else if ( strstrmatch(fontname,"script")!=NULL ) {
	    pfminfo->pfmfamily = 0x40;
	    pfminfo->panose[0] = 3;
	}
	if ( samewid==-1 )
	    pfminfo->pfmfamily |= 0x1;	/* Else it assumes monospace */

/* urw uses 4 character abreviations */
	if ( weight!=NULL )
	    OS2WeightCheck(pfminfo,weight);
	OS2WeightCheck(pfminfo,fontname);

	if ( strstrmatch(fontname,"ultra")!=NULL &&
		strstrmatch(fontname,"condensed")!=NULL ) {
	    pfminfo->width = 1;
	    pfminfo->panose[3] = 8;
	} else if ( strstrmatch(fontname,"extra")!=NULL &&
		strstrmatch(fontname,"condensed")!=NULL ) {
	    pfminfo->width = 2;
	    pfminfo->panose[3] = 8;
	} else if ( strstrmatch(fontname,"semi")!=NULL &&
		strstrmatch(fontname,"condensed")!=NULL ) {
	    pfminfo->width = 4;
	    pfminfo->panose[3] = 6;
	} else if ( strstrmatch(fontname,"condensed")!=NULL ||
		strstrmatch(fontname,"narrow")!=NULL ) {
	    pfminfo->width = 3;
	    pfminfo->panose[3] = 6;
	} else if ( strstrmatch(fontname,"ultra")!=NULL &&
		strstrmatch(fontname,"expanded")!=NULL ) {
	    pfminfo->width = 9;
	    pfminfo->panose[3] = 7;
	} else if ( strstrmatch(fontname,"extra")!=NULL &&
		strstrmatch(fontname,"expanded")!=NULL ) {
	    pfminfo->width = 8;
	    pfminfo->panose[3] = 7;
	} else if ( strstrmatch(fontname,"semi")!=NULL &&
		strstrmatch(fontname,"expanded")!=NULL ) {
	    pfminfo->width = 6;
	    pfminfo->panose[3] = 5;
	} else if ( strstrmatch(fontname,"expanded")!=NULL ) {
	    pfminfo->width = 7;
	    pfminfo->panose[3] = 5;
	}
	if ( samewid>0 )
	    pfminfo->panose[3] = 9;
    }
    if ( !pfminfo->subsuper_set )
	SFDefaultOS2SubSuper(pfminfo,sf->ascent+sf->descent);
}

void OS2FigureCodePages(SplineFont *sf, uint32 CodePage[2]) {
    int i, k;
    uint32 latin1[8];
    int has_ascii, has_latin1;

    memset(latin1,0,sizeof(latin1));
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( sf->glyphs[i]->unicodeenc<256 && sf->glyphs[i]->unicodeenc>=0 )
	    latin1[(sf->glyphs[i]->unicodeenc>>5)] |= 1<<(sf->glyphs[i]->unicodeenc&31);
    }

    has_ascii = latin1[1]==0xffffffff && latin1[2]==0xffffffff &&
	    (latin1[3]&0x7fffffff)==0x7fffffff;		/* DEL is not a char */
    has_latin1 = has_ascii && (latin1[5]&~1&~(1<<0xd))== (~1&~(1<<0xd))	&& /* Ignore nobreak space, softhyphen */
	    latin1[6]==0xffffffff && latin1[7] == 0xffffffff;
    CodePage[0] = CodePage[1] = 0;
    if ( has_ascii ) CodePage[1] |= 1U<<31;		/* US (Ascii I assume) */
    if ( has_latin1 ) CodePage[0] |= 1U<<30;		/* WE/latin1 */

    k=0;
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( sf->glyphs[i]->unicodeenc==0xde && has_ascii )
	    CodePage[0] |= 1<<0;		/* (ANSI) Latin1 */
	else if ( sf->glyphs[i]->unicodeenc==0x13d && has_ascii ) {
	    CodePage[0] |= 1<<1;		/* latin2 */
	    CodePage[1] |= 1<<26;		/* latin2 */
	} else if ( sf->glyphs[i]->unicodeenc==0x411 ) {
	    CodePage[0] |= 1<<2;		/* cyrillic */
	    CodePage[1] |= 1<<17;		/* MS DOS Russian */
	    CodePage[1] |= 1<<25;		/* IBM Cyrillic */
	} else if ( sf->glyphs[i]->unicodeenc==0x386 ) {
	    CodePage[0] |= 1<<3;		/* greek */
	    CodePage[1] |= 1<<16;		/* IBM Greek */
	    CodePage[1] |= 1<<28;		/* Greek, former 437 G */
	} else if ( sf->glyphs[i]->unicodeenc==0x130 && has_ascii ) {
	    CodePage[0] |= 1<<4;		/* turkish */
	    CodePage[1] |= 1<<24;		/* IBM turkish */
	} else if ( sf->glyphs[i]->unicodeenc==0x5d0 ) {
	    CodePage[0] |= 1<<5;		/* hebrew */
	    CodePage[1] |= 1<<21;		/* hebrew */
	} else if ( sf->glyphs[i]->unicodeenc==0x631 ) {
	    CodePage[0] |= 1<<6;		/* arabic */
	    CodePage[1] |= 1<<19;		/* arabic */
	    CodePage[1] |= 1<<29;		/* arabic; ASMO 708 */
	} else if ( sf->glyphs[i]->unicodeenc==0x157 && has_ascii ) {
	    CodePage[0] |= 1<<7;		/* baltic */
	    CodePage[1] |= 1<<27;		/* baltic */
	} else if ( sf->glyphs[i]->unicodeenc==0x20AB && has_ascii ) {
	    CodePage[0] |= 1<<8;		/* vietnamese */
	} else if ( sf->glyphs[i]->unicodeenc==0xe45 )
	    CodePage[0] |= 1<<16;		/* thai */
	else if ( sf->glyphs[i]->unicodeenc==0x30a8 )
	    CodePage[0] |= 1<<17;		/* japanese */
	else if ( sf->glyphs[i]->unicodeenc==0x3105 )
	    CodePage[0] |= 1<<18;		/* simplified chinese */
	else if ( sf->glyphs[i]->unicodeenc==0x3131 )
	    CodePage[0] |= 1<<19;		/* korean wansung */
	else if ( sf->glyphs[i]->unicodeenc==0x592E )
	    CodePage[0] |= 1<<20;		/* traditional chinese */
	else if ( sf->glyphs[i]->unicodeenc==0xacf4 )
	    CodePage[0] |= 1<<21;		/* korean Johab */
	else if ( sf->glyphs[i]->unicodeenc==0x2030 && has_ascii )
	    CodePage[0] |= 1U<<29;		/* mac roman */
	else if ( sf->glyphs[i]->unicodeenc==0x2665 && has_ascii )
	    CodePage[0] |= 1U<<30;		/* OEM */
#if 0	/* the symbol bit doesn't mean it contains the glyphs in symbol */
	/* rather that one is using a symbol encoding. Or that there are */
	/* glyphs with unicode encoding between 0xf000 and 0xf0ff, in which */
	/* case those guys should be given a symbol encoding */
	/* There's a bug in the way otf fonts handle this (but not ttf) and */
	/*  they only seem to list the symbol glyphs */
	else if ( sf->glyphs[i]->unicodeenc==0x21d4 )
	    CodePage[0] |= 1U<<31;		/* symbol */
#else
	/* This doesn't work well either. In ttf fonts the bit is ignored */
	/*  in otf fonts the bit means "ignore all other bits" */
	else if ( sf->glyphs[i]->unicodeenc>=0xf000 && sf->glyphs[i]->unicodeenc<=0xf0ff )
	    CodePage[0] |= 1U<<31;		/* symbol */
#endif
	else if ( sf->glyphs[i]->unicodeenc==0xc3 && has_ascii )
	    CodePage[1] |= 1<<18;		/* MS-DOS Nordic */
	else if ( sf->glyphs[i]->unicodeenc==0xe9 && has_ascii )
	    CodePage[1] |= 1<<20;		/* MS-DOS Canadian French */
	else if ( sf->glyphs[i]->unicodeenc==0xf5 && has_ascii )
	    CodePage[1] |= 1<<23;		/* MS-DOS Portuguese */
	else if ( sf->glyphs[i]->unicodeenc==0xfe && has_ascii )
	    CodePage[1] |= 1<<22;		/* MS-DOS Icelandic */
    }
}

static void WinBB(SplineFont *sf,uint16 *winascent,uint16 *windescent,struct alltabs *at) {
    /* The windows ascent/descent is calculated on the ymin/max of the */
    /*  glyphs in the so called ANSI character set. I'm going to pretend */
    /*  that's Latin1 with a few additions */
    /* Well, that's what is documented, but the documentation says contradictory */
    /*  things. I believe that winAscent should be the same as hhea.ascent */

    *winascent = at->head.ymax;
    *windescent = -at->head.ymin;		/* Should be positive */
    if ( sf->cidmaster!=NULL )
	sf = sf->cidmaster;

    if ( sf->pfminfo.winascent_add )
	*winascent += sf->pfminfo.os2_winascent;
    else
	*winascent  = sf->pfminfo.os2_winascent;
    if ( sf->pfminfo.windescent_add )
	*windescent += sf->pfminfo.os2_windescent;
    else
	*windescent  = sf->pfminfo.os2_windescent;
}

static void setos2(struct os2 *os2,struct alltabs *at, SplineFont *sf,
	enum fontformat format) {
    int i,j,cnt1,cnt2,first,last,avg1,avg2,gid;
    char *pt;
    static int const weightFactors[26] = { 64, 14, 27, 35, 100, 20, 14, 42, 63,
	3, 6, 35, 20, 56, 56, 17, 4, 49, 56, 71, 31, 10, 18, 3, 18, 2 };
    EncMap *map;

    os2->version = 1;
    os2->weightClass = sf->pfminfo.weight;
    os2->widthClass = sf->pfminfo.width;
    os2->fstype = 0x8;
    if ( sf->pfminfo.fstype!=-1 )
	os2->fstype = sf->pfminfo.fstype;
    if ( !sf->pfminfo.subsuper_set )
	SFDefaultOS2SubSuper(&sf->pfminfo,sf->ascent+sf->descent);
    os2->ysupYSize = sf->pfminfo.os2_supysize;
    os2->ysubXSize = sf->pfminfo.os2_subxsize;
    os2->ysubYSize = sf->pfminfo.os2_subysize;
    os2->ysupXSize = sf->pfminfo.os2_supxsize;
    os2->ysubYOff = sf->pfminfo.os2_subyoff;
    os2->ysubXOff = sf->pfminfo.os2_subxoff;
    os2->ysupXOff = sf->pfminfo.os2_supxoff;
    os2->ysupYOff = sf->pfminfo.os2_supyoff;
    os2->yStrikeoutSize = sf->pfminfo.os2_strikeysize;
    os2->yStrikeoutPos = sf->pfminfo.os2_strikeypos;
    os2->fsSel = (at->head.macstyle&1?32:0)|(at->head.macstyle&2?1:0);
    if ( sf->fullname!=NULL && strstrmatch(sf->fullname,"outline")!=NULL )
	os2->fsSel |= 8;
    if ( os2->fsSel==0 ) os2->fsSel = 64;		/* Regular */
/* David Lemon @Adobe.COM
1)  The sTypoAscender and sTypoDescender values should sum to 2048 in 
a 2048-unit font. They indicate the position of the em square 
relative to the baseline.
GWW: Nope, sTypoAscender-sTypoDescender == EmSize

2)  The usWinAscent and usWinDescent values represent the maximum 
height and depth of specific glyphs within the font, and some 
applications will treat them as the top and bottom of the font 
bounding box. (the "ANSI" glyphs)
GWW: That's what's documented. But it means non-ANSI glyphs get clipped. So the
docs are wrong.
*/
    if ( sf->pfminfo.typoascent_add )
	os2->ascender = sf->ascent + sf->pfminfo.os2_typoascent;
    else
	os2->ascender = sf->pfminfo.os2_typoascent;
    if ( sf->pfminfo.typodescent_add )
	os2->descender = -sf->descent + sf->pfminfo.os2_typodescent;	/* Should be neg */
    else
	os2->descender = sf->pfminfo.os2_typodescent;
    WinBB(sf,&os2->winascent,&os2->windescent,at);
    os2->linegap = sf->pfminfo.os2_typolinegap;
    os2->sFamilyClass = sf->pfminfo.os2_family_class;

    avg1 = avg2 = last = 0; first = 0x10000;
    cnt1 = cnt2 = 0;
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( SCWorthOutputting(sf->glyphs[i]) &&
		sf->glyphs[i]->unicodeenc!=-1 ) {
	    if ( sf->glyphs[i]->unicodeenc > 0xffff )
		os2->unicoderange[57>>5] |= (1<<(57&31));
	    for ( j=0; j<sizeof(uniranges)/sizeof(uniranges[0]); ++j )
		if ( sf->glyphs[i]->unicodeenc>=uniranges[j][0] &&
			sf->glyphs[i]->unicodeenc<=uniranges[j][1] ) {
		    int bit = uniranges[j][2];
		    os2->unicoderange[bit>>5] |= (1<<(bit&31));
	    break;
		}
	    if ( sf->glyphs[i]->unicodeenc<first ) first = sf->glyphs[i]->unicodeenc;
	    if ( sf->glyphs[i]->unicodeenc>last ) last = sf->glyphs[i]->unicodeenc;
	    if ( sf->glyphs[i]->width!=0 ) {
		avg2 += sf->glyphs[i]->width; ++cnt2;
	    }
	    if ( sf->glyphs[i]->unicodeenc==' ') {
		avg1 += sf->glyphs[i]->width * 166; ++cnt1;
	    } else if (sf->glyphs[i]->unicodeenc>='a' && sf->glyphs[i]->unicodeenc<='z') {
		avg1 += sf->glyphs[i]->width * weightFactors[sf->glyphs[i]->unicodeenc-'a']; ++cnt1;
	    }
	}
    }
    if ( format==ff_ttfsym )	/* MS Symbol font has this set to zero. Does it matter? */
	memset(os2->unicoderange,0,sizeof(os2->unicoderange));

    if ( sf->pfminfo.pfmset )
	strncpy(os2->achVendID,sf->pfminfo.os2_vendor,4);
    else if ( TTFFoundry!=NULL )
	strncpy(os2->achVendID,TTFFoundry,4);
    else
	memcpy(os2->achVendID,"PfEd",4);
    for ( pt=os2->achVendID; pt<os2->achVendID && *pt!='\0'; ++pt );
    while ( pt<os2->achVendID ) *pt++ = ' ';	/* Pad with spaces not NUL */

    /* v1,2 & v3,4 have different ways of calculating avgCharWid. */
    /* but I'm told that using the v3 way breaks display of CJK fonts in windows */
    os2->avgCharWid = 500;
    os2->v1_avgCharWid = os2->v3_avgCharWid = 0;
    if ( cnt1==27 )
	os2->v1_avgCharWid = avg1/1000;
    if ( cnt2!=0 )
	os2->v3_avgCharWid = avg2/cnt2;
    memcpy(os2->panose,sf->pfminfo.panose,sizeof(os2->panose));
    map = at->map;
    if ( format==ff_ttfsym ) {
	os2->ulCodePage[0] = 0x80000000;
	os2->ulCodePage[1] = 0;
	first = 255; last = 0;
	for ( i=0; i<map->enccount && i<255; ++i )
	    if ( (gid=map->map[i])!=-1 && sf->glyphs[gid]!=NULL &&
		    sf->glyphs[gid]->ttf_glyph!=-1 ) {
		if ( i<first ) first = i;
		if ( i>last ) last = i;
	    }
	os2->firstcharindex = 0xf000 + first;
	os2->lastcharindex  = 0xf000 + last;
    } else {
	if ( first>13 && format!=ff_otf && format!=ff_otfcid ) first = 13;	/* We give the font an extra char mapped to cr (13) */
	os2->firstcharindex = first;
	os2->lastcharindex = last;
	OS2FigureCodePages(sf, os2->ulCodePage);
	/* Herbert Duerr: */
	/* Some old versions of Windows do not provide access to all    */
	/* glyphs in a font if the fonts contains non-PUA symbols       */
	/* and thus only has sets the codepage flag for symbol          */
	/* => a workaround for this problem on Windows legacy versions  */
	/* is to use an OS2-table version without codepage flags        */
	/* GWW: */
	/* This sounds to me like a windows bug rather than one in ff	*/
	/*  and this is a work-around for windows. As far as I can tell	*/
	/*  ff is setting the codepage field properly, it's just that	*/
	/*  windows doesn't interpret that bit correctly		*/
	if( format!=ff_ttfsym )
	    if( (os2->ulCodePage[0]&~(1U<<31))==0 && os2->ulCodePage[1]==0 )
                os2->version = 0;
    }

    if ( format==ff_otf || format==ff_otfcid ) {
	BlueData bd;

	QuickBlues(sf,&bd);		/* This handles cid fonts properly */
	/*os2->version = 2;*/		/* current version is 3. It added some bit defns but no fields */
	if ( os2->version != 0 )
	    os2->version = 3;
	os2->xHeight = bd.xheight;
	os2->capHeight = bd.caph;
	os2->defChar = ' ';
	os2->breakChar = ' ';
	os2->maxContext = 1;	/* Kerning will set this to 2, ligature to whatever */
    }

    if ( os2->version==3 && os2->v3_avgCharWid!=0 )
	os2->avgCharWid = os2->v3_avgCharWid;
    else if ( os2->v1_avgCharWid!=0 )
	os2->avgCharWid = os2->v1_avgCharWid;
    else if ( os2->v3_avgCharWid!=0 )
	os2->avgCharWid = os2->v3_avgCharWid;
}

static void redoloca(struct alltabs *at) {
    int i;

    at->loca = tmpfile();
    if ( at->head.locais32 ) {
	for ( i=0; i<=at->maxp.numGlyphs; ++i )
	    putlong(at->loca,at->gi.loca[i]);
	at->localen = sizeof(int32)*(at->maxp.numGlyphs+1);
    } else {
	for ( i=0; i<=at->maxp.numGlyphs; ++i )
	    putshort(at->loca,at->gi.loca[i]/2);
	at->localen = sizeof(int16)*(at->maxp.numGlyphs+1);
	if ( ftell(at->loca)&2 )
	    putshort(at->loca,0);
    }
    if ( at->format!=ff_type42 && at->format!=ff_type42cid ) {
	free(at->gi.loca);
	at->gi.loca = NULL;
    }
}

static void dummyloca(struct alltabs *at) {

    at->loca = tmpfile();
    if ( at->head.locais32 ) {
	putlong(at->loca,0);
	at->localen = sizeof(int32);
    } else {
	putshort(at->loca,0);
	at->localen = sizeof(int16);
	putshort(at->loca,0);	/* pad it */
    }
}

static void redohead(struct alltabs *at) {
    at->headf = tmpfile();

    putlong(at->headf,at->head.version);
    putlong(at->headf,at->head.revision);
    putlong(at->headf,at->head.checksumAdj);
    putlong(at->headf,at->head.magicNum);
    putshort(at->headf,at->head.flags);
    putshort(at->headf,at->head.emunits);
    putlong(at->headf,at->head.createtime[0]);
    putlong(at->headf,at->head.createtime[1]);
    putlong(at->headf,at->head.modtime[0]);
    putlong(at->headf,at->head.modtime[1]);
    putshort(at->headf,at->head.xmin);
    putshort(at->headf,at->head.ymin);
    putshort(at->headf,at->head.xmax);
    putshort(at->headf,at->head.ymax);
    putshort(at->headf,at->head.macstyle);
    putshort(at->headf,at->head.lowestreadable);
    putshort(at->headf,at->head.dirhint);
    putshort(at->headf,at->head.locais32);
    putshort(at->headf,at->head.glyphformat);

    at->headlen = ftell(at->headf);
    if ( (at->headlen&2)!=0 )
	putshort(at->headf,0);
}

static void redohhead(struct alltabs *at,int isv) {
    int i;
    struct hhead *head;
    FILE *f;

    if ( !isv ) {
	f = at->hheadf = tmpfile();
	head = &at->hhead;
    } else {
	f = at->vheadf = tmpfile();
	head = &at->vhead;
    }

    putlong(f,head->version);
    putshort(f,head->ascender);
    putshort(f,head->descender);
    putshort(f,head->linegap);
    putshort(f,head->maxwidth);
    putshort(f,head->minlsb);
    putshort(f,head->minrsb);
    putshort(f,head->maxextent);
    putshort(f,head->caretSlopeRise);
    putshort(f,head->caretSlopeRun);
    for ( i=0; i<5; ++i )
	putshort(f,head->mbz[i]);
    putshort(f,head->metricformat);
    putshort(f,head->numMetrics);

    if ( !isv ) {
	at->hheadlen = ftell(f);
	if ( (at->hheadlen&2)!=0 )
	    putshort(f,0);
    } else {
	at->vheadlen = ftell(f);
	if ( (at->vheadlen&2)!=0 )
	    putshort(f,0);
    }
}

static void redovorg(struct alltabs *at) {

    at->vorgf = tmpfile();
    putshort(at->vorgf,at->vorg.majorVersion);
    putshort(at->vorgf,at->vorg.minorVersion);
    putshort(at->vorgf,at->vorg.defaultVertOriginY);
    putshort(at->vorgf,at->vorg.numVertOriginYMetrics);

    at->vorglen = ftell(at->vorgf);
    if ( (at->vorglen&2)!=0 )
	putshort(at->vorgf,0);
}

static void redomaxp(struct alltabs *at,enum fontformat format) {
    at->maxpf = tmpfile();

    putlong(at->maxpf,at->maxp.version);
    putshort(at->maxpf,at->maxp.numGlyphs);
    if ( format!=ff_otf && format!=ff_otfcid ) {
	putshort(at->maxpf,at->maxp.maxPoints);
	putshort(at->maxpf,at->maxp.maxContours);
	putshort(at->maxpf,at->maxp.maxCompositPts);
	putshort(at->maxpf,at->maxp.maxCompositCtrs);
	putshort(at->maxpf,at->maxp.maxZones);
	putshort(at->maxpf,at->maxp.maxTwilightPts);
	putshort(at->maxpf,at->maxp.maxStorage);
	putshort(at->maxpf,at->maxp.maxFDEFs);
	putshort(at->maxpf,at->maxp.maxIDEFs);
	putshort(at->maxpf,at->maxp.maxStack);
	putshort(at->maxpf,at->maxp.maxglyphInstr);
	putshort(at->maxpf,at->maxp.maxnumcomponents);
	putshort(at->maxpf,at->maxp.maxcomponentdepth);
    }

    at->maxplen = ftell(at->maxpf);
    if ( (at->maxplen&2)!=0 )
	putshort(at->maxpf,0);
}

static void redoos2(struct alltabs *at) {
    int i;
    at->os2f = tmpfile();

    putshort(at->os2f,at->os2.version);
    putshort(at->os2f,at->os2.avgCharWid);
    putshort(at->os2f,at->os2.weightClass);
    putshort(at->os2f,at->os2.widthClass);
    putshort(at->os2f,at->os2.fstype);
    putshort(at->os2f,at->os2.ysubXSize);
    putshort(at->os2f,at->os2.ysubYSize);
    putshort(at->os2f,at->os2.ysubXOff);
    putshort(at->os2f,at->os2.ysubYOff);
    putshort(at->os2f,at->os2.ysupXSize);
    putshort(at->os2f,at->os2.ysupYSize);
    putshort(at->os2f,at->os2.ysupXOff);
    putshort(at->os2f,at->os2.ysupYOff);
    putshort(at->os2f,at->os2.yStrikeoutSize);
    putshort(at->os2f,at->os2.yStrikeoutPos);
    putshort(at->os2f,at->os2.sFamilyClass);
    for ( i=0; i<10; ++i )
	putc(at->os2.panose[i],at->os2f);
    for ( i=0; i<4; ++i )
	putlong(at->os2f,at->os2.unicoderange[i]);
    for ( i=0; i<4; ++i )
	putc(at->os2.achVendID[i],at->os2f);
    putshort(at->os2f,at->os2.fsSel);
    putshort(at->os2f,at->os2.firstcharindex);
    putshort(at->os2f,at->os2.lastcharindex);
    putshort(at->os2f,at->os2.ascender);
    putshort(at->os2f,at->os2.descender);
    putshort(at->os2f,at->os2.linegap);
    putshort(at->os2f,at->os2.winascent);
    putshort(at->os2f,at->os2.windescent);
    if ( at->os2.version>=1 ) {
    putlong(at->os2f,at->os2.ulCodePage[0]);
    putlong(at->os2f,at->os2.ulCodePage[1]);
    }

    if ( at->os2.version>=2 ) {
	putshort(at->os2f,at->os2.xHeight);
	putshort(at->os2f,at->os2.capHeight);
	putshort(at->os2f,at->os2.defChar);
	putshort(at->os2f,at->os2.breakChar);
	putshort(at->os2f,at->os2.maxContext);
    }

    at->os2len = ftell(at->os2f);
    if ( (at->os2len&2)!=0 )
	putshort(at->os2f,0);
}

#if 0
static int icomp(const void *a, const void *b) {
    return *(int *)a - *(int *)b;
}
#endif

static void dumpgasp(struct alltabs *at, SplineFont *sf) {
#if 1
    at->gaspf = tmpfile();
    putshort(at->gaspf,0);	/* Version number */
    putshort(at->gaspf,1);
#else		/* What was I thinking? I don't check which bitmaps we are outputting even */
    BDFFont *bdf;
    int i, nbitmaps = 0, rangecnt = 0;
    int *bitmapsizes;

    for ( bdf=sf->bitmaps; bdf!=NULL; bdf=bdf->next )
	if ( BDFDepth(bdf)==1 ) nbitmaps++;
    bitmapsizes = gcalloc( nbitmaps, sizeof(int) );
    for ( i=0, bdf=sf->bitmaps; bdf!=NULL; bdf=bdf->next )
	if ( BDFDepth(bdf)==1 ) bitmapsizes[i++] = bdf->pixelsize;
    qsort ( bitmapsizes, nbitmaps, sizeof(int), icomp );
    rangecnt = 1;
    if ( nbitmaps>0 && bitmapsizes[0]>1 ) rangecnt++;
    for ( i=0; i<nbitmaps; i++ ) {
        if ( i>0 && bitmapsizes[i-1]+1<bitmapsizes[i] ) rangecnt++;
	if ( i==nbitmaps-1 || bitmapsizes[i]+1<bitmapsizes[i+1] ) rangecnt++;
    }

    at->gaspf = tmpfile();
    putshort(at->gaspf,0);	/* Version number */
    putshort(at->gaspf,rangecnt);
    for ( i=0; i<nbitmaps; i++ ) {
	if ( (i==0 && bitmapsizes[i]>1) || 
	     (i>0 && bitmapsizes[i-1]+1<bitmapsizes[i]) ) {
	    putshort(at->gaspf,bitmapsizes[i]-1);
	    putshort(at->gaspf,0x2);	/* Grey scale, no gridfitting */
	}
	if ( (i<nbitmaps-1 && bitmapsizes[i]+1<bitmapsizes[i+1]) ||
	      i==nbitmaps-1 ) {
	    putshort(at->gaspf,bitmapsizes[i]);
	    putshort(at->gaspf,0x0);	/* No grey scale, no gridfitting */
	}
    }
    gfree(bitmapsizes);
#endif
    putshort(at->gaspf,0xffff);	/* Upper bound on pixels/em for this range */
    putshort(at->gaspf,0x2);	/* Grey scale, no gridfitting */
				    /* No hints, so no grids to fit */
    at->gasplen = ftell(at->gaspf);
	/* This table is always 32 bit aligned */
}

#if 0
static void dumpmacstr(FILE *file,unichar_t *str) {
    int ch;
    unsigned char *table;

    do {
	ch = *str++;
	if ( ch==0 )
	    putc('\0',file);
	else if ( (ch>>8)>=mac_from_unicode.first && (ch>>8)<=mac_from_unicode.last &&
		(table = mac_from_unicode.table[(ch>>8)-mac_from_unicode.first])!=NULL &&
		table[ch&0xff]!=0 )
	    putc(table[ch&0xff],file);
	else
	    putc('?',file);	/* if we were to omit an unencoded char all our position calculations would be off */
    } while ( ch!='\0' );
}
#endif

static void dumpstr(FILE *file,char *str) {
    do {
	putc(*str,file);
    } while ( *str++!='\0' );
}

#if 0
static void dumpc2ustr(FILE *file,char *str) {
    do {
	putc('\0',file);
	putc(*str,file);
    } while ( *str++!='\0' );
}
#endif

static void dumpustr(FILE *file,char *utf8_str) {
    unichar_t *ustr = utf82u_copy(utf8_str), *pt=ustr;
    do {
	putc(*pt>>8,file);
	putc(*pt&0xff,file);
    } while ( *pt++!='\0' );
    free(ustr);
}

static void dumppstr(FILE *file,char *str) {
    putc(strlen(str),file);
    fwrite(str,sizeof(char),strlen(str),file);
}

char *utf8_verify_copy(const char *str) {
    /* When given a postscript string it SHOULD be in ASCII. But it will often*/
    /* contain a copyright symbol (sometimes in latin1, sometimes in macroman)*/
    /* unfortunately both encodings use 0xa9 for copyright so we can't distinguish */
    /* guess that it's latin1 (or that copyright is the only odd char which */
    /* means a latin1 conversion will work for macs too). */

    if ( str==NULL )
return( NULL );

    if ( utf8_valid(str))
return( copy(str));		/* Either in ASCII (good) or appears to be utf8*/
return( latin1_2_utf8_copy(str));
}

/* Oh. If the encoding is symbol (platform=3, specific=0) then Windows won't */
/*  accept the font unless the name table also has entries for (3,0). I'm not */
/*  sure if this is the case for the CJK encodings (docs don't mention that) */
/*  but let's do it just in case */
void DefaultTTFEnglishNames(struct ttflangname *dummy, SplineFont *sf) {
    time_t now;
    struct tm *tm;
    char buffer[200];

    if ( dummy->names[ttf_copyright]==NULL || *dummy->names[ttf_copyright]=='\0' )
	dummy->names[ttf_copyright] = utf8_verify_copy(sf->copyright);
    if ( dummy->names[ttf_family]==NULL || *dummy->names[ttf_family]=='\0' )
	dummy->names[ttf_family] = utf8_verify_copy(sf->familyname);
    if ( dummy->names[ttf_subfamily]==NULL || *dummy->names[ttf_subfamily]=='\0' )
	dummy->names[ttf_subfamily] = utf8_verify_copy(SFGetModifiers(sf));
    if ( dummy->names[ttf_uniqueid]==NULL || *dummy->names[ttf_uniqueid]=='\0' ) {
	time(&now);
	tm = localtime(&now);
	sprintf( buffer, "%s : %s : %d-%d-%d",
		BDFFoundry?BDFFoundry:TTFFoundry?TTFFoundry:"FontForge 1.0",
		sf->fullname!=NULL?sf->fullname:sf->fontname,
		tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900 );
	dummy->names[ttf_uniqueid] = copy(buffer);
    }
    if ( dummy->names[ttf_fullname]==NULL || *dummy->names[ttf_fullname]=='\0' )
	dummy->names[ttf_fullname] = utf8_verify_copy(sf->fullname);
    if ( dummy->names[ttf_version]==NULL || *dummy->names[ttf_version]=='\0' ) {
	if ( sf->subfontcnt!=0 )
	    sprintf( buffer, "Version %f ", sf->cidversion );
	else if ( sf->version!=NULL )
	    sprintf(buffer,"Version %.20s ", sf->version);
	else
	    strcpy(buffer,"Version 1.0" );
	dummy->names[ttf_version] = copy(buffer);
    }
    if ( dummy->names[ttf_postscriptname]==NULL || *dummy->names[ttf_postscriptname]=='\0' )
	dummy->names[ttf_postscriptname] = utf8_verify_copy(sf->fontname);
}

typedef struct {
    uint16 platform;
    uint16 specific;
    uint16 lang;
    uint16 strid;
    uint16 len;
    uint16 offset;
} NameEntry;

typedef struct {
    FILE *strings;
    int cur, max;
    enum fontformat format;
    Encoding *encoding_name;
    NameEntry *entries;
    int applemode;
} NamTab;

static int compare_entry(const void *_mn1, const void *_mn2) {
    const NameEntry *mn1 = _mn1, *mn2 = _mn2;

    if ( mn1->platform!=mn2->platform )
return( mn1->platform - mn2->platform );
    if ( mn1->specific!=mn2->specific )
return( mn1->specific - mn2->specific );
    if ( mn1->lang!=mn2->lang )
return( mn1->lang - mn2->lang );

return( mn1->strid-mn2->strid );
}

static void AddEncodedName(NamTab *nt,char *utf8name,uint16 lang,uint16 strid) {
    NameEntry *ne;
    int maclang, macenc= -1, specific;
    char *macname = NULL;

    if ( nt->cur+6>=nt->max ) {
	if ( nt->cur==0 )
	    nt->entries = galloc((nt->max=100)*sizeof(NameEntry));
	else
	    nt->entries = grealloc(nt->entries,(nt->max+=100)*sizeof(NameEntry));
    }

    ne = nt->entries + nt->cur;

    ne->platform = 3;		/* Windows */
    ne->specific = 1;		/* unicode */
    ne->lang     = lang;
    ne->strid    = strid;
    ne->offset   = ftell(nt->strings);
    ne->len      = 2*utf82u_strlen(utf8name);
    dumpustr(nt->strings,utf8name);
    ++ne;

    if ( nt->format==ff_ttfsym ) {
	*ne = ne[-1];
	ne->specific = 0;	/* Windows "symbol" */
	++ne;
    }

    maclang = WinLangToMac(lang);
    if ( !nt->applemode && lang!=0x409 )
	maclang = 0xffff;
    if ( maclang!=0xffff ) {
#ifdef FONTFORGE_CONFIG_APPLE_UNICODE_NAMES
	*ne = ne[-1];
	ne->platform = 0;	/* Mac unicode */
	ne->specific = 0;	/* 3 => Unicode 2.0 semantics */ /* 0 ("default") is also a reasonable value */
	ne->lang     = maclang;
	++ne;
#endif

	macenc = MacEncFromMacLang(maclang);
	macname = Utf8ToMacStr(utf8name,macenc,maclang);
	if ( macname!=NULL ) {
	    ne->platform = 1;		/* apple non-unicode encoding */
	    ne->specific = macenc;	/* whatever */
	    ne->lang     = maclang;
	    ne->strid    = strid;
	    ne->offset   = ftell(nt->strings);
	    ne->len      = strlen(macname);
	    dumpstr(nt->strings,macname);
	    ++ne;
	    free(macname);
	}
    }

    specific =  nt->encoding_name->is_korean ? 5 :	/* Wansung, korean */
		nt->encoding_name->is_japanese ? 2 :	/* SJIS */
		nt->encoding_name->is_simplechinese ? 3 :/* packed gb2312, don't know the real name */
		strmatch(nt->encoding_name->enc_name,"EUC-GB12345")==0 ? 3 :/* Lie */
		nt->encoding_name->is_tradchinese ? 4 :	/* Big5, traditional Chinese */
			-1;
    if ( specific != -1 ) {
	ne->platform = 3;		/* windows */
	ne->specific = specific;	/* whatever */
	ne->lang     = lang;
	ne->strid    = strid;
	if ( macname!=NULL &&
		(( specific== 2 && macenc==1 ) ||	/* Japanese */
		 ( specific== 3 && macenc==25 ) ||	/* simplified chinese */
		 ( specific== 4 && macenc==2 ) ||	/* traditional chinese */
		 ( specific== 5 && macenc==3 )) ) {	/* wansung korean */
	    ne->offset = ne[-1].offset;
	    ne->len    = ne[-1].len;
	} else {
	    char *space, *encname, *out;
	    ICONV_CONST char *in;
	    Encoding *enc;
	    size_t inlen, outlen;
	    ne->offset  = ftell(nt->strings);
	    encname     = nt->encoding_name->is_japanese ? "SJIS" :
			strmatch(nt->encoding_name->enc_name,"JOHAB")==0 ? "JOHAB" :
			nt->encoding_name->is_korean ? "EUC-KR" :
			nt->encoding_name->is_simplechinese ? "EUC-CN" :
			    nt->encoding_name->enc_name;
	    enc = FindOrMakeEncoding(encname);
	    if ( enc==NULL )
		--ne;
	    else {
		unichar_t *uin = utf82u_copy(utf8name);
		outlen = 3*strlen(utf8name)+10;
		out = space = galloc(outlen+2);
		in = (char *) uin; inlen = 2*u_strlen(uin);
		iconv(enc->fromunicode,NULL,NULL,NULL,NULL);	/* should not be needed, but just in case */
		iconv(enc->fromunicode,&in,&inlen,&out,&outlen);
		out[0] = '\0'; out[1] = '\0';
		ne->offset = ftell(nt->strings);
		ne->len    = strlen(space);
		dumpstr(nt->strings,space);
		free(space); free(uin);
	    }
	}
	++ne;
    }
    nt->cur = ne - nt->entries;
}

static void AddMacName(NamTab *nt,struct macname *mn, int strid) {
    NameEntry *ne;

    if ( nt->cur+1>=nt->max ) {
	if ( nt->cur==0 )
	    nt->entries = galloc((nt->max=100)*sizeof(NameEntry));
	else
	    nt->entries = grealloc(nt->entries,(nt->max+=100)*sizeof(NameEntry));
    }

    ne = nt->entries + nt->cur;

    ne->platform = 1;		/* apple non-unicode encoding */
    ne->specific = mn->enc;	/* whatever */
    ne->lang     = mn->lang;
    ne->strid    = strid;
    ne->offset   = ftell(nt->strings);
    ne->len      = strlen(mn->name);
    dumpstr(nt->strings,mn->name);

    ++nt->cur;
}

/* There's an inconsistancy here. Apple's docs say there most be only one */
/*  nameid==6 and that name must be ascii (presumably plat=1, spec=0, lang=0) */
/* The opentype docs say there must be two (psl=1,0,0 & psl=3,1,0x409) any */
/*  others are to be ignored */
/* A representative from Apple says they will change their spec to accept */
/*  the opentype version, and tells me that they don't currently care */
/* So ignore this */
/* Undocumented fact: Windows insists on having a UniqueID string 3,1 */
static void dumpnames(struct alltabs *at, SplineFont *sf,enum fontformat format) {
    int i,j;
    struct ttflangname dummy, *cur, *useng = NULL;
    struct macname *mn;
    struct other_names *on, *onn;
    NamTab nt;
    struct otfname *otfn;

    memset(&nt,0,sizeof(nt));
    nt.encoding_name = at->map->enc;
    nt.format	     = format;
    nt.applemode     = at->applemode;
    nt.strings	     = tmpfile();

    memset(&dummy,0,sizeof(dummy));
    for ( cur=sf->names; cur!=NULL; cur=cur->next ) {
	if ( cur->lang==0x409 ) {
	    dummy = *cur;
	    useng = cur;
    break;
	}
    }
    DefaultTTFEnglishNames(&dummy, sf);

    for ( i=0; i<ttf_namemax; ++i ) if ( dummy.names[i]!=NULL )
	AddEncodedName(&nt,dummy.names[i],0x409,i);
    for ( cur=sf->names; cur!=NULL; cur=cur->next ) {
	if ( cur->lang!=0x409 )
	    for ( i=0; i<ttf_namemax; ++i )
		if ( cur->names[i]!=NULL )
		    AddEncodedName(&nt,cur->names[i],cur->lang,i);
    }

    /* The examples I've seen of the feature table only contain platform==mac */
    /*  so I'm not including apple unicode */
    if ( at->feat_name!=NULL ) {
	for ( i=0; at->feat_name[i].strid!=0; ++i ) {
	    for ( mn=at->feat_name[i].mn; mn!=NULL; mn=mn->next )
		AddMacName(&nt,mn,at->feat_name[i].strid);
#if 0	/* I'm not sure why I keep track of these alternates */
	/*  Dumping them out like this is a bad idea. It might be worth */
	/*  something if we searched through the alternate sets for languages */
	/*  not found in the main set, but at the moment I don't think so */
	/*  What happens now is that I get duplicate names output */
	    for ( mn=at->feat_name[i].smn; mn!=NULL; mn=mn->next )
		AddMacName(&nt,mn,at->feat_name[i].strid);
#endif
	}
    }
    /* And the names used by the fvar table aren't mac unicode either */
    for ( on = at->other_names; on!=NULL; on=onn ) {
	for ( mn = on->mn; mn!=NULL ; mn = mn->next )
	    AddMacName(&nt,mn,on->strid);
	onn = on->next;
	chunkfree(on,sizeof(*on));
    }
    /* Wow, the GPOS 'size' feature uses the name table in a very mac-like way*/
    if ( at->fontstyle_name_strid!=0 && sf->fontstyle_name!=NULL ) {
	for ( otfn = sf->fontstyle_name; otfn!=NULL; otfn = otfn->next )
	    AddEncodedName(&nt,otfn->name,otfn->lang,at->fontstyle_name_strid);
    }

    qsort(nt.entries,nt.cur,sizeof(NameEntry),compare_entry);

    at->name = tmpfile();
    putshort(at->name,0);				/* format */
    putshort(at->name,nt.cur);				/* numrec */
    putshort(at->name,(3+nt.cur*6)*sizeof(int16));	/* offset to strings */

    for ( i=0; i<nt.cur; ++i ) {
	putshort(at->name,nt.entries[i].platform);
	putshort(at->name,nt.entries[i].specific);
	putshort(at->name,nt.entries[i].lang);
	putshort(at->name,nt.entries[i].strid);
	putshort(at->name,nt.entries[i].len);
	putshort(at->name,nt.entries[i].offset);
    }
    if ( !ttfcopyfile(at->name,nt.strings,(3+nt.cur*6)*sizeof(int16),"name-data"))
	at->error = true;

    at->namelen = ftell(at->name);
    if ( (at->namelen&3)!=0 )
	for ( j= 4-(at->namelen&3); j>0; --j )
	    putc('\0',at->name);

    for ( i=0; i<ttf_namemax; ++i )
	if ( useng==NULL || dummy.names[i]!=useng->names[i] )
	    free( dummy.names[i]);
    free( nt.entries );
    free( at->feat_name );
}

static void dumppost(struct alltabs *at, SplineFont *sf, enum fontformat format) {
    int pos, i,j, shouldbe;
    int shorttable = (format==ff_otf || format==ff_otfcid ||
	    (at->gi.flags&ttf_flag_shortps));
    uint32 here;

    at->post = tmpfile();

    putlong(at->post,shorttable?0x00030000:0x00020000);	/* formattype */
    putfixed(at->post,sf->italicangle);
    putshort(at->post,sf->upos);
    putshort(at->post,sf->uwidth);
    putlong(at->post,at->isfixed);
    putlong(at->post,0);		/* no idea about memory */
    putlong(at->post,0);		/* no idea about memory */
    putlong(at->post,0);		/* no idea about memory */
    putlong(at->post,0);		/* no idea about memory */
    if ( !shorttable ) {
	here = ftell(at->post);
	putshort(at->post,at->maxp.numGlyphs);

	shouldbe = 0;
	for ( i=0, pos=0; i<at->gi.gcnt; ++i ) {
	    if ( at->gi.bygid[i]!=-1 && sf->glyphs[at->gi.bygid[i]]!=NULL ) {
		SplineChar *sc = sf->glyphs[at->gi.bygid[i]];
		while ( i>shouldbe ) {
		    if ( shouldbe==0 )
			putshort(at->post,0);		/* glyph 0 is named .notdef */
		    else if ( shouldbe==1 )
			putshort(at->post,1);		/* glyphs 1&2 are .null and cr */
		    else if ( shouldbe==2 )
			putshort(at->post,2);		/* or something */
		    else
			putshort(at->post,0);
		    ++shouldbe;
		}
		if ( strcmp(sc->name,".notdef")==0 )
		    putshort(at->post,0);
		else {
		    for ( j=0; j<258; ++j )
			if ( strcmp(sc->name,ttfstandardnames[j])==0 )
		    break;
		    if ( j!=258 )
			putshort(at->post,j);
		    else {
			putshort(at->post,pos+258);
			++pos;
		    }
		}
		++shouldbe;
	    }
	}

	if ( shouldbe!=at->maxp.numGlyphs ) {
	    fseek(at->post,here,SEEK_SET);
	    putshort(at->post,shouldbe);
	    fseek(at->post,0,SEEK_END);
	}
	if ( pos!=0 ) {
	    for ( i=0; i<at->gi.gcnt; ++i ) if ( at->gi.bygid[i]!=-1 ) {
		SplineChar *sc = sf->glyphs[at->gi.bygid[i]];
		if ( strcmp(sc->name,".notdef")==0 )
		    /* Do Nothing */;
		else {
		    for ( j=0; j<258; ++j )
			if ( strcmp(sc->name,ttfstandardnames[j])==0 )
		    break;
		    if ( j!=258 )
			/* Do Nothing */;
		    else
			dumppstr(at->post,sc->name);
		}
	    }
	}
    }
    at->postlen = ftell(at->post);
    if ( (at->postlen&3)!=0 )
	for ( j= 4-(at->postlen&3); j>0; --j )
	    putc('\0',at->post);
}

static FILE *_Gen816Enc(SplineFont *sf,int *tlen,EncMap *map) {
    int i, j, complained, pos, k, subheadindex, jj, isbig5=false;
    uint16 table[256];
    struct subhead subheads[128];
    uint16 *glyphs;
    uint16 tempglyphs[256];
    int base, lbase, basebound, subheadcnt, planesize, plane0size;
    int base2, base2bound;
    FILE *sub;
    char *encname = map->enc->iconv_name!=NULL ? map->enc->iconv_name : map->enc->enc_name;

    *tlen = 0;

    base2 = -1; base2bound = -2;
    if ( map->enc->is_tradchinese && strstrmatch(encname,"hkscs")!=NULL ) {
	base = 0x81;
	basebound = 0xfe;
	subheadcnt = basebound-base+1;
	lbase = 0x40;
	planesize = 191;
    } else if ( map->enc->is_tradchinese || sf->uni_interp) {
	base = 0xa1;
	basebound = 0xf9;	/* wcl-02.ttf's cmap claims to go up to fc, but everything after f9 is invalid (according to what I know of big5, f9 should be the end) */
	subheadcnt = basebound-base+1;
	lbase = 0x40;
	planesize = 191;
	isbig5 = true;
    } else if ( strstrmatch(encname,"euc")!=NULL ) {
	base = 0xa1;
	basebound = 0xfd;
	lbase = 0xa1;
	subheadcnt = basebound-base+1;
	planesize = 0xfe - lbase +1;
    } else if ( strstrmatch(encname,"johab")!=NULL ) {
	base = 0x84;
	basebound = 0xf9;
	lbase = 0x31;
	subheadcnt = basebound-base+1;
	planesize = 0xfe -0x31+1;	/* Stupid gcc bug, thinks 0xfe- is ambiguous (exponant) */
    } else if ( strstrmatch(encname,"sjis")!=NULL ) {
	base = 129;
	basebound = 159;
	lbase = 64;
	planesize = 252 - lbase +1;
	base2 = 0xe0;
	/* SJIS supports "user defined characters" between 0xf040 and 0xfcfc */
	/*  there probably won't be any, but allow space for them if there are*/
	for ( base2bound=0xfc00; base2bound>0xefff; --base2bound )
	    if ( base2bound<map->enccount && map->map[base2bound]!=-1 &&
		    SCWorthOutputting(sf->glyphs[map->map[base2bound]]))
	break;
	base2bound >>= 8;
	subheadcnt = basebound-base + 1 + base2bound-base2 + 1;
    } else {
	IError( "Unsupported 8/16 encoding %s\n", map->enc->enc_name );
return( NULL );
    }
    plane0size = base2==-1? base : base2;
    i=0;
    if ( base2!=-1 ) {
	for ( i=basebound; i<base2 && i<map->enccount; ++i )
	    if ( map->map[i]==-1 )
	continue;
	    else if ( SCWorthOutputting(sf->glyphs[map->map[i]]))
	break;
	if ( i==base2 || i==map->enccount )
	    i = 0;
    }
    if ( i==0 ) {
	for ( i=0; i<base && i<map->enccount; ++i )
	    if ( map->map[i]==-1 )
	continue;
	    else if ( SCWorthOutputting(sf->glyphs[map->map[i]]))
	break;
    }
#if 0
    if ( i==base || i==sf->glyphcnt )
return( NULL );		/* Doesn't have the single byte entries */
	/* Can use the normal 16 bit encoding scheme */
	/* Always do an 8/16, and we'll do a unicode table too now... */
#endif

    if ( base2!=-1 ) {
	for ( i=base; i<=basebound && i<map->enccount; ++i )
	    if ( map->map[i]!=-1 && SCWorthOutputting(sf->glyphs[map->map[i]])) {
		gwwv_post_error(_("Bad Encoding"),_("There is a single byte character (%d) using one of the slots needed for double byte characters"),i);
	break;
	    }
	if ( i==basebound+1 )
	    for ( i=base2; i<256 && i<map->enccount; ++i )
		if ( map->map[i]!=-1 && SCWorthOutputting(sf->glyphs[map->map[i]])) {
		    gwwv_post_error(_("Bad Encoding"),_("There is a single byte character (%d) using one of the slots needed for double byte characters"),i);
	    break;
		}
    } else {
	for ( i=base; i<=256 && i<map->enccount; ++i )
	    if ( map->map[i]!=-1 && SCWorthOutputting(sf->glyphs[map->map[i]])) {
		gwwv_post_error(_("Bad Encoding"),_("There is a single byte character (%d) using one of the slots needed for double byte characters"),i);
	break;
	    }
    }
    for ( i=256; i<(base<<8) && i<map->enccount; ++i )
	if ( map->map[i]!=-1 && SCWorthOutputting(sf->glyphs[map->map[i]])) {
	    gwwv_post_error(_("Bad Encoding"),_("There is a character (%d) which cannot be encoded"),i);
    break;
	}
    if ( i==(base<<8) && base2==-1 )
	for ( i=((basebound+1)<<8); i<0x10000 && i<map->enccount; ++i )
	    if ( map->map[i]!=-1 && SCWorthOutputting(sf->glyphs[map->map[i]])) {
		gwwv_post_error(_("Bad Encoding"),_("There is a character (%d) which cannot be encoded"),i);
	break;
	    }

    memset(table,'\0',sizeof(table));
    for ( i=base; i<=basebound; ++i )
	table[i] = 8*(i-base+1);
    for ( i=base2; i<=base2bound; ++i )
	table[i] = 8*(i-base2+basebound-base+1+1);
    memset(subheads,'\0',sizeof(subheads));
    subheads[0].first = 0; subheads[0].cnt = plane0size;
    for ( i=1; i<subheadcnt+1; ++i ) {
	subheads[i].first = lbase;
	subheads[i].cnt = planesize;
    }
    glyphs = gcalloc(subheadcnt*planesize+plane0size,sizeof(uint16));
    subheads[0].rangeoff = 0;
    for ( i=0; i<plane0size && i<map->enccount; ++i )
	if ( map->map[i]!=-1 && sf->glyphs[map->map[i]]!=NULL &&
		sf->glyphs[map->map[i]]->ttf_glyph!=-1 )
	    glyphs[i] = sf->glyphs[map->map[i]]->ttf_glyph;
    
    pos = 1;

    complained = false;
    subheadindex = 1;
    for ( jj=0; jj<2 || (base2==-1 && jj<1); ++jj )
	    for ( j=((jj==0?base:base2)<<8); j<=((jj==0?basebound:base2bound)<<8); j+= 0x100 ) {
	for ( i=0; i<lbase; ++i )
	    if ( !complained && map->map[i+j]!=-1 &&
		    SCWorthOutputting(sf->glyphs[map->map[i+j]])) {
		gwwv_post_error(_("Bad Encoding"),_("There is a character (%d) which is not normally in the encoding"),i+j);
		complained = true;
	    }
	if ( isbig5 ) {
	    /* big5 has a gap here. Does johab? */
	    for ( i=0x7f; i<0xa1; ++i )
		if ( !complained && map->map[i+j]!=-1 &&
			SCWorthOutputting(sf->glyphs[map->map[i+j]])) {
		    gwwv_post_error(_("Bad Encoding"),_("There is a character (%d) which is not normally in the encoding"),i+j);
		    complained = true;
		}
	}
	memset(tempglyphs,0,sizeof(tempglyphs));
	for ( i=0; i<planesize; ++i )
	    if ( map->map[j+lbase+i]!=-1 && sf->glyphs[map->map[j+lbase+i]]!=NULL &&
		    sf->glyphs[map->map[j+lbase+i]]->ttf_glyph!=-1 )
		tempglyphs[i] = sf->glyphs[map->map[j+lbase+i]]->ttf_glyph;
	for ( i=1; i<pos; ++i ) {
	    int delta = 0;
	    for ( k=0; k<planesize; ++k )
		if ( tempglyphs[k]==0 && glyphs[plane0size+(i-1)*planesize+k]==0 )
		    /* Still matches */;
		else if ( tempglyphs[k]==0 || glyphs[plane0size+(i-1)*planesize+k]==0 )
	    break;  /* Doesn't match */
		else if ( delta==0 )
		    delta = (uint16) (tempglyphs[k]-glyphs[plane0size+(i-1)*planesize+k]);
		else if ( tempglyphs[k]==(uint16) (glyphs[plane0size+(i-1)*planesize+k]+delta) )
		    /* Still matches */;
		else
	    break;
	    if ( k==planesize ) {
		subheads[subheadindex].delta = delta;
		subheads[subheadindex].rangeoff = plane0size+(i-1)*planesize;
	break;
	    }
	}
	if ( subheads[subheadindex].rangeoff==0 ) {
	    memcpy(glyphs+(pos-1)*planesize+plane0size,tempglyphs,planesize*sizeof(uint16));
	    subheads[subheadindex].rangeoff = plane0size+(pos++-1)*planesize ;
	}
	++subheadindex;
    }

    /* fixup offsets */
    /* my rangeoffsets are indexes into the glyph array. That's nice and */
    /*  simple. Unfortunately ttf says they are offsets from the current */
    /*  location in the file (sort of) so we now fix them up. */
    for ( i=0; i<subheadcnt+1; ++i )
	subheads[i].rangeoff = subheads[i].rangeoff*sizeof(uint16) +
		(subheadcnt-i)*sizeof(struct subhead) + sizeof(uint16);

    sub = tmpfile();
    if ( sub==NULL )
return( NULL );

    putshort(sub,2);		/* 8/16 format */
    putshort(sub,0);		/* Subtable length, we'll come back and fix this */
    putshort(sub,0);		/* version/language, not meaningful in ms systems */
    for ( i=0; i<256; ++i )
	putshort(sub,table[i]);
    for ( i=0; i<subheadcnt+1; ++i ) {
	putshort(sub,subheads[i].first);
	putshort(sub,subheads[i].cnt);
	putshort(sub,subheads[i].delta);
	putshort(sub,subheads[i].rangeoff);
    }
    for ( i=0; i<(pos-1)*planesize+plane0size; ++i )
	putshort(sub,glyphs[i]);
    free(glyphs);

    *tlen = ftell(sub);
    fseek(sub,2,SEEK_SET);
    putshort(sub,*tlen);	/* Length, I said we'd come back to it */
    rewind( sub );
return( sub );
}

static FILE *Needs816Enc(SplineFont *sf,int *tlen,EncMap *map, FILE **apple, int *appletlen) {
    FILE *sub;
    char *encname = map->enc->iconv_name!=NULL ? map->enc->iconv_name : map->enc->enc_name;
    EncMap *oldmap = map;
    EncMap *applemap = NULL;
    Encoding *enc;

    *tlen = 0;
    if ( apple!=NULL ) {
	*apple = NULL;
	*appletlen = 0;
    }
    if ( sf->cidmaster!=NULL || sf->subfontcnt!=0 )
return( NULL );
    if ( (strstrmatch(encname,"big")!=NULL && strchr(encname,'5')!=NULL) ||
	    strstrmatch(encname,"johab")!=NULL ||
	    strstrmatch(encname,"sjis")!=NULL ||
	    strstrmatch(encname,"cp932")!=NULL ||
	    strstrmatch(encname,"euc-kr")!=NULL ||
	    strstrmatch(encname,"euc-cn")!=NULL )
	/* Already properly encoded */;
    else if ( strstrmatch(encname,"2022")!=NULL &&
	    (strstrmatch(encname,"JP2")!=NULL ||
	     strstrmatch(encname,"JP-2")!=NULL ||
	     strstrmatch(encname,"JP-3")!=NULL ))
return( NULL );		/* No 8/16 encoding for JP2 nor JP3 */
    else if ( sf->uni_interp>=ui_japanese && sf->uni_interp<=ui_korean ) {
	enc = FindOrMakeEncoding(
		sf->uni_interp==ui_japanese ? "sjis" :
		sf->uni_interp==ui_trad_chinese ? "big5" :
		sf->uni_interp==ui_simp_chinese ? "euc-cn" :
		    "euc-kr");
	if ( map->enc!=enc ) {
	    map = EncMapFromEncoding(sf,enc);
	    encname = map->enc->iconv_name!=NULL ? map->enc->iconv_name : map->enc->enc_name;
	}
    } else
return( NULL );

    /* Both MS and Apple extend sjis. I don't know how to get iconv to give me*/
    /*  apple's encoding though. So I generate one 8/16 table for MS based on */
    /*  their extension (cp932), and another table based on plain sjis for Apple*/
    /* Don't know if this is true of other cjk encodings... for the moment I */
    /*  will just use standard encodings for them */
    if ( strstrmatch(encname,"sjis")!=NULL ) {
	enc = FindOrMakeEncoding("cp932");
	if ( enc!=NULL ) {
	    applemap = map;
	    map = EncMapFromEncoding(sf,enc);
	}
    } else if ( strstrmatch(encname,"cp932")!=NULL )
	applemap = EncMapFromEncoding(sf,FindOrMakeEncoding("sjis"));

    if ( applemap!=NULL )
	*apple = _Gen816Enc(sf,appletlen,applemap);
    sub = _Gen816Enc(sf,tlen,map);

    if ( applemap!=NULL && applemap!=oldmap )
	EncMapFree(applemap);
    if ( map!=oldmap )
	EncMapFree(map);
return( sub );
}

static FILE *NeedsUCS4Table(SplineFont *sf,int *ucs4len,EncMap *map) {
    int i=0,j,group;
    FILE *format12;
    SplineChar *sc;
    EncMap *freeme = NULL;
    struct altuni *altuni;

    if ( map->enc->is_unicodefull )
	i=0x10000;
    else if ( map->enc->is_custom )
	i = 0;
    else
	i = map->enc->char_cnt;
    for ( ; i<map->enccount; ++i ) {
	if ( map->map[i]!=-1 && SCWorthOutputting(sf->glyphs[map->map[i]]) ) {
	    if ( sf->glyphs[map->map[i]]->unicodeenc>=0x10000 )
    break;
	    for ( altuni=sf->glyphs[map->map[i]]->altuni; altuni!=NULL && altuni->unienc<0x10000;
		    altuni=altuni->next );
	    if ( altuni!=NULL )
    break;
	}
    }

    if ( i>=map->enccount )
return(NULL);

    if ( !map->enc->is_unicodefull )
	map = freeme = EncMapFromEncoding(sf,FindOrMakeEncoding("ucs4"));

    format12 = tmpfile();
    if ( format12==NULL )
return( NULL );

    putshort(format12,12);		/* Subtable format */
    putshort(format12,0);		/* padding */
    putlong(format12,0);		/* Length, we'll come back to this */
    putlong(format12,0);		/* language */
    putlong(format12,0);		/* Number of groups, we'll come back to this */

    group = 0;
    for ( i=0; i<map->enccount; ++i ) if ( map->map[i]!=-1 && SCWorthOutputting(sf->glyphs[map->map[i]]) && sf->glyphs[map->map[i]]->unicodeenc!=-1 ) {
	sc = sf->glyphs[map->map[i]];
	for ( j=i+1; j<map->enccount && map->map[j]!=-1 &&
		SCWorthOutputting(sf->glyphs[map->map[j]]) &&
		sf->glyphs[map->map[j]]->unicodeenc!=-1 &&
		sf->glyphs[map->map[j]]->ttf_glyph==sc->ttf_glyph+j-i; ++j );
	--j;
	putlong(format12,i);		/* start char code */
	putlong(format12,j);		/* end char code */
	putlong(format12,sc->ttf_glyph);
	++group;
	i=j;				/* move to the start of the next group */
    }
    *ucs4len = ftell(format12);
    fseek(format12,4,SEEK_SET);
    putlong(format12,*ucs4len);		/* Length, I said we'd come back to it */
    putlong(format12,0);		/* language */
    putlong(format12,group);		/* Number of groups */
    rewind( format12 );

    if ( freeme!=NULL )
	EncMapFree(freeme);
return( format12 );
}

static FILE *NeedsUCS2Table(SplineFont *sf,int *ucs2len,EncMap *map) {
    /* We always want a format 4 2byte unicode encoding map */
    uint32 *avail = galloc(65536*sizeof(uint32));
    int i,j,l;
    int segcnt, cnt=0, delta, rpos;
    struct cmapseg { uint16 start, end; uint16 delta; uint16 rangeoff; } *cmapseg;
    uint16 *ranges;
    SplineChar *sc;
    FILE *format4 = tmpfile();

    memset(avail,0xff,65536*sizeof(uint32));
    if ( map->enc->is_unicodebmp || map->enc->is_unicodefull ) { int gid;
	for ( i=0; i<65536 && i<map->enccount; ++i ) if ( (gid=map->map[i])!=-1 && sf->glyphs[gid]!=NULL && sf->glyphs[gid]->ttf_glyph!=-1 ) {
	    avail[i] = gid;
	    ++cnt;
	}
    } else {
	struct altuni *altuni;
	for ( i=0; i<sf->glyphcnt; ++i ) {
	    if ( (sc=sf->glyphs[i])!=NULL && sc->ttf_glyph!=-1 ) {
		if ( sc->unicodeenc>=0 && sc->unicodeenc<=0xffff ) {
		    avail[sc->unicodeenc] = i;
		    ++cnt;
		}
		for ( altuni=sc->altuni; altuni!=NULL; altuni = altuni->next ) {
		    if ( altuni->unienc>=0 && altuni->unienc<=0xffff ) {
			avail[altuni->unienc] = i;
			++cnt;
		    }
		}
	    }
	}
    }

    j = -1;
    for ( i=segcnt=0; i<65536; ++i ) {
	if ( avail[i]!=0xffffffff && j==-1 ) {
	    j=i;
	    ++segcnt;
	} else if ( j!=-1 && avail[i]==0xffffffff )
	    j = -1;
    }
    cmapseg = gcalloc(segcnt+1,sizeof(struct cmapseg));
    ranges = galloc(cnt*sizeof(int16));
    j = -1;
    for ( i=segcnt=0; i<65536; ++i ) {
	if ( avail[i]!=0xffffffff && j==-1 ) {
	    j=i;
	    cmapseg[segcnt].start = j;
	    ++segcnt;
	} else if ( j!=-1 && avail[i]==0xffffffff ) {
	    cmapseg[segcnt-1].end = i-1;
	    j = -1;
	}
    }
    if ( j!=-1 )
	cmapseg[segcnt-1].end = i-1;
    /* create a dummy segment to mark the end of the table */
    cmapseg[segcnt].start = cmapseg[segcnt].end = 0xffff;
    cmapseg[segcnt++].delta = 1;
    rpos = 0;
    for ( i=0; i<segcnt-1; ++i ) {
	l = avail[cmapseg[i].start];
	sc = sf->glyphs[l];
	delta = sc->ttf_glyph-cmapseg[i].start;
	for ( j=cmapseg[i].start; j<=cmapseg[i].end; ++j ) {
	    l = avail[j];
	    sc = sf->glyphs[l];
	    if ( delta != sc->ttf_glyph-j )
	break;
	}
	if ( j>cmapseg[i].end )
	    cmapseg[i].delta = delta;
	else {
	    cmapseg[i].rangeoff = (rpos + (segcnt-i)) * sizeof(int16);
	    for ( j=cmapseg[i].start; j<=cmapseg[i].end; ++j ) {
		l = avail[j];
		sc = sf->glyphs[l];
		ranges[rpos++] = sc->ttf_glyph;
	    }
	}
    }
    free(avail);


    putshort(format4,4);		/* format */
    putshort(format4,(8+4*segcnt+rpos)*sizeof(int16));
    putshort(format4,0);		/* language/version */
    putshort(format4,2*segcnt);	/* segcnt */
    for ( j=0,i=1; i<=segcnt; i<<=1, ++j );
    putshort(format4,i);		/* 2*2^floor(log2(segcnt)) */
    putshort(format4,j-1);
    putshort(format4,2*segcnt-i);
    for ( i=0; i<segcnt; ++i )
	putshort(format4,cmapseg[i].end);
    putshort(format4,0);
    for ( i=0; i<segcnt; ++i )
	putshort(format4,cmapseg[i].start);
    for ( i=0; i<segcnt; ++i )
	putshort(format4,cmapseg[i].delta);
    for ( i=0; i<segcnt; ++i )
	putshort(format4,cmapseg[i].rangeoff);
    for ( i=0; i<rpos; ++i )
	putshort(format4,ranges[i]);
    free(ranges);
    free(cmapseg);
    *ucs2len = ftell(format4);
return( format4 );
}

static void dumpcmap(struct alltabs *at, SplineFont *sf,enum fontformat format) {
    int i,enccnt, issmall, hasmac;
    uint16 table[256];
    SplineChar *sc;
    int alreadyprivate = false;
    int anyglyphs = 0;
    int wasotf = format==ff_otf || format==ff_otfcid;
    EncMap *map = at->map;
    int ucs4len=0, ucs2len=0, cjklen=0, applecjklen=0;
    FILE *format12, *format4, *format2, *apple2;
    int mspos, ucs4pos, cjkpos, applecjkpos;

    for ( i=at->gi.gcnt-1; i>0 ; --i ) if ( at->gi.bygid[i]!=-1 ) {
	if ( SCWorthOutputting(sf->glyphs[at->gi.bygid[i]])) {
	    anyglyphs = true;
	    if ( sf->glyphs[at->gi.bygid[i]]->unicodeenc!=-1 )
    break;
	}
    }
    if ( sf->subfontcnt==0 && !anyglyphs ) {
	gwwv_post_error(_("No Encoded Glyphs"),_("Warning: Font contained no glyphs"));
    }
    if ( sf->subfontcnt==0 && format!=ff_ttfsym) {
	if ( i==0 && anyglyphs ) {
#ifndef FONTFORGE_CONFIG_NO_WINDOWING_UI
	    if ( map->enccount<=256 ) {
#if defined(FONTFORGE_CONFIG_GDRAW)
		char *buts[3];
		buts[0] = _("_Yes"); buts[1] = _("_No"); buts[2] = NULL;
#else
		static char *buts[] = { GTK_STOCK_YES, GTK_STOCK_NO, NULL };
#endif
		if ( gwwv_ask(_("No Encoded Glyphs"),(const char **) buts,0,1,_("This font contains no glyphs with unicode encodings.\nWould you like to use a \"Symbol\" encoding instead of Unicode?"))==0 )
		    format = ff_ttfsym;
	    } else
#endif		/* FONTFORGE_CONFIG_NO_WINDOWING_UI */
		gwwv_post_error(_("No Encoded Glyphs"),_("This font contains no glyphs with unicode encodings.\nYou will probably not be able to use the output."));
	}
    }

    at->cmap = tmpfile();

    /* MacRoman encoding table */ /* Not going to bother with making this work for cid fonts */
    /* I now see that Apple doesn't restrict us to format 0 sub-tables (as */
    /*  the docs imply) but instead also uses format 6 tables. Wildly in- */
    /*  appropriate as they are for 2byte encodings, but Apple uses them */
    /*  for one byte ones too */
    memset(table,'\0',sizeof(table));
    if ( !wasotf ) {
	table[29] = table[8] = table[0] = 1;
	table[9] = table[13] = 2;
    }
    for ( i=0; i<256 ; ++i ) {
	extern unichar_t MacRomanEnc[];
	sc = SFFindExistingCharMac(sf,map,MacRomanEnc[i]);
	if ( sc!=NULL && sc->ttf_glyph!=-1 )
	    table[i] = sc->ttf_glyph;
    }
    if ( table[0]==0 ) table[0] = 1;

    if ( format==ff_ttfsym ) {
	int acnt=0, pcnt=0;
	for ( i=0; i<map->enccount && i<0xffff; ++i ) {
	    if ( map->map[i]!=-1 && sf->glyphs[map->map[i]]!=NULL &&
		    sf->glyphs[map->map[i]]->ttf_glyph!=-1 ) {
		if ( i>=0xf000 && i<=0xf0ff )
		    ++acnt;
		else if ( i>=0x20 && i<=0xff )
		    ++pcnt;
	    }
	}
	alreadyprivate = acnt>pcnt;
	memset(table,'\0',sizeof(table));
	if ( !wasotf ) {
	    table[29] = table[8] = table[0] = 1;
	    table[9] = table[13] = 2;
	}
	if ( !alreadyprivate ) {
	    for ( i=0; i<map->enccount && i<256; ++i ) {
		if ( map->map[i]!=-1 && (sc = sf->glyphs[map->map[i]])!=NULL &&
			sc->ttf_glyph!=-1 )
		    table[i] = sc->ttf_glyph;
	    }
	} else {
	    for ( i=0xf000; i<=0xf0ff && i<sf->glyphcnt; ++i ) {
		if ( map->map[i]!=-1 && (sc = sf->glyphs[map->map[i]])!=NULL &&
			sc->ttf_glyph!=-1 )
		    table[i-0xf000] = sc->ttf_glyph;
	    }
	}
	/* if the user has read in a ttf symbol file then it will already have */
	/*  the right private use encoding, and we don't want to mess it up. */
	/*  The alreadyprivate flag should detect this case */
	if ( !alreadyprivate ) {
	    for ( i=0; i<map->enccount && i<256; ++i ) {
		if ( map->map[i]!=-1 && (sc = sf->glyphs[map->map[i]])!=NULL ) {
		    sc->orig_pos = sc->unicodeenc;
		    sc->unicodeenc = 0xf000 + i;
		}
	    }
	    for ( ; i<map->enccount; ++i ) {
		if ( map->map[i]!=-1 && (sc = sf->glyphs[map->map[i]])!=NULL ) {
		    sc->orig_pos = sc->unicodeenc;
		    sc->unicodeenc = -1;
		}
	    }
	}
    }

    format4  = NeedsUCS2Table(sf,&ucs2len,map);
    format12 = NeedsUCS4Table(sf,&ucs4len,map);
    format2  = Needs816Enc(sf,&cjklen,map,&apple2,&applecjklen);

    /* Two/Three/Four encoding table pointers, one for ms, one for mac */
    /*  usually one for mac big, just a copy of ms */
    /* plus we may have a format12 encoding for ucs4, mac doesn't support */
    /* plus we may have a format2 encoding for cjk, sometimes I know the codes for the mac... */
    /* sometimes the mac will have a slightly different cjk table */
    if ( format==ff_ttfsym ) {
	enccnt = 2;
	hasmac = 0;
    } else {
	hasmac = 1;
	enccnt = 3;
	if ( format12!=NULL )
	    enccnt = 4;
	if ( format2!=NULL ) {
	    if ( strstrmatch(map->enc->enc_name,"johab")!=NULL ) {
		++enccnt;
	    } else {
		enccnt+=2;
		hasmac=3;
	    }
	}
    }

    putshort(at->cmap,0);		/* version */
    putshort(at->cmap,enccnt);	/* num tables */

    mspos = 2*sizeof(uint16)+enccnt*(2*sizeof(uint16)+sizeof(uint32));
    ucs4pos = mspos+ucs2len;
    cjkpos = ucs4pos+ucs4len;
    if ( apple2==NULL ) {
	applecjkpos = cjkpos;
	applecjklen = cjklen;
    } else
	applecjkpos = cjkpos + cjklen;
    if ( hasmac&1 ) {
	/* big mac table, just a copy of the ms table */
	putshort(at->cmap,0);	/* mac unicode platform */
	putshort(at->cmap,3);	/* Unicode 2.0 */
	putlong(at->cmap,mspos);
    }
    putshort(at->cmap,1);		/* mac platform */
    putshort(at->cmap,0);		/* plat specific enc, script=roman */
	/* Even the symbol font on the mac claims a mac roman encoding */
	/* although it actually contains a symbol encoding. There is an*/
	/* "RSymbol" language listed for Mac (specific=8) but it isn't used*/
    putlong(at->cmap,applecjkpos+applecjklen);	/* offset from tab start to sub tab start */
    if ( format2!=NULL && (hasmac&2) ) {
	/* mac cjk table, often a copy of the ms table */
	putshort(at->cmap,1);		/* mac platform */
	putshort(at->cmap,
	    map->enc->is_japanese || sf->uni_interp==ui_japanese ? 1 :	/* SJIS */
	    map->enc->is_korean || sf->uni_interp==ui_korean ? 3 :	/* Korean */
	    map->enc->is_simplechinese || sf->uni_interp==ui_simp_chinese ? 25 :/* Simplified Chinese */
	    2 );			/* Big5 */
	putlong(at->cmap,applecjkpos);
    }

    putshort(at->cmap,3);	/* ms platform */
    putshort(at->cmap,		/* plat specific enc */
	    format==ff_ttfsym ? 0 :	/* Symbol */
	     1 );			/* Unicode */
    putlong(at->cmap,mspos);		/* offset from tab start to sub tab start */

    if ( format2!=NULL ) {
	putshort(at->cmap,3);		/* ms platform */
	putshort(at->cmap,		/* plat specific enc */
		strstrmatch(map->enc->enc_name,"johab")!=NULL ? 6 :
		map->enc->is_korean || sf->uni_interp==ui_korean ?		5 :
		map->enc->is_japanese || sf->uni_interp==ui_japanese ?		2 :
		map->enc->is_simplechinese || sf->uni_interp==ui_simp_chinese ?	3 :
			    4);		/* Big5 */
	putlong(at->cmap,cjkpos);	/* offset from tab start to sub tab start */
    }

    if ( format12!=NULL ) {
	putshort(at->cmap,3);		/* ms platform */
	putshort(at->cmap,10);		/* plat specific enc, ucs4 */
	putlong(at->cmap,ucs4pos);	/* offset from tab start to sub tab start */
    }
    if ( format4!=NULL ) {
	if ( !ttfcopyfile(at->cmap,format4,mspos,"cmap-Unicode16")) at->error = true;
    }
    if ( format12!=NULL ) {
	if ( !ttfcopyfile(at->cmap,format12,ucs4pos,"cmap-Unicode32")) at->error = true;
    }
    if ( format2!=NULL ) {
	if ( !ttfcopyfile(at->cmap,format2,cjkpos,"cmap-cjk")) at->error = true;
    }
    if ( apple2!=NULL ) {
	if ( !ttfcopyfile(at->cmap,apple2,applecjkpos,"cmap-applecjk")) at->error = true;
    }

    /* Mac table */
    issmall = true;
    for ( i=0; i<256; ++i )
	if ( table[i]>=256 ) {
	    issmall = false;
    break;
	}
    if ( issmall ) {
	putshort(at->cmap,0);		/* format */
	putshort(at->cmap,262);		/* length = 256bytes + 6 header bytes */
	putshort(at->cmap,0);		/* language = english */
	for ( i=0; i<256; ++i )
	    putc(table[i],at->cmap);
    } else {
	putshort(at->cmap,6);		/* format 6 */
	putshort(at->cmap,522);		/* length = 256short +10 header bytes */
	putshort(at->cmap,0);		/* language = english */
	putshort(at->cmap,0);		/* first code */
	putshort(at->cmap,256);		/* entry count */
	for ( i=0; i<256; ++i )
	    putshort(at->cmap,table[i]);
    }

    at->cmaplen = ftell(at->cmap);
    if ( (at->cmaplen&2)!=0 )
	putshort(at->cmap,0);

    if ( format==ff_ttfsym ) {
	if ( !alreadyprivate ) {
	    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
		sf->glyphs[i]->unicodeenc = sf->glyphs[i]->orig_pos;
		sf->glyphs[i]->orig_pos = i;
	    }
	}
    }
}

static int32 filecheck(FILE *file) {
    uint32 sum = 0, chunk;

    rewind(file);
    while ( 1 ) {
	chunk = getuint32(file);
	if ( feof(file))
    break;
	sum += chunk;
    }
return( sum );
}
    
static void AbortTTF(struct alltabs *at, SplineFont *sf) {
    int i;

    if ( at->loca!=NULL )
	fclose(at->loca);
    if ( at->name!=NULL )
	fclose(at->name);
    if ( at->post!=NULL )
	fclose(at->post);
    if ( at->gpos!=NULL )
	fclose(at->gpos);
    if ( at->gsub!=NULL )
	fclose(at->gsub);
    if ( at->gdef!=NULL )
	fclose(at->gdef);
    if ( at->kern!=NULL )
	fclose(at->kern);
    if ( at->cmap!=NULL )
	fclose(at->cmap);
    if ( at->headf!=NULL )
	fclose(at->headf);
    if ( at->hheadf!=NULL )
	fclose(at->hheadf);
    if ( at->maxpf!=NULL )
	fclose(at->maxpf);
    if ( at->os2f!=NULL )
	fclose(at->os2f);
    if ( at->cvtf!=NULL )
	fclose(at->cvtf);
    if ( at->vheadf!=NULL )
	fclose(at->vheadf);
    if ( at->vorgf!=NULL )
	fclose(at->vorgf);
    if ( at->cfff!=NULL )
	fclose(at->cfff);

    if ( at->gi.glyphs!=NULL )
	fclose(at->gi.glyphs);
    if ( at->gi.hmtx!=NULL )
	fclose(at->gi.hmtx);
    if ( at->gi.vmtx!=NULL )
	fclose(at->gi.vmtx);
    if ( at->fpgmf!=NULL )
	fclose(at->fpgmf);
    if ( at->prepf!=NULL )
	fclose(at->prepf);
    if ( at->gaspf!=NULL )
	fclose(at->gaspf);

    if ( at->sidf!=NULL )
	fclose(at->sidf);
    if ( at->sidh!=NULL )
	fclose(at->sidh);
    if ( at->charset!=NULL )
	fclose(at->charset);
    if ( at->encoding!=NULL )
	fclose(at->encoding);
    if ( at->private!=NULL )
	fclose(at->private);
    if ( at->charstrings!=NULL )
	fclose(at->charstrings);
    if ( at->fdselect!=NULL )
	fclose(at->fdselect);
    if ( at->fdarray!=NULL )
	fclose(at->fdarray);
    if ( at->bdat!=NULL )
	fclose(at->bdat);
    if ( at->bloc!=NULL )
	fclose(at->bloc);
    if ( at->ebsc!=NULL )
	fclose(at->ebsc);

    if ( at->prop!=NULL )
	fclose(at->prop);
    if ( at->opbd!=NULL )
	fclose(at->opbd);
    if ( at->acnt!=NULL )
	fclose(at->acnt);
    if ( at->lcar!=NULL )
	fclose(at->lcar);
    if ( at->feat!=NULL )
	fclose(at->feat);
    if ( at->morx!=NULL )
	fclose(at->morx);

    if ( at->pfed!=NULL )
	fclose(at->pfed);
    if ( at->tex!=NULL )
	fclose(at->tex);

    if ( at->gvar!=NULL )
	fclose(at->gvar);
    if ( at->fvar!=NULL )
	fclose(at->fvar);
    if ( at->cvar!=NULL )
	fclose(at->cvar);
    if ( at->avar!=NULL )
	fclose(at->avar);

    for ( i=0; i<sf->subfontcnt; ++i ) {
	if ( at->fds[i].private!=NULL )
	    fclose(at->fds[i].private);
    }
    if ( sf->subfontcnt!=0 ) {
	free(sf->glyphs);
	sf->glyphs = NULL;
	sf->glyphcnt = sf->glyphmax = 0;
    }
    if ( at->fds!=NULL )
	free( at->fds );
    free( at->gi.bygid );
}

static int SFHasInstructions(SplineFont *sf) {
    int i;

    if ( sf->mm!=NULL && sf->mm->apple )
	sf = sf->mm->normal;

    if ( sf->subfontcnt!=0 )
return( false );		/* Truetype doesn't support cid keyed fonts */

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( strcmp(sf->glyphs[i]->name,".notdef")==0 )
    continue;		/* ff produces fonts with instructions in .notdef & not elsewhere. Ignore these */
	if ( sf->glyphs[i]->ttf_instrs!=NULL )
return( true );
    }
return( false );
}

static void MaxpFromTable(struct alltabs *at,SplineFont *sf) {
    struct ttf_table *maxp;

    maxp = SFFindTable(sf,CHR('m','a','x','p'));
    if ( maxp==NULL && sf->mm!=NULL && sf->mm->apple )
	maxp = SFFindTable(sf->mm->normal,CHR('m','a','x','p'));
    if ( maxp==NULL || maxp->len<13*sizeof(uint16) )
return;
    /* We can figure out the others ourselves, but these depend on the contents */
    /*  of uninterpretted tables */
    at->maxp.maxZones = memushort(maxp->data,maxp->len, 7*sizeof(uint16));
    at->maxp.maxTwilightPts = memushort(maxp->data,maxp->len, 8*sizeof(uint16));
    at->maxp.maxStorage = memushort(maxp->data,maxp->len, 9*sizeof(uint16));
    at->maxp.maxFDEFs = memushort(maxp->data,maxp->len, 10*sizeof(uint16));
    at->maxp.maxIDEFs = memushort(maxp->data,maxp->len, 11*sizeof(uint16));
    at->maxp.maxStack = memushort(maxp->data,maxp->len, 12*sizeof(uint16));
}

static FILE *dumpstoredtable(SplineFont *sf,uint32 tag,int *len) {
    struct ttf_table *tab = SFFindTable(sf,tag);
    FILE *out;

    if ( tab==NULL && sf->mm!=NULL && sf->mm->apple )
	tab = SFFindTable(sf->mm->normal,tag);
    if ( tab==NULL ) {
	*len = 0;
return( NULL );
    }

    out = tmpfile();
    fwrite(tab->data,1,tab->len,out);
    if ( (tab->len&1))
	putc('\0',out);
    if ( (tab->len+1)&2 )
	putshort(out,0);
    *len = tab->len;
return( out );
}

static FILE *dumpsavedtable(struct ttf_table *tab) {
    FILE *out;

    if ( tab==NULL )
return( NULL );

    out = tmpfile();
    fwrite(tab->data,1,tab->len,out);
    if ( (tab->len&1))
	putc('\0',out);
    if ( (tab->len+1)&2 )
	putshort(out,0);
return( out );
}

static int tagcomp(const void *_t1, const void *_t2) {
    struct taboff *t1 = (struct taboff *) _t1, *t2 = (struct taboff *) _t2;
return( (int) (t1->tag - t2->tag) );
}

static int tcomp(const void *_t1, const void *_t2) {
    struct taboff *t1 = *((struct taboff **) _t1), *t2 = *((struct taboff **) _t2);
return( t1->orderingval - t2->orderingval );
}

static int tcomp2(const void *_t1, const void *_t2) {
    struct taboff *t1 = *((struct taboff **) _t1), *t2 = *((struct taboff **) _t2);
    if ( t1->offset>t2->offset )
return( 1 );
    else if ( t1->offset < t2->offset )
return( -1 );
return( 0 );
}

static int initTables(struct alltabs *at, SplineFont *sf,enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags) {
    int i, j, aborted, ebdtpos, eblcpos, offset;
    BDFFont *bdf;
    struct ttf_table *tab;

    if ( strmatch(at->map->enc->enc_name,"symbol")==0 && format==ff_ttf )
	format = ff_ttfsym;

    SFDefaultOS2Info(&sf->pfminfo,sf,sf->fontname);

    at->gi.xmin = at->gi.ymin = 15000;
    at->gi.sf = sf;
    if ( bf!=bf_ttf && bf!=bf_sfnt_dfont && bf!=bf_otb )
	bsizes = NULL;
    if ( bsizes!=NULL ) {
	for ( i=j=0; bsizes[i]!=0; ++i ) {
	    for ( bdf=sf->bitmaps; bdf!=NULL && (bdf->pixelsize!=(bsizes[i]&0xffff) || BDFDepth(bdf)!=(bsizes[i]>>16)); bdf=bdf->next );
	    if ( bdf!=NULL )
		bsizes[j++] = bsizes[i];
	    else
		gwwv_post_error(_("Missing bitmap strike"), _("The font database does not contain a bitmap of size %d and depth %d"), bsizes[i]&0xffff, bsizes[i]>>16 );
	}
	bsizes[j] = 0;
	for ( i=0; bsizes[i]!=0; ++i );
	at->gi.strikecnt = i;
	if ( i==0 ) bsizes=NULL;
    }

    if ( sf->subfonts!=NULL ) {
	SFDummyUpCIDs(&at->gi,sf);	/* life is easier if we ignore the seperate fonts of a cid keyed fonts and treat it as flat */
    } else if ( format!=ff_none )
	AssignTTFGlyph(&at->gi,sf,at->map,format==ff_otf);
    else {
	if ( bsizes==NULL ) {
	    gwwv_post_error(_("No bitmap strikes"), _("No bitmap strikes"));
	    AbortTTF(at,sf);
return( false );
	}
	AssignTTFBitGlyph(&at->gi,sf,at->map,bsizes);
    }

    at->maxp.version = 0x00010000;
    if ( format==ff_otf || format==ff_otfcid || (format==ff_none && at->applemode) )
	at->maxp.version = 0x00005000;
    at->maxp.maxnumcomponents = 0;
    at->maxp.maxcomponentdepth = 0;
    at->maxp.maxZones = 2;		/* 1 would probably do, don't use twilight */
    at->maxp.maxFDEFs = 1;		/* Not even 1 */
    at->maxp.maxStorage = 1;		/* Not even 1 */
    at->maxp.maxStack = 64;		/* A guess, it's probably more like 8 */
    if ( format==ff_otf || format==ff_otfcid || (format==ff_none && at->applemode) )
	at->maxp.version = 0x00005000;
    else
	MaxpFromTable(at,sf);
    at->gi.maxp = &at->maxp;
    if ( format==ff_otf )
	aborted = !dumptype2glyphs(sf,at);
    else if ( format==ff_otfcid )
	aborted = !dumpcidglyphs(sf,at);
    else if ( format==ff_none && at->applemode ) {
	aborted = !dumpcffhmtx(at,sf,true);
    } else if ( format==ff_none && at->otbbitmaps ) {
	aborted = !dumpcffhmtx(at,sf,true);
	dumpnoglyphs(sf,&at->gi);
    } else if ( format==ff_none && at->msbitmaps ) {
	aborted = !dumpglyphs(sf,&at->gi);
    } else {
	struct ttf_table *tab;
	/* There's a typo in Adobe's docs, and the instructions in these tables*/
	/*  need to be included in the max glyph instruction length */
	if ( (tab = SFFindTable(sf,CHR('f','p','g','m'))) != NULL ) {
	    at->maxp.maxglyphInstr = tab->len;
	    at->gi.has_instrs = true;
	}
	if ( (tab = SFFindTable(sf,CHR('p','r','e','p'))) != NULL ) {
	    if ( at->maxp.maxglyphInstr < tab->len )
		at->maxp.maxglyphInstr = tab->len;
	    at->gi.has_instrs = true;
	}
	at->gi.has_instrs = (SFFindTable(sf,CHR('f','p','g','m')) != NULL ||
		SFFindTable(sf,CHR('p','r','e','p')) != NULL);
	/* if format==ff_none the following will put out lots of space glyphs */
	aborted = !dumpglyphs(sf,&at->gi);
    }
    if ( format!=ff_type42 && format!=ff_type42cid ) {
	if ( bsizes!=NULL && !aborted )
	    ttfdumpbitmap(sf,at,bsizes);
	if ( bsizes!=NULL && format==ff_none && !at->applemode )
	    ttfdumpbitmapscaling(sf,at,bsizes);
    }
    if ( aborted ) {
	AbortTTF(at,sf);
return( false );
    }

    if ( at->gi.xmin==15000 ) at->gi.xmin = 0;
    if ( at->gi.ymin==15000 ) at->gi.ymin = 0;
    if ( bsizes!=NULL && format==ff_none ) {
	if (  sf->ascent >at->gi.ymax ) at->gi.ymax =  sf->ascent;
	if ( -sf->descent<at->gi.ymin ) at->gi.ymin = -sf->descent;
    }
    at->head.xmin = at->gi.xmin;
    at->head.ymin = at->gi.ymin;
    at->head.xmax = at->gi.xmax;
    at->head.ymax = at->gi.ymax;

    sethead(&at->head,sf,at);
    sethhead(&at->hhead,&at->vhead,at,sf);
    setvorg(&at->vorg,sf);
    setos2(&at->os2,at,sf,format);	/* should precede kern/ligature output */
    if ( at->gi.glyph_len<0x20000 )
	at->head.locais32 = 0;
    if ( format==ff_none && at->otbbitmaps )
	dummyloca(at);
    else if ( format!=ff_otf && format!=ff_otfcid && (format!=ff_none || (bsizes!=NULL && !at->applemode && at->opentypemode)) )
	redoloca(at);
    redohead(at);
    redohhead(at,false);
    if ( sf->hasvmetrics ) {
	redohhead(at,true);
	redovorg(at);		/* I know, VORG is only meaningful in a otf font and I dump it out in ttf too. Well, it will help ME read the font back in, and it won't bother anyone else. So there. */
    }
    redomaxp(at,format);

    if ( format!=ff_type42 && format!=ff_type42cid ) {
	if ( at->opentypemode ) {
	    otf_orderlangs(at,sf);
	    otf_dumpgpos(at,sf);
	    otf_dumpgsub(at,sf);
	    otf_dumpgdef(at,sf);
	    free(at->scripts); at->scripts = NULL;
	}
	if ( at->dovariations )
	    ttf_dumpvariations(at,sf);
	if ( at->applemode ) {
	    ttf_dumpkerns(at,sf);
	    aat_dumplcar(at,sf);
	    aat_dumpmorx(at,sf);		/* Sets the feat table too */
	    aat_dumpopbd(at,sf);
	    aat_dumpprop(at,sf);
	}
	if ( !at->applemode && (!at->opentypemode || (at->gi.flags&ttf_flag_oldkern)) )
	    ttf_dumpkerns(at,sf);		/* everybody supports a mimimal kern table */
    
	dumpnames(at,sf,format);		/* Must be after dumpmorx which may create extra names */
						/* GPOS 'size' can also create names */
	redoos2(at);
    }
    if ( format!=ff_otf && format!=ff_otfcid && format!=ff_none ) {
	if ( !SFHasInstructions(sf) && format!=ff_type42 && format!=ff_type42cid )
	    dumpgasp(at, sf);
	at->fpgmf = dumpstoredtable(sf,CHR('f','p','g','m'),&at->fpgmlen);
	at->prepf = dumpstoredtable(sf,CHR('p','r','e','p'),&at->preplen);
	at->cvtf = dumpstoredtable(sf,CHR('c','v','t',' '),&at->cvtlen);
    }
    for ( tab=sf->ttf_tab_saved; tab!=NULL; tab=tab->next )
	tab->temp = dumpsavedtable(tab);
    if ( format!=ff_type42 && format!=ff_type42cid ) {
	dumppost(at,sf,format);
	dumpcmap(at,sf,format);

	pfed_dump(at,sf);
	tex_dump(at,sf);
    }
    if ( sf->subfonts!=NULL ) {
	free(sf->glyphs); sf->glyphs = NULL;
	sf->glyphcnt = sf->glyphmax = 0;
    }
    free( at->gi.bygid );
    at->gi.gcnt = 0;

    if ( format==ff_otf || format==ff_otfcid ) {
	at->tabdir.version = CHR('O','T','T','O');
#ifdef FONTFORGE_CONFIG_APPLE_ONLY_TTF		/* This means that Windows will reject the font. In general not a good idea */
    } else if ( at->applemode && !at->opentypemode ) {
	at->tabdir.version = CHR('t','r','u','e');
#endif
    } else {
	at->tabdir.version = 0x00010000;
    }

    i = 0;

    if ( format==ff_otf || format==ff_otfcid ) {
	at->tabdir.tabs[i].tag = CHR('C','F','F',' ');
	at->tabdir.tabs[i].length = at->cfflen;
	at->tabdir.tabs[i++].data = at->cfff;
    }

    if ( at->bdat!=NULL && at->opentypemode ) {
	ebdtpos = i;
	at->tabdir.tabs[i].tag = CHR('E','B','D','T');
	at->tabdir.tabs[i].length = at->bdatlen;
	at->tabdir.tabs[i++].data = at->bdat;
    }

    if ( at->bloc!=NULL && at->opentypemode ) {
	eblcpos = i;
	at->tabdir.tabs[i].tag = CHR('E','B','L','C');
	at->tabdir.tabs[i].data = at->bloc;
	at->tabdir.tabs[i++].length = at->bloclen;
    }

    if ( at->ebsc!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('E','B','S','C');
	at->tabdir.tabs[i].data = at->ebsc;
	at->tabdir.tabs[i++].length = at->ebsclen;
    }

    if ( at->gdef!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('G','D','E','F');
	at->tabdir.tabs[i].data = at->gdef;
	at->tabdir.tabs[i++].length = at->gdeflen;
    }

    if ( at->gpos!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('G','P','O','S');
	at->tabdir.tabs[i].data = at->gpos;
	at->tabdir.tabs[i++].length = at->gposlen;
    }

    if ( at->gsub!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('G','S','U','B');
	at->tabdir.tabs[i].data = at->gsub;
	at->tabdir.tabs[i++].length = at->gsublen;
    }

    if ( at->os2f!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('O','S','/','2');
	at->tabdir.tabs[i].data = at->os2f;
	at->tabdir.tabs[i++].length = at->os2len;
    }

    if ( at->pfed!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('P','f','E','d');
	at->tabdir.tabs[i].data = at->pfed;
	at->tabdir.tabs[i++].length = at->pfedlen;
    }

    if ( at->tex!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('T','e','X',' ');
	at->tabdir.tabs[i].data = at->tex;
	at->tabdir.tabs[i++].length = at->texlen;
    }

    if ( at->bdf!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('B','D','F',' ');
	at->tabdir.tabs[i].data = at->bdf;
	at->tabdir.tabs[i++].length = at->bdflen;
    }

    if ( at->vorgf!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('V','O','R','G');
	at->tabdir.tabs[i].data = at->vorgf;
	at->tabdir.tabs[i++].length = at->vorglen;
    }

    if ( at->acnt!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('a','c','n','t');
	at->tabdir.tabs[i].data = at->acnt;
	at->tabdir.tabs[i++].length = at->acntlen;
    }

    if ( at->bdat!=NULL && at->applemode ) {
	at->tabdir.tabs[i].tag = CHR('b','d','a','t');
	if ( !at->opentypemode ) {
	    at->tabdir.tabs[i].data = at->bdat;
	    at->tabdir.tabs[i++].length = at->bdatlen;
	} else {
	    at->tabdir.tabs[i].data = NULL;
	    at->tabdir.tabs[i].dup_of = ebdtpos;
	    at->tabdir.tabs[i++].length = at->tabdir.tabs[ebdtpos].length;
	}
    }

    if ( format==ff_none && at->applemode ) {
	/* Bitmap only fonts get a bhed table rather than a head */
	at->tabdir.tabs[i].tag = CHR('b','h','e','d');
	at->tabdir.tabs[i].data = at->headf;
	at->tabdir.tabs[i++].length = at->headlen;
    }

    if ( at->bloc!=NULL && at->applemode ) {
	at->tabdir.tabs[i].tag = CHR('b','l','o','c');
	if ( !at->opentypemode ) {
	    at->tabdir.tabs[i].data = at->bloc;
	    at->tabdir.tabs[i++].length = at->bloclen;
	} else {
	    at->tabdir.tabs[i].data = NULL;
	    at->tabdir.tabs[i].dup_of = eblcpos;
	    at->tabdir.tabs[i++].length = at->tabdir.tabs[eblcpos].length;
	}
    }

    if ( at->cmap!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('c','m','a','p');
	at->tabdir.tabs[i].data = at->cmap;
	at->tabdir.tabs[i++].length = at->cmaplen;
    }

    if ( format!=ff_otf && format!=ff_otfcid && format!=ff_none ) {
	if ( at->cvtf!=NULL ) {
	    at->tabdir.tabs[i].tag = CHR('c','v','t',' ');
	    at->tabdir.tabs[i].data = at->cvtf;
	    at->tabdir.tabs[i++].length = at->cvtlen;
	}
    }

    if ( at->feat!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('f','e','a','t');
	at->tabdir.tabs[i].data = at->feat;
	at->tabdir.tabs[i++].length = at->featlen;
    }

    if ( at->fpgmf!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('f','p','g','m');
	at->tabdir.tabs[i].data = at->fpgmf;
	at->tabdir.tabs[i++].length = at->fpgmlen;
    }

    if ( at->gaspf!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('g','a','s','p');
	at->tabdir.tabs[i].data = at->gaspf;
	at->tabdir.tabs[i++].length = at->gasplen;
    }

    if ( at->gi.glyphs!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('g','l','y','f');
	at->tabdir.tabs[i].data = at->gi.glyphs;
	at->tabdir.tabs[i++].length = at->gi.glyph_len;
    }

    if ( format!=ff_none || !at->applemode ) {
	at->tabdir.tabs[i].tag = CHR('h','e','a','d');
	at->tabdir.tabs[i].data = at->headf;
	at->tabdir.tabs[i++].length = at->headlen;
    }

    at->tabdir.tabs[i].tag = CHR('h','h','e','a');
    at->tabdir.tabs[i].data = at->hheadf;
    at->tabdir.tabs[i++].length = at->hheadlen;

    at->tabdir.tabs[i].tag = CHR('h','m','t','x');
    at->tabdir.tabs[i].data = at->gi.hmtx;
    at->tabdir.tabs[i++].length = at->gi.hmtxlen;

    if ( at->kern!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('k','e','r','n');
	at->tabdir.tabs[i].data = at->kern;
	at->tabdir.tabs[i++].length = at->kernlen;
    }

    if ( at->lcar!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('l','c','a','r');
	at->tabdir.tabs[i].data = at->lcar;
	at->tabdir.tabs[i++].length = at->lcarlen;
    }

    if ( at->loca!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('l','o','c','a');
	at->tabdir.tabs[i].data = at->loca;
	at->tabdir.tabs[i++].length = at->localen;
    }

    at->tabdir.tabs[i].tag = CHR('m','a','x','p');
    at->tabdir.tabs[i].data = at->maxpf;
    at->tabdir.tabs[i++].length = at->maxplen;

    if ( at->morx!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('m','o','r','x');
	at->tabdir.tabs[i].data = at->morx;
	at->tabdir.tabs[i++].length = at->morxlen;
    }

    if ( at->name!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('n','a','m','e');
	at->tabdir.tabs[i].data = at->name;
	at->tabdir.tabs[i++].length = at->namelen;
    }

    if ( at->opbd!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('o','p','b','d');
	at->tabdir.tabs[i].data = at->opbd;
	at->tabdir.tabs[i++].length = at->opbdlen;
    }

    if ( at->post!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('p','o','s','t');
	at->tabdir.tabs[i].data = at->post;
	at->tabdir.tabs[i++].length = at->postlen;
    }

    if ( format!=ff_otf && format!=ff_otfcid && format!=ff_none ) {
	if ( at->prepf!=NULL ) {
	    at->tabdir.tabs[i].tag = CHR('p','r','e','p');
	    at->tabdir.tabs[i].data = at->prepf;
	    at->tabdir.tabs[i++].length = at->preplen;
	}
    }

    if ( at->prop!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('p','r','o','p');
	at->tabdir.tabs[i].data = at->prop;
	at->tabdir.tabs[i++].length = at->proplen;
    }

    if ( at->vheadf!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('v','h','e','a');
	at->tabdir.tabs[i].data = at->vheadf;
	at->tabdir.tabs[i++].length = at->vheadlen;

	at->tabdir.tabs[i].tag = CHR('v','m','t','x');
	at->tabdir.tabs[i].data = at->gi.vmtx;
	at->tabdir.tabs[i++].length = at->gi.vmtxlen;
    }

    if ( at->fvar!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('f','v','a','r');
	at->tabdir.tabs[i].data = at->fvar;
	at->tabdir.tabs[i++].length = at->fvarlen;
    }
    if ( at->gvar!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('g','v','a','r');
	at->tabdir.tabs[i].data = at->gvar;
	at->tabdir.tabs[i++].length = at->gvarlen;
    }
    if ( at->cvar!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('c','v','a','r');
	at->tabdir.tabs[i].data = at->cvar;
	at->tabdir.tabs[i++].length = at->cvarlen;
    }
    if ( at->avar!=NULL ) {
	at->tabdir.tabs[i].tag = CHR('a','v','a','r');
	at->tabdir.tabs[i].data = at->avar;
	at->tabdir.tabs[i++].length = at->avarlen;
    }

    if ( i>=MAX_TAB )
	IError("Miscalculation of number of tables needed. Up sizeof tabs array in struct tabdir in ttf.h" );

    for ( tab=sf->ttf_tab_saved; tab!=NULL && i<MAX_TAB; tab=tab->next ) {
	at->tabdir.tabs[i].tag = tab->tag;
	at->tabdir.tabs[i].data = tab->temp;
	at->tabdir.tabs[i++].length = tab->len;
    }
    if ( tab!=NULL )
	IError("Some user supplied tables omitted. Up sizeof tabs array in struct tabdir in ttf.h" );

    qsort(at->tabdir.tabs,i,sizeof(struct taboff),tagcomp);
    
    at->tabdir.numtab = i;
    at->tabdir.searchRange = (i<16?8:i<32?16:i<64?32:64)*16;
    at->tabdir.entrySel = (i<16?3:i<32?4:i<64?5:6);
    at->tabdir.rangeShift = at->tabdir.numtab*16-at->tabdir.searchRange;
    for ( i=0; i<at->tabdir.numtab; ++i ) {
	struct taboff *tab = &at->tabdir.tabs[i];
	at->tabdir.ordered[i] = tab;
/* This is the ordering of tables in ARIAL. I've no idea why it makes a */
/*  difference to order them, time to do a seek seems likely to be small, but */
/*  other people make a big thing about ordering them so I'll do it. */
/* I got bored after glyph. Adobe follows the same scheme for their otf fonts */
/*  so at least the world is consistant */
/* On the other hand, MS Font validator has a different idea. Oh well */
/* From: http://partners.adobe.com/asn/tech/type/opentype/recom.jsp	      */
/* TrueType Ordering							      */
/*  head, hhea, maxp, OS/2, hmtx, LTSH, VDMX, hdmx, cmap, fpgm, prep, cvt,    */
/*  loca, glyf, kern, name, post, gasp, PCLT, DSIG			      */
/* CFF in OpenType Ordering						      */
/*  head, hhea, maxp, OS/2, name, cmap, post, CFF, (other tables, as convenient) */
	if ( format==ff_otf || format==ff_otfcid ) {
	    tab->orderingval = tab->tag==CHR('h','e','a','d')? 1 :
			       tab->tag==CHR('h','h','e','a')? 2 :
			       tab->tag==CHR('m','a','x','p')? 3 :
			       tab->tag==CHR('O','S','/','2')? 4 :
			       tab->tag==CHR('n','a','m','e')? 5 :
			       tab->tag==CHR('c','m','a','p')? 6 :
			       tab->tag==CHR('p','o','s','t')? 7 :
			       tab->tag==CHR('C','F','F',' ')? 8 :
			       tab->tag==CHR('G','D','E','F')? 17 :
			       tab->tag==CHR('G','S','U','B')? 18 :
			       tab->tag==CHR('G','P','O','S')? 19 :
			       20;
	} else {
	    tab->orderingval = tab->tag==CHR('h','e','a','d')? 1 :
			       tab->tag==CHR('h','h','e','a')? 2 :
			       tab->tag==CHR('m','a','x','p')? 3 :
			       tab->tag==CHR('O','S','/','2')? 4 :
			       tab->tag==CHR('h','m','t','x')? 5 :
			       tab->tag==CHR('L','T','S','H')? 6 :
			       tab->tag==CHR('V','D','M','X')? 7 :
			       tab->tag==CHR('h','d','m','x')? 8 :
			       tab->tag==CHR('c','m','a','p')? 9 :
			       tab->tag==CHR('f','p','g','m')? 10 :
			       tab->tag==CHR('p','r','e','p')? 11 :
			       tab->tag==CHR('c','v','t',' ')? 12 :
			       tab->tag==CHR('l','o','c','a')? 13 :
			       tab->tag==CHR('g','l','y','f')? 14 :
			       tab->tag==CHR('k','e','r','n')? 15 :
			       tab->tag==CHR('n','a','m','e')? 16 :
			       tab->tag==CHR('p','o','s','t')? 17 :
			       tab->tag==CHR('g','a','s','p')? 18 :
			       tab->tag==CHR('P','C','L','T')? 19 :
			       tab->tag==CHR('D','S','I','G')? 20 :
			       tab->tag==CHR('G','D','E','F')? 21 :
			       tab->tag==CHR('G','S','U','B')? 22 :
			       tab->tag==CHR('G','P','O','S')? 23 :
			       24;
	    }
       }

    qsort(at->tabdir.ordered,at->tabdir.numtab,sizeof(struct taboff *),tcomp);

    offset = sizeof(int32)+4*sizeof(int16) + at->tabdir.numtab*4*sizeof(int32);
    for ( i=0; i<at->tabdir.numtab; ++i ) if ( at->tabdir.ordered[i]->data!=NULL ) {
	at->tabdir.ordered[i]->offset = offset;
	offset += ((at->tabdir.ordered[i]->length+3)>>2)<<2;
	at->tabdir.ordered[i]->checksum = filecheck(at->tabdir.ordered[i]->data);
    }
    for ( i=0; i<at->tabdir.numtab; ++i ) if ( at->tabdir.ordered[i]->data==NULL ) {
	struct taboff *tab = &at->tabdir.tabs[at->tabdir.ordered[i]->dup_of];
	at->tabdir.ordered[i]->offset = tab->offset;
	at->tabdir.ordered[i]->checksum = tab->checksum;
    }
return( true );
}

static char *Tag2String(uint32 tag) {
    static char buffer[8];

    buffer[0] = tag>>24;
    buffer[1] = tag>>16;
    buffer[2] = tag>>8;
    buffer[3] = tag;
    buffer[4] = 0;
return( buffer );
}

static void dumpttf(FILE *ttf,struct alltabs *at, enum fontformat format) {
    int32 checksum;
    int i, head_index=-1;
    /* I can't use fwrite because I (may) have to byte swap everything */

    putlong(ttf,at->tabdir.version);
    putshort(ttf,at->tabdir.numtab);
    putshort(ttf,at->tabdir.searchRange);
    putshort(ttf,at->tabdir.entrySel);
    putshort(ttf,at->tabdir.rangeShift);
    for ( i=0; i<at->tabdir.numtab; ++i ) {
	if ( at->tabdir.tabs[i].tag==CHR('h','e','a','d') || at->tabdir.tabs[i].tag==CHR('b','h','e','d') )
	    head_index = i;
	putlong(ttf,at->tabdir.tabs[i].tag);
	putlong(ttf,at->tabdir.tabs[i].checksum);
	putlong(ttf,at->tabdir.tabs[i].offset);
	putlong(ttf,at->tabdir.tabs[i].length);
    }

    for ( i=0; i<at->tabdir.numtab; ++i ) if ( at->tabdir.ordered[i]->data!=NULL )
	if ( !ttfcopyfile(ttf,at->tabdir.ordered[i]->data,
		at->tabdir.ordered[i]->offset,Tag2String(at->tabdir.tabs[i].tag)))
	    at->error = true;

    checksum = filecheck(ttf);
    checksum = 0xb1b0afba-checksum;
    if ( head_index!=-1 )
	fseek(ttf,at->tabdir.tabs[head_index].offset+2*sizeof(int32),SEEK_SET);
    putlong(ttf,checksum);

    /* ttfcopyfile closed all the files (except ttf) */
}

static void DumpGlyphToNameMap(char *fontname,SplineFont *sf) {
    char *d, *e;
    char *newname = galloc(strlen(fontname)+10);
    FILE *file;
    int i,k,max;
    SplineChar *sc;

    strcpy(newname,fontname);
    d = strrchr(newname,'/');
    if ( d==NULL ) d=newname;
    e = strrchr(d,'.');
    if ( e==NULL ) e = newname+strlen(newname);
    strcpy(e,".g2n");

    file = fopen(newname,"wb");
    if ( file==NULL ) {
	LogError( _("Failed to open glyph to name map file for writing: %s\n"), newname );
	free(newname);
return;
    }

    if ( sf->subfontcnt==0 )
	max = sf->glyphcnt;
    else {
	for ( k=max=0; k<sf->subfontcnt; ++k )
	    if ( sf->subfonts[k]->glyphcnt > max )
		max = sf->subfonts[k]->glyphcnt;
    }
    for ( i=0; i<max; ++i ) {
	sc = NULL;
	if ( sf->subfontcnt==0 )
	    sc = sf->glyphs[i];
	else {
	    for ( k=0; k<sf->subfontcnt; ++k ) if ( i<sf->subfonts[k]->glyphcnt )
		if ( (sc=sf->subfonts[k]->glyphs[i])!=NULL )
	    break;
	}
	if ( sc!=NULL && sc->ttf_glyph!=-1 ) {
	    fprintf( file, "GLYPHID %d\tPSNAME %s", sc->ttf_glyph, sc->name );
	    if ( sc->unicodeenc!=-1 )
		fprintf( file, "\tUNICODE %04X", sc->unicodeenc );
	    putc('\n',file);
	}
    }
    fclose(file);
    free(newname);
}

static int dumpcff(struct alltabs *at,SplineFont *sf,enum fontformat format,
	FILE *cff) {
    int ret;

    if ( format==ff_cff ) {
	AssignTTFGlyph(&at->gi,sf,at->map,true);
	ret = dumptype2glyphs(sf,at);
    } else {
	SFDummyUpCIDs(&at->gi,sf);	/* life is easier if we ignore the seperate fonts of a cid keyed fonts and treat it as flat */
	ret = dumpcidglyphs(sf,at);
	free(sf->glyphs); sf->glyphs = NULL;
	sf->glyphcnt = sf->glyphmax = 0;
    }
    free( at->gi.bygid );

    if ( !ret )
	at->error = true;
    else if ( at->gi.flags & ps_flag_nocffsugar ) {
	if ( !ttfcopyfile(cff,at->cfff,0,"CFF"))
	    at->error = true;
    } else {
	long len;
	char buffer[80];
	fprintf(cff,"%%!PS-Adobe-3.0 Resource-FontSet\n");
	fprintf(cff,"%%%%DocumentNeedResources:ProcSet (FontSetInit)\n");
	fprintf(cff,"%%%%Title: (FontSet/%s)\n", sf->fontname);
	fprintf(cff,"%%%%EndComments\n" );
	fprintf(cff,"%%%%IncludeResource: ProcSet(FontSetInit)\n" );
	fprintf(cff,"%%%%BeginResource: FontSet(%s)\n", sf->fontname );
	fprintf(cff,"/FontSetInit /ProcSet findresource begin\n" );
	fseek(at->cfff,0,SEEK_END);
	len = ftell(at->cfff);
	rewind(at->cfff);
	sprintf( buffer, "/%s %ld StartData\n", sf->fontname, len );
	fprintf(cff,"%%%%BeginData: %ld Binary Bytes\n", len+strlen(buffer) );
	fputs(buffer,cff);
	if ( !ttfcopyfile(cff,at->cfff,ftell(cff),"CFF"))
	    at->error = true;
	fprintf(cff,"\n%%%%EndData\n" );
	fprintf(cff,"%%%%EndResource\n" );
	fprintf(cff,"%%%%EOF\n" );
    }
return( !at->error );
}

int _WriteTTFFont(FILE *ttf,SplineFont *sf,enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *map) {
    struct alltabs at;
    char *oldloc;
    int i;

    oldloc = setlocale(LC_NUMERIC,"C");		/* TrueType probably doesn't need this, but OpenType does for floats in dictionaries */
    if ( format==ff_otfcid || format== ff_cffcid ) {
	if ( sf->cidmaster ) sf = sf->cidmaster;
    } else {
	if ( sf->subfontcnt!=0 ) sf = sf->subfonts[0];
    }

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	sf->glyphs[i]->ttf_glyph = -1;

    memset(&at,'\0',sizeof(struct alltabs));
    at.gi.flags = flags;
    at.applemode = (flags&ttf_flag_applemode)?1:0;
    at.opentypemode = (flags&ttf_flag_otmode)?1:0;
    at.msbitmaps = bsizes!=NULL && !at.applemode && at.opentypemode;
    at.otbbitmaps = bsizes!=NULL && bf==bf_otb;
    at.gi.onlybitmaps = format==ff_none;
    at.gi.bsizes = bsizes;
    at.gi.fixed_width = CIDOneWidth(sf);
    at.isotf = format==ff_otf || format==ff_otfcid;
    at.format = format;
    at.next_strid = 256;
    if ( at.applemode && sf->mm!=NULL && sf->mm->apple &&
	    (format==ff_ttf || format==ff_ttfsym ||  format==ff_ttfmacbin ||
			format==ff_ttfdfont) &&
	    MMValid(sf->mm,false)) {
	at.dovariations = true;
	at.gi.dovariations = true;
	sf = sf->mm->normal;
    }
    at.sf = sf;
    at.map = map;

    if ( format==ff_cff || format==ff_cffcid ) {
	dumpcff(&at,sf,format,ttf);
    } else {
	if ( initTables(&at,sf,format,bsizes,bf,flags))
	    dumpttf(ttf,&at,format);
    }
    setlocale(LC_NUMERIC,oldloc);
    if ( at.error || ferror(ttf))
return( 0 );

#ifdef __CygWin
    /* Modern versions of windows want the execute bit set on a ttf file */
    /* I've no idea what this corresponds to in windows, nor any idea on */
    /*  how to set it from the windows UI, but this seems to work */
    {
	struct stat buf;
	fstat(fileno(ttf),&buf);
	fchmod(fileno(ttf),S_IXUSR | buf.st_mode );
    }
#endif

return( 1 );
}

int WriteTTFFont(char *fontname,SplineFont *sf,enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *map) {
    FILE *ttf;
    int ret;

    if (( ttf=fopen(fontname,"wb+"))==NULL )
return( 0 );
    ret = _WriteTTFFont(ttf,sf,format,bsizes,bf,flags,map);
    if ( ret && (flags&ttf_flag_glyphmap) )
	DumpGlyphToNameMap(fontname,sf);
    if ( fclose(ttf)==-1 )
return( 0 );
return( ret );
}

struct hexout {
    FILE *type42;
    int bytesout;
};

static void dumphex(struct hexout *hexout,FILE *temp,int length) {
    int i, ch, ch1;

    if ( length&1 )
	LogError( _("Table length should not be odd\n") );

    while ( length>65534 ) {
	dumphex(hexout,temp,65534);
	length -= 65534;
    }

    fprintf( hexout->type42, " <\n  " );
    hexout->bytesout = 0;
    for ( i=0; i<length; ++i ) {
	ch = getc(temp);
	if ( ch==EOF )
    break;
	if ( hexout->bytesout>=31 ) {
	    fprintf( hexout->type42, "\n  " );
	    hexout->bytesout = 0;
	}
	ch1 = ch>>4;
	if ( ch1>=10 )
	    ch1 += 'A'-10;
	else
	    ch1 += '0';
	putc(ch1,hexout->type42);
	ch1 = ch&0xf;
	if ( ch1>=10 )
	    ch1 += 'A'-10;
	else
	    ch1 += '0';
	putc(ch1,hexout->type42);
	++hexout->bytesout;
    }
    fprintf( hexout->type42, "\n  00\n >\n" );
}
    
static void dumptype42(FILE *type42,struct alltabs *at, enum fontformat format) {
    FILE *temp = tmpfile();
    struct hexout hexout;
    int i, length;

    dumpttf(temp,at,format);
    rewind(temp);

    hexout.type42 = type42;
    hexout.bytesout = 0;

    /* Resort the tables into file order */
    qsort(at->tabdir.ordered,at->tabdir.numtab,sizeof(struct taboff *),tcomp2);

    dumphex(&hexout,temp,at->tabdir.ordered[0]->offset);

    for ( i=0; i<at->tabdir.numtab; ++i ) {
	if ( at->tabdir.ordered[i]->length>65534 && at->tabdir.ordered[i]->tag==CHR('g','l','y','f')) {
	    uint32 last = 0;
	    int j;
	    fseek(temp,at->tabdir.ordered[i]->offset,SEEK_SET);
	    for ( j=0; j<at->maxp.numGlyphs; ++j ) {
		if ( at->gi.loca[j+1]-last > 65534 ) {
		    dumphex(&hexout,temp,at->gi.loca[j]-last);
		    last = at->gi.loca[j];
		}
	    }
	    dumphex(&hexout,temp,at->gi.loca[j]-last);
	} else {
	    if ( i<at->tabdir.numtab-1 )
		length = at->tabdir.ordered[i+1]->offset-at->tabdir.ordered[i]->offset;
	    else {
		fseek(temp,0,SEEK_END);
		length = ftell(temp)-at->tabdir.ordered[i]->offset;
	    }
	    fseek(temp,at->tabdir.ordered[i]->offset,SEEK_SET);
	    dumphex(&hexout,temp,length);
	}
    }

    fclose( temp );
}

int _WriteType42SFNTS(FILE *type42,SplineFont *sf,enum fontformat format,
	int flags,EncMap *map) {
    struct alltabs at;
    char *oldloc;
    int i;

    oldloc = setlocale(LC_NUMERIC,"C");		/* TrueType probably doesn't need this, but OpenType does for floats in dictionaries */
    if ( sf->subfontcnt!=0 ) sf = sf->subfonts[0];

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	sf->glyphs[i]->ttf_glyph = -1;

    memset(&at,'\0',sizeof(struct alltabs));
    at.gi.flags = flags;
    at.applemode = false;
    at.opentypemode = false;
    at.msbitmaps = false;
    at.otbbitmaps = false;
    at.gi.onlybitmaps = false;
    at.gi.bsizes = NULL;
    at.gi.fixed_width = CIDOneWidth(sf);
    at.isotf = false;
    at.format = format;
    at.next_strid = 256;
    at.sf = sf;
    at.map = map;

    if ( initTables(&at,sf,format,NULL,bf_none,flags))
	dumptype42(type42,&at,format);
    free(at.gi.loca);

    setlocale(LC_NUMERIC,oldloc);
    if ( at.error || ferror(type42))
return( 0 );

return( 1 );
}
