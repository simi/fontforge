/* Copyright (C) 2007,2008 by George Williams */
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
#include "fontforgevw.h"
#include <ustring.h>
#include <utype.h>
#include <gkeysym.h>
#include <math.h>


/* ************************************************************************** */
/* ***************************** Condense/Extend **************************** */
/* ************************************************************************** */

/* We need to look at the counters. There are two types of counters: */
/*  the left and right side bearings */
/*  internal counters                */
/* If the glyph is nicely hinted with vertical stems then all we need to */
/*  do is look at the hints. Complications "B" which has slightly different */
/*  counters top and bottom. */
/* I'm going to assume that LCG glyphs have at most two counter zones, one */
/*  near the bottom (baseline), one near the top */
/* However many glyphs have diagonal stems: V, A, W, M, K, X, Y */
/*  Many of these have two zones (like "B" above) W, M, K, Y (maybe X) */
/* Find the places where these guys hit the baseline/cap-height (x-height) */
/*  and define these as potential counter boundries. Ignore places where   */
/*  glyphs hit with curves (like O, Q, p).
/* Remember to merge a hint with a top/bottom hit (harder with serifs) */

/* We may still not have a useful counter: 7 3 2 C E T */
/*  Use the left and right sides of the bounding box (do I need to know */
/*  StemSnapV? -- standard stem size -- yes, don't want to expand the stem) */
/* Don't try to make I 1 l i grow, only one stem even if the serifs make the */
/*  bounding box bigger */

/* If the font is italic, then skew it by the italic angle in hopes of getting*/
/*  some real vertical stems, rehint, condense/extend & unskew */

double SFStdVW(SplineFont *sf) {
    double stdvw = 0;
    char *ret;

    if ( sf->private!=NULL && (ret=PSDictHasEntry(sf->private,"StdVW"))!=NULL )
	stdvw = strtod(ret,NULL);

    if ( stdvw<=0 )
	stdvw = (sf->ascent+sf->descent)/12.5;
return( stdvw );
}

static void CIAdd(struct counterinfo *ci,int z,double start,double width) {
    int i, j;

    if ( width<0 ) {
	start += width;
	width = -width;
    }
    for ( i = 0; i<ci->cnts[z]; ++i ) {
	if ( start+width<ci->zones[z][i].start )
    break;
	if ( start<ci->zones[z][i].start + ci->zones[z][i].width )
return;		/* It intersects something that's already there */
		/* Assume the previous entry came from a hint and */
		/* so specifies the stem without the serifs and is better */
    }

    /* Need to add */
    if ( ci->cnts[z]>=ci->maxes[z] )
	ci->zones[z] = grealloc(ci->zones[z],(ci->maxes[z]+=10)*sizeof(struct ci_zones));
    for ( j=ci->cnts[z]; j>i; --j )
	ci->zones[z][j] = ci->zones[z][j-1];
    ci->zones[z][i].start = ci->zones[z][i].moveto   = start;
    ci->zones[z][i].width = ci->zones[z][i].newwidth = width;
    ++ ci->cnts[z];
}

static int SpOnEdge(SplinePoint *sp,double y,int dir,struct counterinfo *ci,int z) {
    SplinePoint *nsp, *nnsp, *psp;

    if ( sp->me.y<=y-1 || sp->me.y>y+1 )
return( false );

    /* We've already checked that we have a closed contour, so we don't need */
    /*  to worry that something might be NULL */
    psp = sp->prev->from;		/* the previous point must not be near the y value */
    if (( psp->me.y>y-1 && psp->me.y<=y+1 ) ||
	    ( dir>0 && psp->me.y<=y ) ||
	    ( dir<0 && psp->me.y>=y ) )
return( true );				/* But the point itself was on the edge */

    /* Either the next point is on the edge too, or we can have a dished */
    /*  serif, where the next point is off the edge, but the one after is on */
    /*  In a TrueType font there may be several such points, but for a PS */
    /*  flex hint there will be only one */
    nsp = sp->next->to;
    while ( nsp!=sp &&
	    ((dir>0 && nsp->me.y>y+1 && nsp->me.y<y+10) ||
	     (dir<0 && nsp->me.y<y-1 && nsp->me.y>y-10)) )
	nsp = nsp->next->to;
    if ( nsp==sp )
return( true );
    if ( nsp->me.y<=y-1 || nsp->me.y>y+1 )
return( true );
    nnsp = nsp->next->to;
    if (( nnsp->me.y>y-1 && nnsp->me.y<=y+1 ) ||
	( dir>0 && nnsp->me.y<=y ) ||
	( dir<0 && nnsp->me.y>=y ) )
return( true );

    if ( nsp->me.x-sp->me.x > 3.5 * ci->stdvw || nsp->me.x-sp->me.x < -3.5*ci->stdvw )
return( true );
    CIAdd(ci,z,sp->me.x,nsp->me.x-sp->me.x);
return( true );
}

static int HintActiveAt(StemInfo *h,double y) {
    HintInstance *hi;

    for ( hi=h->where; hi!=NULL; hi=hi->next ) {
	if ( y>=hi->begin && y<=hi->end )
return( true );
    }
return( false );
}

static void PerGlyphFindCounters(struct counterinfo *ci,SplineChar *sc, int layer) {
    StemInfo *h;
    double y, diff;
    int i,z;
    DBounds b;
    SplineSet *ss;
    SplinePoint *sp;

    ci->sc = sc;
    ci->layer = layer;
    ci->bottom_y = 0;
    if ( sc->unicodeenc!=-1 && sc->unicodeenc<0x10000 && isupper(sc->unicodeenc))
	ci->top_y = ci->bd.caph>0?ci->bd.caph:4*sc->parent->ascent/5;
    else
	ci->top_y = ci->bd.xheight>0?ci->bd.xheight:sc->parent->ascent/2;
    ci->boundry = ci->top_y/2;
    ci->has_two_zones = false;
    ci->cnts[0] = ci->cnts[1] = 0;

    diff = (ci->top_y - ci->bottom_y)/16.0;
    for ( h=sc->vstem; h!=NULL; h=h->next ) {
	for ( i=1, y=ci->bottom_y+diff; i<16; ++i, y+=diff ) {
	    if ( HintActiveAt(h,y)) {
		if ( i<8 ) {
		    CIAdd(ci,BOT_Z,h->start,h->width);
		    y += (7-i)*diff;
		    i = 7;
		} else if ( i==8 ) {
		    CIAdd(ci,BOT_Z,h->start,h->width);
		    CIAdd(ci,TOP_Z,h->start,h->width);
	break;
		} else {
		    CIAdd(ci,TOP_Z,h->start,h->width);
	break;
		}
	    }
	}
    }

    for ( ss = sc->layers[layer].splines; ss!=NULL; ss=ss->next ) {
	if ( ss->first->prev==NULL )
    continue;
	for ( sp=ss->first; ; ) {
	    if ( SpOnEdge(sp,ci->bottom_y,1,ci,BOT_Z))
		/* All Done */;
	    else if ( SpOnEdge(sp,ci->top_y,-1,ci,TOP_Z))
		/* All Done */;
	    /* Checked for sp->next==NULL above loop */
	    sp = sp->next->to;
	    if ( sp==ss->first )
	break;
	}
    }

    SplineSetFindBounds(sc->layers[layer].splines,&b);
    ci->bb = b;
    if ( ci->cnts[0]<2 && ci->cnts[1]<2 ) {
	if ( b.maxx - b.minx > 4*ci->stdvw ) {
	    for ( i=0; i<2; ++i ) {
		CIAdd(ci,i,b.minx,1.5*ci->stdvw);
		CIAdd(ci,i,b.maxx-1.5*ci->stdvw, 1.5*ci->stdvw);
	    }
	}
    }

    if ( ci->cnts[0]!=ci->cnts[1] )
	ci->has_two_zones = true;
    else {
	for ( i=0; i<ci->cnts[0]; ++i ) {
	    /* if one stem is entirely within the other, then that counts as */
	    /*  the same */
	    if (( ci->zones[0][i].start<=ci->zones[1][i].start &&
		    ci->zones[0][i].start+ci->zones[0][i].width >= ci->zones[1][i].start+ci->zones[1][i].width) ||
		  ( ci->zones[1][i].start<=ci->zones[0][i].start &&
		    ci->zones[1][i].start+ci->zones[1][i].width >= ci->zones[0][i].start+ci->zones[0][i].width) )
	continue;		/* They match, close enough */
	    ci->has_two_zones = true;
	break;
	}
    }

    /* We do not include the side bearing adjustment here. That must be done  */
    /*  separately, because we skip counter movements if there are no counters*/
    for ( z=0; z<2; ++z ) {
	for ( i=1; i<ci->cnts[z]; ++i ) {
	    ci->zones[z][i].moveto = ci->zones[z][i-1].moveto + ci->zones[z][i-1].newwidth +
		    ci->c_factor/100.0 * (ci->zones[z][i].start-(ci->zones[z][i-1].start+ci->zones[z][i-1].width)) +
		    ci->c_add;
	}
    }

    if ( ci->has_two_zones ) {
	int j,k;
	double diff;
	/* Now are there any common stems in the two zones? Common stems */
	/*  should be forced to the same location even if that isn't what */
	/*  we calculated above */
	for ( i=0; i<ci->cnts[0]; ++i ) {
	    for ( j=0; j<ci->cnts[1]; ++j ) {
		if ( ci->zones[0][i].start == ci->zones[1][j].start &&
			ci->zones[0][i].moveto != ci->zones[1][j].moveto ) {
		    if ( ci->zones[0][i].moveto > ci->zones[1][j].moveto ) {
			diff = ci->zones[0][i].moveto - ci->zones[1][j].moveto;
			for ( k=j; k<ci->cnts[1]; ++k )
			    ci->zones[1][j].moveto += diff;
		    } else {
			diff = ci->zones[1][j].moveto - ci->zones[0][i].moveto;
			for ( k=j; k<ci->cnts[0]; ++k )
			    ci->zones[0][i].moveto += diff;
		    }
		}
	    }
	}
    }
}

static void BPAdjustCEZ(BasePoint *bp, struct counterinfo *ci, int z) {
    int i;

    if ( ci->cnts[z]<2 )	/* No counters */
return;
    if ( bp->x<ci->zones[z][0].start+ci->zones[z][0].width ) {
	if ( bp->x<ci->zones[z][0].start || ci->zones[z][0].width==ci->zones[z][0].newwidth )
	    bp->x += ci->zones[z][0].moveto - ci->zones[z][0].start;
	else
	    bp->x = ci->zones[z][0].moveto +
		    ci->zones[z][0].newwidth * (bp->x-ci->zones[z][0].start)/ ci->zones[z][0].width;
return;
    }

    for ( i=1; i<ci->cnts[z]; ++i ) {
	if ( bp->x<ci->zones[z][i].start+ci->zones[z][i].width ) {
	    if ( bp->x<ci->zones[z][i].start ) {
		double base = ci->zones[z][i-1].moveto + ci->zones[z][i-1].newwidth;
		double oldbase = ci->zones[z][i-1].start + ci->zones[z][i-1].width;
		bp->x = base +
			(bp->x-oldbase) *
				(ci->zones[z][i].moveto-base)/
			        (ci->zones[z][i].start-oldbase);
	    } else {
		bp->x = ci->zones[z][i].moveto +
		    ci->zones[z][i].newwidth * (bp->x-ci->zones[z][i].start)/ ci->zones[z][i].width;
	    }
return;
	}
    }

    bp->x += ci->zones[z][i-1].moveto + ci->zones[z][i-1].newwidth -
	    (ci->zones[z][i-1].start + ci->zones[z][i-1].width);
}

static void BPAdjustCE(BasePoint *bp, struct counterinfo *ci) {

    if ( !ci->has_two_zones )
	BPAdjustCEZ(bp,ci,0);
    else if ( ci->cnts[BOT_Z]<2 && ci->cnts[TOP_Z]>=2 )
	BPAdjustCEZ(bp,ci,TOP_Z);
    else if ( ci->cnts[TOP_Z]<2 && ci->cnts[BOT_Z]>=2 )
	BPAdjustCEZ(bp,ci,BOT_Z);
    else if ( bp->y > ci->boundry )
	BPAdjustCEZ(bp,ci,TOP_Z);
    else
	BPAdjustCEZ(bp,ci,BOT_Z);
}

void CI_Init(struct counterinfo *ci,SplineFont *sf) {

    QuickBlues(sf, ci->layer, &ci->bd);

    ci->stdvw = SFStdVW(sf);
}

void SCCondenseExtend(struct counterinfo *ci,SplineChar *sc, int layer,
	int do_undoes) {
    SplineSet *ss;
    SplinePoint *sp;
    Spline *s, *first;
    DBounds b;
    int width;
    double offset;
    real transform[6];
    int order2 = sc->layers[layer].order2;

    if ( do_undoes )
	SCPreserveLayer(sc,layer,false);

    if ( ci->correct_italic && sc->parent->italicangle!=0 ) {
	memset(transform,0,sizeof(transform));
	transform[0] = transform[3] = 1;
	transform[2] = tan( sc->parent->italicangle * 3.1415926535897932/180.0 );
	SplinePointListTransform(sc->layers[layer].splines,transform,true);
	StemInfosFree(sc->vstem); sc->vstem=NULL;
    }
    if ( sc->vstem==NULL )
	_SplineCharAutoHint(sc,ci->layer,&ci->bd,NULL,false);

    PerGlyphFindCounters(ci,sc, layer);

    for ( ss = sc->layers[layer].splines; ss!=NULL; ss = ss->next ) {
	for ( sp=ss->first; ; ) {
	    BPAdjustCE(&sp->nextcp,ci);
	    BPAdjustCE(&sp->prevcp,ci);
	    if ( sp->ttfindex==0xffff && order2 ) {
		sp->me.x = (sp->nextcp.x+sp->prevcp.x)/2;
		sp->me.y = (sp->nextcp.y+sp->prevcp.y)/2;
	    } else
		BPAdjustCE(&sp->me,ci);
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==ss->first )
	break;
	}
	first = NULL;
	for ( s=ss->first->next; s!=NULL && s!=first; s = s->to->next ) {
	    if ( first==NULL ) first = s;
	    SplineRefigure(s);
	}
    }

    SplineSetFindBounds(sc->layers[layer].splines,&b);
    memset(transform,0,sizeof(transform));
    transform[0] = transform[3] = 1;
    transform[4] = ci->bb.minx*ci->sb_factor/100. + ci->sb_add - b.minx;
    if ( transform[4]!=0 )
	SplinePointListTransform(sc->layers[layer].splines,transform,true);
    if ( layer!=ly_back ) {
	width = b.maxx + (sc->width-ci->bb.maxx)*ci->sb_factor/100. + ci->sb_add;
	SCSynchronizeWidth(sc,width,sc->width,NULL);
	offset = (b.maxx-b.minx)/2 - (ci->bb.maxx-ci->bb.minx)/2;
	/* We haven't really changed the left side bearing by offset, but */
	/*  this is the amount (about) by which we need to adjust accents */
	SCSynchronizeLBearing(sc,offset,ci->layer);
    }

    if ( ci->correct_italic && sc->parent->italicangle!=0 ) {
	/* If we unskewed it, we want to skew it now */
	memset(transform,0,sizeof(transform));
	transform[0] = transform[3] = 1;
	transform[2] = -tan( sc->parent->italicangle * 3.1415926535897932/180.0 );
	SplinePointListTransform(sc->layers[layer].splines,transform,true);
    }
    
    if ( layer!=ly_back ) {
	/* Hints will be inccorrect (misleading) after these transformations */
	StemInfosFree(sc->vstem); sc->vstem=NULL;
	StemInfosFree(sc->hstem); sc->hstem=NULL;
	DStemInfosFree(sc->dstem); sc->dstem=NULL;
	SCOutOfDateBackground(sc);
    }
    SCCharChangedUpdate(sc,layer);
}

void ScriptSCCondenseExtend(SplineChar *sc,struct counterinfo *ci) {

    SCCondenseExtend(ci, sc, ci->layer, true);

    free( ci->zones[0]);
    free( ci->zones[1]);
}

void FVCondenseExtend(FontViewBase *fv,struct counterinfo *ci) {
    int i, gid;
    SplineChar *sc;

    for ( i=0; i<fv->map->enccount; ++i ) if ( fv->selected[i] &&
	    (gid = fv->map->map[i])!=-1 && (sc=fv->sf->glyphs[gid])!=NULL ) {
	SCCondenseExtend(ci,sc,ly_fore,true);
    }

    free( ci->zones[0]);
    free( ci->zones[1]);
}

/* ************************************************************************** */
/* ***************************** Embolden Dialog **************************** */
/* ************************************************************************** */


struct ptmoves {
    SplinePoint *sp;
    BasePoint pdir, ndir;
    double factor;
    BasePoint newpos;
    uint8 touched;
};

static int MaxContourCount(SplineSet *ss) {
    int cnt, ccnt;
    SplinePoint *sp;

    ccnt = 0;
    for ( ; ss!=NULL ; ss=ss->next ) {
	if ( ss->first->prev==NULL )
    continue;
	for ( cnt=0, sp=ss->first; ; ) {
	    sp = sp->next->to; ++cnt;
	    if ( sp==ss->first )
	break;
	}
	if ( cnt>ccnt ) ccnt = cnt;
    }
return( ccnt );
}

static int PtMovesInitToContour(struct ptmoves *ptmoves,SplineSet *ss) {
    int cnt;
    SplinePoint *sp;
    BasePoint dir1, dir2;
    double len;

    for ( cnt=0, sp=ss->first; ; ) {
	ptmoves[cnt].sp = sp;
	ptmoves[cnt].newpos = sp->me;
	ptmoves[cnt].touched = false;
	if ( sp->nonextcp ) {
	    dir1.x = sp->next->to->me.x - sp->me.x;
	    dir1.y = sp->next->to->me.y - sp->me.y;
	} else {
	    dir1.x = sp->nextcp.x - sp->me.x;
	    dir1.y = sp->nextcp.y - sp->me.y;
	}
	len = dir1.x*dir1.x + dir1.y*dir1.y;
	if ( len!=0 ) {
	    len = sqrt(len);
	    dir1.x /= len;
	    dir1.y /= len;
	}
	ptmoves[cnt].ndir = dir1;
	if ( dir1.x<0 ) dir1.x = -dir1.x;

	if ( sp->noprevcp ) {
	    dir2.x = sp->prev->from->me.x - sp->me.x;
	    dir2.y = sp->prev->from->me.y - sp->me.y;
	} else {
	    dir2.x = sp->prevcp.x - sp->me.x;
	    dir2.y = sp->prevcp.y - sp->me.y;
	}
	len = dir2.x*dir2.x + dir2.y*dir2.y;
	if ( len!=0 ) {
	    len = sqrt(len);
	    dir2.x /= len;
	    dir2.y /= len;
	}
	ptmoves[cnt].pdir = dir2;
	if ( dir2.x<0 ) dir2.x = -dir2.x;
	
	ptmoves[cnt].factor = dir1.x>dir2.x ? dir1.x : dir2.x;

	sp = sp->next->to; ++cnt;
	if ( sp==ss->first )
    break;
    }
    ptmoves[cnt] = ptmoves[0];	/* Life is easier if we don't have to worry about edge effects */
return( cnt );
}

static void InterpolateControlPointsAndSet(struct ptmoves *ptmoves,int cnt) {
    SplinePoint *sp, *nsp;
    int i;
    int order2 = ptmoves[0].sp->next!=NULL && ptmoves[0].sp->next->order2;

    ptmoves[cnt].newpos = ptmoves[0].newpos;
    for ( i=0; i<cnt; ++i ) {
	sp = ptmoves[i].sp;
	nsp = ptmoves[i+1].sp;
	if ( sp->nonextcp )
	    sp->nextcp = ptmoves[i].newpos;
	if ( nsp->noprevcp )
	    nsp->prevcp = ptmoves[i+1].newpos;
	if ( isnan(ptmoves[i].newpos.y) )
	    IError( "Nan value in InterpolateControlPointsAndSet\n" );
	if ( sp->me.y!=nsp->me.y ) {
	    sp->nextcp.y = ptmoves[i].newpos.y + (sp->nextcp.y-sp->me.y)*
				(ptmoves[i+1].newpos.y - ptmoves[i].newpos.y)/
				(nsp->me.y - sp->me.y);
	    nsp->prevcp.y = ptmoves[i].newpos.y + (nsp->prevcp.y-sp->me.y)*
				(ptmoves[i+1].newpos.y - ptmoves[i].newpos.y)/
				(nsp->me.y - sp->me.y);
	    if ( sp->me.x!=nsp->me.x ) {
		sp->nextcp.x = ptmoves[i].newpos.x + (sp->nextcp.x-sp->me.x)*
				    (ptmoves[i+1].newpos.x - ptmoves[i].newpos.x)/
				    (nsp->me.x - sp->me.x);
		nsp->prevcp.x = ptmoves[i].newpos.x + (nsp->prevcp.x-sp->me.x)*
				    (ptmoves[i+1].newpos.x - ptmoves[i].newpos.x)/
				    (nsp->me.x - sp->me.x);
	    }
	}
	if ( isnan(sp->nextcp.y) )
	    IError( "Nan value in InterpolateControlPointsAndSet\n" );
    }
    for ( i=0; i<cnt; ++i )
	ptmoves[i].sp->me = ptmoves[i].newpos;
    if ( order2 ) {
	for ( i=0; i<cnt; ++i ) if ( (sp = ptmoves[i].sp)->ttfindex==0xffff ) {
	    sp->me.x = (sp->nextcp.x+sp->prevcp.x)/2;
	    sp->me.y = (sp->nextcp.y+sp->prevcp.y)/2;
	}
    }
    for ( i=0; i<cnt; ++i )
	SplineRefigure(ptmoves[i].sp->next);
}

static void CorrectLeftSideBearing(SplineSet *ss_expanded,SplineChar *sc,int layer) {
    real transform[6];
    DBounds old, new;
    /* Now correct the left side bearing */

    SplineSetFindBounds(sc->layers[layer].splines,&old);
    SplineSetFindBounds(ss_expanded,&new);
    memset(transform,0,sizeof(transform));
    transform[0] = transform[3] = 1;
    transform[4] = old.minx - new.minx;
    if ( transform[4]!=0 ) {
	SplinePointListTransform(ss_expanded,transform,true);
	if ( layer==ly_fore )
	    SCSynchronizeLBearing(sc,transform[4],layer);
    }
}

static SplinePoint *FindMatchingPoint(int ptindex,SplineSet *ss) {
    SplinePoint *sp;

    if( ptindex==0 )
return( NULL );
    for ( ; ss!=NULL; ss=ss->next ) {
	for ( sp=ss->first; ; ) {
	    if ( sp->ptindex == ptindex )
return( sp );
	    if ( sp->next == NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==ss->first )
	break;
	}
    }
return( NULL );
}

static StemInfo *OnHint(StemInfo *stems,double searchy,double *othery) {
    StemInfo *h;

    for ( h=stems; h!=NULL; h=h->next ) {
	if ( h->start == searchy ) {
	    *othery = h->start+h->width;
return( h );
	} else if ( h->start+h->width == searchy ) {
	    *othery = h->start;
return( h );
	}
    }

    for ( h=stems; h!=NULL; h=h->next ) {
	if ( searchy>=h->start-2 && searchy<=h->start+2 ) {
	    *othery = h->start+h->width;
return( h );
	} else if ( searchy>=h->start+h->width-2 && searchy<=h->start+h->width+2 ) {
	    *othery = h->start;
return( h );
	}
    }

return( NULL );
}

static StemInfo *MightBeOnHint(StemInfo *stems,struct lcg_zones *zones,
	struct ptmoves *pt,double *othery) {
    double offset;

    if ( pt->ndir.y!=0 && pt->pdir.y!=0 )
return( NULL );			/* Not horizontal, not on a stem */

    offset = (pt->ndir.y==0 && pt->ndir.x>0) ||
	    (pt->pdir.y==0 && pt->pdir.x<0 ) ?  zones->stroke_width/2
					     : -zones->stroke_width/2;
return( OnHint(stems,pt->sp->me.y-offset,othery));
}

static int InHintAroundZone(StemInfo *stems,double searchy,double contains_y) {
    StemInfo *h;

    for ( h=stems; h!=NULL; h=h->next ) {
	if ( h->start >= searchy && h->start+h->width<=searchy &&
		 h->start >= contains_y && h->start+h->width<=contains_y )
return( true );
    }
return( false );
}

static int IsStartPoint(SplinePoint *sp, SplineChar *sc, int layer) {
    SplineSet *ss;
    
    if ( sp->ptindex==0 )
return( false );
    for ( ss=sc->layers[layer].splines; ss!=NULL; ss=ss->next ) {
	if ( ss->first->ptindex==sp->ptindex )
return( true );
    }
return( false );
}

static void FindStartPoint(SplineSet *ss_expanded, SplineChar *sc, int layer) {
    SplinePoint *sp;
    SplineSet *ss;

    for ( ss=ss_expanded; ss!=NULL; ss=ss->next ) {
	int found = false;
	if ( ss->first->prev==NULL )
    continue;
	for ( sp=ss->first; ; ) {
	    if ( IsStartPoint(sp,sc,layer) ) {
		ss->first = ss->last = sp;
		found = true;
	break;
	    }
	    sp = sp->prev->from;
	    if ( sp==ss->first )
	break;
	}
	if ( !found )
	    ss->first = ss->last = sp->prev->from;		/* Often true */
    }
}

static SplineSet *LCG_EmboldenHook(SplineSet *ss_expanded,struct lcg_zones *zones,
	SplineChar *sc, int layer) {
    SplineSet *ss;
    SplinePoint *sp, *nsp, *psp;
    int cnt, ccnt, i;
    struct ptmoves *ptmoves;
    /* When we do an expand stroke we expect that a glyph's bounding box will */
    /*  increase by half the stroke width in each direction. Now we want to do*/
    /*  different operations in each dimension. We don't want to increase the */
    /*  x-height, or the cap height, whereas it is ok to make the glyph wider */
    /*  but it would be nice to preserve the left and right side bearings. */
    /* So shift the glyph over by width/2 (this preserves the left side bearing)*/
    /* Increment the width by width (this preserves the right side bearing)   */
    /* The y dimension is more complex. In a normal latin (greek, cyrillic)   */
    /*  font the x-height is key, in a font with only capital letters it would*/
    /*  be the cap-height. In the subset of the font containing superscripts  */
    /*  we might want to key off of the superscript size. The expansion will  */
    /*  mean that the glyph protrudes width/2 below the baseline, and width/2 */
    /*  above the x-height and cap-height. Neither of those is acceptable. So */
    /*  choose some intermediate level, say x-height/2, any point above this  */
    /*  moves down by width/2, any point below this moves up by width/2 */
    /* That'll work for many glyphs. But consider "o", depending on how it is */
    /*  designed we might be moving the leftmost point either up or down when */
    /*  it should remain near the center. So perhaps we have a band which is  */
    /*  width wide right around x-height/2 and points in that band don't move */
    /* Better. Consider a truetype "o" were there will be intermediate points */
    /*  Between the top-most and left-most. These shouldn't move by the full */
    /*  width/2. They should move by some interpolated amount. Based on the */
    /*  center of the glyph? That would screw up "h". Take the normal to the */
    /*  curve. Dot it into the y direction. Move the point by that fraction */
    /* Control points should be interpolated between the movements of their */
    /*  on-curve points */

    ccnt = MaxContourCount(ss_expanded);
    if ( ccnt==0 )
return(ss_expanded);			/* No points? Nothing to do */
    ptmoves = galloc((ccnt+1)*sizeof(struct ptmoves));
    for ( ss = ss_expanded; ss!=NULL ; ss=ss->next ) {
	if ( ss->first->prev==NULL )
    continue;
	cnt = PtMovesInitToContour(ptmoves,ss);
	for ( i=0; i<cnt; ++i ) {
	    int p = i==0?cnt-1:i-1, sign = 0;
	    sp = ptmoves[i].sp;
	    nsp = ptmoves[i+1].sp;
	    psp = ptmoves[p].sp;
	    if ( sp->me.y>=zones->top_zone )
		sign = -1;
	    else if ( sp->me.y<zones->bottom_zone )
		sign = 1;
		
	    /* Fix vertical serifs */
	    if ( zones->serif_height>0 &&
		     RealWithin(sp->me.y,zones->bottom_bound+zones->serif_height+zones->stroke_width/2,zones->serif_fuzz) ) {
		ptmoves[i].newpos.y = zones->bottom_bound+zones->serif_height;
	    } else if ( zones->serif_height>0 &&
		     RealWithin(sp->me.y,zones->top_bound-zones->serif_height-zones->stroke_width/2,zones->serif_fuzz) ) {
		ptmoves[i].newpos.y = zones->top_bound-zones->serif_height;
	    } else if ( sign ) {
		ptmoves[i].newpos.y += sign*zones->stroke_width*ptmoves[i].factor/2;
		/* This is to improve the looks of diagonal stems */
		if ( sp->next->islinear && sp->prev->islinear && nsp->next->islinear &&
			nsp->me.y == sp->me.y &&
			ptmoves[i].pdir.x*ptmoves[i+1].ndir.x + ptmoves[i].pdir.y*ptmoves[i+1].ndir.y<-.999 ) {
		    if ( ptmoves[i].pdir.y<0 )
			sign = -sign;
		    ptmoves[i].newpos.x += sign*zones->stroke_width*ptmoves[i].pdir.x;
		} else if ( sp->next->islinear && sp->prev->islinear && psp->next->islinear &&
			psp->me.y == sp->me.y &&
			ptmoves[i].ndir.x*ptmoves[p].pdir.x + ptmoves[i].ndir.y*ptmoves[p].pdir.y<-.999 ) {
		    if ( ptmoves[i].ndir.y<0 )
			sign = -sign;
		    ptmoves[i].newpos.x += sign*zones->stroke_width*ptmoves[i].ndir.x;
		}
	    }
	}
	InterpolateControlPointsAndSet(ptmoves,cnt);
    }
    free(ptmoves);

    if ( zones->counter_type == ct_retain || (sc->width!=0 && zones->counter_type == ct_auto))
	CorrectLeftSideBearing(ss_expanded,sc,layer);
return(ss_expanded);
}

static SplineSet *LCG_HintedEmboldenHook(SplineSet *ss_expanded,struct lcg_zones *zones,
	SplineChar *sc,int layer) {
    SplineSet *ss;
    SplinePoint *sp, *nsp, *psp, *origsp;
    int cnt, ccnt, i, n, p, j;
    struct ptmoves *ptmoves;
    StemInfo *h;
    double othery;
    /* Anything below the baseline moves up by width/2 */
    /* Anything that was on the base line moves up so that it still is on the baseline */
    /*  If it's a diagonal stem, may want to move in x as well as y */
    /* Anything on a hint one side of which is on the base line, then the other */
    /*  side moves up so it is hint->width + width above the baseline */

    /* Same for either x-height or cap-height, except that everything moves down */

    /* Any points on hints between baseline/x-height are fixed */

    /* Other points between baseline/x-height follow IUP rules */

    if ( layer!=ly_fore || sc->hstem==NULL )
return( LCG_EmboldenHook(ss_expanded,zones,sc,layer));

    FindStartPoint(ss_expanded,sc,layer);
    ccnt = MaxContourCount(ss_expanded);
    if ( ccnt==0 )
return(ss_expanded);			/* No points? Nothing to do */
    ptmoves = galloc((ccnt+1)*sizeof(struct ptmoves));
    for ( ss = ss_expanded; ss!=NULL ; ss=ss->next ) {
	if ( ss->first->prev==NULL )
    continue;
	cnt = PtMovesInitToContour(ptmoves,ss);
	/* Touch (usually move) all points which are either in our zones or on a hint */
	for ( i=0; i<cnt; ++i ) {
	    int sign = 0;
	    sp = ptmoves[i].sp;
	    origsp = FindMatchingPoint(sp->ptindex,sc->layers[layer].splines);
	    h = NULL; othery = 0;
	    if ( origsp!=NULL )
		h = OnHint(sc->hstem,origsp->me.y,&othery);
	    else
		h = MightBeOnHint(sc->hstem,zones,&ptmoves[i],&othery);

	    /* Fix vertical serifs */
	    if ( zones->serif_height>0 &&
		    (( origsp!=NULL && RealWithin(origsp->me.y, zones->bottom_bound+zones->serif_height,zones->serif_fuzz)) ||
		     RealWithin(sp->me.y,zones->bottom_bound+zones->serif_height+zones->stroke_width/2,zones->serif_fuzz)) ) {
		ptmoves[i].touched = true;
		ptmoves[i].newpos.y = zones->bottom_bound+zones->serif_height;
	    } else if ( zones->serif_height>0 &&
		    (( origsp!=NULL && RealWithin(origsp->me.y, zones->top_bound-zones->serif_height,zones->serif_fuzz)) ||
		     RealWithin(sp->me.y,zones->top_bound-zones->serif_height-zones->stroke_width/2,zones->serif_fuzz)) ) {
		ptmoves[i].touched = true;
		ptmoves[i].newpos.y = zones->top_bound-zones->serif_height;
	    } else if ( sp->me.y>=zones->top_bound || (h!=NULL && othery+zones->stroke_width/2>=zones->top_bound))
		sign = -1;
	    else if ( sp->me.y<=zones->bottom_bound || (h!=NULL && othery-zones->stroke_width/2<=zones->bottom_bound))
		sign = 1;
	    else if ( origsp!=NULL &&
			    (InHintAroundZone(sc->hstem,origsp->me.y,zones->top_bound) ||
			     InHintAroundZone(sc->hstem,origsp->me.y,zones->bottom_bound)) )
		/* It's not on the hint, so we want to interpolate it even if */
		/*  it's in an area we'd normally move (tahoma "s" has two */
		/*  points which fall on the baseline but aren't on the bottom */
		/*  hint */
		/* Do Nothing */;
	    else if ( h!=NULL &&
		    ((h->start>=zones->bottom_zone && h->start<=zones->top_zone) ||
		     (h->start+h->width>=zones->bottom_zone && h->start+h->width<=zones->top_zone)) ) {
		/* Point on a hint. In the middle of the glyph */
		/*  This one not in the zones, so it is fixed */
		ptmoves[i].touched = true;
	    }
	    if ( sign ) {
		ptmoves[i].touched = true;
		p = i==0?cnt-1:i-1;
		nsp = ptmoves[i+1].sp;
		psp = ptmoves[p].sp;
		if ( origsp!=NULL && ((origsp->me.y>=zones->bottom_bound-2 && origsp->me.y<=zones->bottom_bound+2 ) ||
			(origsp->me.y>=zones->top_bound-2 && origsp->me.y<=zones->top_bound+2 )) ) {
		    ptmoves[i].newpos.y += sign*zones->stroke_width*ptmoves[i].factor/2;
		    /* This is to improve the looks of diagonal stems */
		    if ( sp->next->islinear && sp->prev->islinear && nsp->next->islinear &&
			    nsp->me.y == sp->me.y &&
			    ptmoves[i].pdir.x*ptmoves[i+1].ndir.x + ptmoves[i].pdir.y*ptmoves[i+1].ndir.y<-.999 ) {
			if ( ptmoves[i].pdir.y<0 )
			    sign = -sign;
			ptmoves[i].newpos.x += sign*zones->stroke_width*ptmoves[i].pdir.x;
		    } else if ( sp->next->islinear && sp->prev->islinear && psp->next->islinear &&
			    psp->me.y == sp->me.y &&
			    ptmoves[i].ndir.x*ptmoves[p].pdir.x + ptmoves[i].ndir.y*ptmoves[p].pdir.y<-.999 ) {
			if ( ptmoves[i].ndir.y<0 )
			    sign = -sign;
			ptmoves[i].newpos.x += sign*zones->stroke_width*ptmoves[i].ndir.x;
		    }
		} else
		    ptmoves[i].newpos.y += sign*zones->stroke_width/2;
	    }
	}
	/* Now find each untouched point and interpolate how it moves */
	for ( i=0; i<cnt; ++i ) if ( !ptmoves[i].touched ) {
	    for ( p=i-1; p!=i; --p ) {
		if ( p<0 ) {
		    p=cnt-1;
		    if ( p==i )
	    break;
		}
		if ( ptmoves[p].touched )
	    break;
	    }
	    if ( p==i )
	break;			/* Nothing on the contour touched. Can't change anything */
	    for ( n=i+1; n!=i; ++n ) {
		if ( n>=cnt ) {
		    n=0;
		    if ( n==i )
	    break;
		}
		if ( ptmoves[n].touched )
	    break;
	    }
	    nsp = ptmoves[n].sp;
	    psp = ptmoves[p].sp;
	    for ( j=p+1; j!=n; ++j ) {
		if ( j==cnt ) {
		    j=0;
		    if ( n==0 )
	    break;
		}
		sp = ptmoves[j].sp;
		if (( sp->me.y>nsp->me.y && sp->me.y>psp->me.y ) ||
			(sp->me.y<nsp->me.y && sp->me.y<psp->me.y )) {
		    if (( sp->me.y>nsp->me.y && nsp->me.y>psp->me.y ) ||
			    (sp->me.y<nsp->me.y && nsp->me.y<psp->me.y )) {
			ptmoves[j].newpos.y += ptmoves[n].newpos.y-nsp->me.y;
			ptmoves[j].newpos.x += ptmoves[n].newpos.x-nsp->me.x ;
		    } else {
			ptmoves[j].newpos.y += ptmoves[p].newpos.y-psp->me.y;
			ptmoves[j].newpos.x += ptmoves[p].newpos.x-psp->me.x ;
		    }
		} else {
		    double diff;
		    diff = nsp->me.y - psp->me.y;
		    if ( diff!=0 ) {
			ptmoves[j].newpos.y += (ptmoves[p].newpos.y-psp->me.y)
						+ (sp->me.y-psp->me.y)*(ptmoves[n].newpos.y-nsp->me.y-(ptmoves[p].newpos.y-psp->me.y))/diff;
			/* Note we even interpolate the x direction depending on */
			/*  y position */
			ptmoves[j].newpos.x += (ptmoves[p].newpos.x-psp->me.x)
						+ (sp->me.y-psp->me.y)*(ptmoves[n].newpos.x-nsp->me.x-(ptmoves[p].newpos.x-psp->me.x))/diff;
		    } else if ( (diff = nsp->me.x - psp->me.x)!=0 ) {
			ptmoves[j].newpos.x += (ptmoves[p].newpos.x-psp->me.x)
						+ (sp->me.x-psp->me.x)*(ptmoves[n].newpos.x-nsp->me.x-(ptmoves[p].newpos.x-psp->me.x))/diff;
			/* Note we even interpolate the y direction depending on */
			/*  x position */
			ptmoves[j].newpos.y += (ptmoves[p].newpos.y-psp->me.y)
						+ (sp->me.x-psp->me.x)*(ptmoves[n].newpos.y-nsp->me.y-(ptmoves[p].newpos.y-psp->me.y))/diff;
		    }
		}
		ptmoves[j].touched = true;
		if ( isnan( ptmoves[j].newpos.y ))
		    IError("Nan value in LCG_HintedEmboldenHook\n" );
	    }
	}
	InterpolateControlPointsAndSet(ptmoves,cnt);
    }
    free(ptmoves);

    if ( zones->counter_type == ct_retain || (sc->width!=0 && zones->counter_type == ct_auto))
	CorrectLeftSideBearing(ss_expanded,sc,layer);
return( ss_expanded );
}

static void AdjustCounters(SplineChar *sc, struct lcg_zones *zones,
	DBounds *old, DBounds *new) {
    struct counterinfo ci;

    /* I did the left side bearing as I went along. I'll do the right side */
    /*  bearing now. I don't use condense/extend because I have more info */
    /*  here, and because I might not want to adjust both by the same amount */
    SCSynchronizeWidth(sc,sc->width+rint(zones->stroke_width),sc->width,NULL);
    /* Now do the internal counters. The Emboldening will (for vertical stems)*/
    /*  have made counters smaller by stroke_width (diagonal stems who knows) */
    /*  so make them bigger by that amount */
    memset(&ci,0,sizeof(ci));
    ci.bd = zones->bd;
    ci.stdvw = zones->stdvw;
    ci.top_y = zones->top_bound;
    ci.bottom_y = zones->bottom_bound;
    ci.boundry = (zones->top_bound+zones->bottom_bound)/2;
    ci.c_add = zones->stroke_width;
    ci.c_factor = ci.sb_factor = 100;
    StemInfosFree(sc->vstem); sc->vstem = NULL;
    SCCondenseExtend(&ci,sc,ly_fore,false);
}

static int NumberLayerPoints(SplineSet *ss) {
    int cnt;
    SplinePoint *pt;

    cnt = 1;
    for ( ; ss!=NULL; ss=ss->next ) {
	for ( pt=ss->first; ; ) {
	    pt->ptindex = cnt++;
	    if ( pt->next==NULL )
	break;
	    pt = pt->next->to;
	    if ( pt==ss->first )
	break;
	}
    }
return( cnt );
}
	    
static SplineSet *BoldSSStroke(SplineSet *ss,StrokeInfo *si,SplineChar *sc,int ro) {
    SplineSet *temp;
    Spline *s1, *s2;

    /* We don't want to use the remove overlap built into SSStroke because */
    /*  only looks at one contour at a time, but we need to look at all together */
    /*  else we might get some unreal internal hints that will screw up */
    /*  counter correction */
    temp = SSStroke(ss,si,sc);
    if ( ro && temp!=NULL && SplineSetIntersect(temp,&s1,&s2))
	temp = SplineSetRemoveOverlap(sc,temp,over_remove);
return( temp );
}

static void SCEmbolden(SplineChar *sc, struct lcg_zones *zones, int layer) {
    StrokeInfo si;
    SplineSet *temp;
    DBounds old, new;
    int adjust_counters;

    memset(&si,0,sizeof(si));
    si.stroke_type = si_std;
    si.join = lj_miter;
    si.cap = lc_square;
    if ( zones->stroke_width>=0 ) {
	si.radius = zones->stroke_width/2.;
	si.removeinternal = true;
    } else {
	si.radius = -zones->stroke_width/2.;
	si.removeexternal = true;
    }
    si.removeoverlapifneeded = false;
    si.toobigwarn = true;

    if ( layer!=ly_back && zones->wants_hints &&
	    sc->hstem == NULL && sc->vstem==NULL && sc->dstem==NULL ) {
	_SplineCharAutoHint(sc,layer,&zones->bd,NULL,false);
    }

    adjust_counters = zones->counter_type==ct_retain ||
	    (zones->counter_type==ct_auto &&
		zones->embolden_hook==LCG_HintedEmboldenHook &&
		sc->width>0 );

    if ( layer==ly_all ) {
	SCPreserveState(sc,false);
	SplineCharFindBounds(sc,&old);
	for ( layer = ly_fore; layer<sc->layer_cnt; ++layer ) {
	    NumberLayerPoints(sc->layers[layer].splines);
	    temp = BoldSSStroke(sc->layers[layer].splines,&si,sc,zones->removeoverlap);
	    if ( zones->embolden_hook!=NULL )
		temp = (zones->embolden_hook)(temp,zones,sc,layer);
	    SplinePointListsFree( sc->layers[layer].splines );
	    sc->layers[layer].splines = temp;
	}
	SplineCharFindBounds(sc,&new);
	if ( adjust_counters )
	    AdjustCounters(sc,zones,&old,&new);
	layer = ly_all;
    } else if ( layer>=0 ) {
	SCPreserveLayer(sc,layer,false);
	NumberLayerPoints(sc->layers[layer].splines);
	SplineSetFindBounds(sc->layers[layer].splines,&old);
	temp = BoldSSStroke(sc->layers[layer].splines,&si,sc,zones->removeoverlap);
	if ( zones->embolden_hook!=NULL )
	    temp = (zones->embolden_hook)(temp,zones,sc,layer);
	SplineSetFindBounds(temp,&new);
	SplinePointListsFree( sc->layers[layer].splines );
	sc->layers[layer].splines = temp;
	if ( adjust_counters && layer==ly_fore )
	    AdjustCounters(sc,zones,&old,&new);
    }

    if ( layer!=ly_back ) {
	/* Hints will be inccorrect (misleading) after these transformations */
	StemInfosFree(sc->vstem); sc->vstem=NULL;
	StemInfosFree(sc->hstem); sc->hstem=NULL;
	DStemInfosFree(sc->dstem); sc->dstem=NULL;
	SCOutOfDateBackground(sc);
    }
    SCCharChangedUpdate(sc,layer);
}

static struct {
    uint32 script;
    SplineSet *(*embolden_hook)(SplineSet *,struct lcg_zones *,SplineChar *, int layer);
} script_hooks[] = {
    { CHR('l','a','t','n'), LCG_HintedEmboldenHook },
    { CHR('c','y','r','l'), LCG_HintedEmboldenHook },
    { CHR('g','r','e','k'), LCG_HintedEmboldenHook },
	/* Hebrew probably works too */
    { CHR('h','e','b','r'), LCG_HintedEmboldenHook },
    0
};

static struct {
    unichar_t from, to;
    SplineSet *(*embolden_hook)(SplineSet *,struct lcg_zones *,SplineChar *, int layer);
} char_hooks[] = {
    { '0','9', LCG_HintedEmboldenHook },
    { '$','%', LCG_HintedEmboldenHook },
    0
};

static void LCG_ZoneInit(SplineFont *sf, int layer, struct lcg_zones *zones,enum embolden_type type) {

    if ( type == embolden_lcg || type == embolden_custom) {
	zones->embolden_hook = LCG_HintedEmboldenHook;
    } else {
	zones->embolden_hook = NULL;
    }
    QuickBlues(sf, layer, &zones->bd);
    zones->stdvw = SFStdVW(sf);
}

static double BlueSearch(char *bluestring, double value, double bestvalue) {
    char *end;
    double try, diff, bestdiff;

    if ( *bluestring=='[' ) ++bluestring;
    if ( (bestdiff = bestvalue-value)<0 ) bestdiff = -bestdiff;

    forever {
	try = strtod(bluestring,&end);
	if ( bluestring==end )
return( bestvalue );
	if ( (diff = try-value)<0 ) diff = -diff;
	if ( diff<bestdiff ) {
	    bestdiff = diff;
	    bestvalue = try;
	}
	bluestring = end;
	(void) strtod(bluestring,&end);		/* Skip the top of blue zone value */
	bluestring = end;
    }
}

static double SearchBlues(SplineFont *sf,int type,double value) {
    char *blues, *others;
    double bestvalue;

    if ( type=='x' )
	value = sf->ascent/2;		/* Guess that the x-height is about half the ascent and then see what we find */
    if ( type=='I' )
	value = 4*sf->ascent/5;		/* Guess that the cap-height is 4/5 the ascent */

    blues = others = NULL;
    if ( sf->private!=NULL ) {
	blues = PSDictHasEntry(sf->private,"BlueValues");
	others = PSDictHasEntry(sf->private,"OtherBlues");
    }
    bestvalue = 0x100000;		/* Random number outside coord range */
    if ( blues!=NULL )
	bestvalue = BlueSearch(blues,value,bestvalue);
    if ( others!=NULL )
	bestvalue = BlueSearch(others,value,bestvalue);
    if ( bestvalue == 0x100000 )
return( value );

return( bestvalue );
}

double SFSerifHeight(SplineFont *sf) {
    SplineChar *isc;
    SplineSet *ss;
    SplinePoint *sp;
    DBounds b;

    if ( sf->strokedfont || sf->multilayer )
return( 0 );

    isc = SFGetChar(sf,'I',NULL);
    if ( isc==NULL )
	isc = SFGetChar(sf,0x0399,"Iota");
    if ( isc==NULL )
	isc = SFGetChar(sf,0x0406,NULL);
    if ( isc==NULL )
return( 0 );

    ss = isc->layers[ly_fore].splines;
    if ( ss==NULL || ss->next!=NULL )		/* Too complicated, probably doesn't have simple serifs (black letter?) */
return( 0 );
    if ( ss->first->prev==NULL )
return( 0 );
    for ( sp=ss->first; ; ) {
	if ( sp->me.y==0 )
    break;
	sp = sp->next->to;
	if ( sp==ss->first )
    break;
    }
    if ( sp->me.y!=0 )
return( 0 );
    SplineCharFindBounds(isc,&b);
    if ( sp->next->to->me.y==0 || sp->next->to->next->to->me.y==0 ) {
	SplinePoint *psp = sp->prev->from;
	if ( psp->me.y>=b.maxy/3 )
return( 0 );			/* Sans Serif, probably */
	if ( !psp->nonextcp && psp->nextcp.x==psp->me.x ) {
	    /* A curve point half-way up the serif? */
	    psp = psp->prev->from;
	    if ( psp->me.y>=b.maxy/3 )
return( 0 );			/* I give up, I don't understand this */
	}
return( psp->me.y );
    } else if ( sp->prev->from->me.y==0 || sp->prev->from->prev->from->me.y==0 ) {
	SplinePoint *nsp = sp->next->to;
	if ( nsp->me.y>=b.maxy/3 )
return( 0 );			/* Sans Serif, probably */
	if ( !nsp->nonextcp && nsp->nextcp.x==nsp->me.x ) {
	    /* A curve point half-way up the serif? */
	    nsp = nsp->next->to;
	    if ( nsp->me.y>=b.maxy/3 )
return( 0 );			/* I give up, I don't understand this */
	}
return( nsp->me.y );
    }

    /* Too complex for me */
return( 0 );
}

static void PerGlyphInit(SplineChar *sc, struct lcg_zones *zones,
	enum embolden_type type) {
    int j;
    SplineChar *hebrew;

    if ( type == embolden_auto ) {
	zones->embolden_hook = NULL;
	for ( j=0; char_hooks[j].from!=0; ++j ) {
	    if ( sc->unicodeenc>=char_hooks[j].from && sc->unicodeenc<=char_hooks[j].to ) {
		zones->embolden_hook = char_hooks[j].embolden_hook;
	break;
	    }
	}
	if ( zones->embolden_hook == NULL ) {
	    uint32 script = SCScriptFromUnicode(sc);
	    for ( j=0; script_hooks[j].script!=0; ++j ) {
		if ( script==script_hooks[j].script ) {
		    zones->embolden_hook = script_hooks[j].embolden_hook;
	    break;
		}
	    }
	}
    }
    if ( type == embolden_lcg || type == embolden_auto ) {
	zones->bottom_bound = 0;
	if ( SCScriptFromUnicode(sc)==CHR('h','e','b','r') &&
		(hebrew=SFGetChar(sc->parent,0x05df,NULL))!=NULL ) {
	    DBounds b;
	    SplineCharFindBounds(hebrew,&b);
	    zones->bottom_zone = b.maxy/3;
	    zones->top_zone = 2*b.maxy/3;
	    zones->top_bound = b.maxy;
	} else if ( sc->unicodeenc!=-1 && sc->unicodeenc<0x10000 && islower(sc->unicodeenc)) {
	    if ( zones->bd.xheight<=0 )
		zones->bd.xheight = SearchBlues(sc->parent,'x',0);
	    zones->bottom_zone = zones->bd.xheight>0 ? zones->bd.xheight/3 :
			    zones->bd.caph>0 ? zones->bd.caph/3 :
			    (sc->parent->ascent/4);
	    zones->top_zone = zones->bd.xheight>0 ? 2*zones->bd.xheight/3 :
			    zones->bd.caph>0 ? zones->bd.caph/2 :
			    (sc->parent->ascent/3);
	    zones->top_bound = zones->bd.xheight>0 ? zones->bd.xheight :
			    zones->bd.caph>0 ? 2*zones->bd.caph/3 :
			    (sc->parent->ascent/2);
	} else if ( sc->unicodeenc!=-1 && sc->unicodeenc<0x10000 && isupper(sc->unicodeenc)) {
	    if ( zones->bd.caph<0 )
		zones->bd.caph = SearchBlues(sc->parent,'I',0);
	    zones->bottom_zone = zones->bd.caph>0 ? zones->bd.caph/3 :
			    (sc->parent->ascent/4);
	    zones->top_zone = zones->bd.caph>0 ? 2*zones->bd.caph/3 :
			    (sc->parent->ascent/2);
	    zones->top_bound = zones->bd.caph>0?zones->bd.caph:4*sc->parent->ascent/5;
	} else {
	    /* It's not upper case. It's not lower case. Hmm. Look for blue */
	    /*  values near the top and bottom of the glyph */
	    DBounds b;

	    SplineCharFindBounds(sc,&b);
	    zones->top_bound = SearchBlues(sc->parent,0,b.maxy);
	    zones->bottom_bound = SearchBlues(sc->parent,-1,b.miny);
	    zones->top_zone = zones->bottom_bound + 3*(zones->top_bound-zones->bottom_bound)/4;
	    zones->bottom_zone = zones->bottom_bound + (zones->top_bound-zones->bottom_bound)/4;
	}
    }
    zones->wants_hints = zones->embolden_hook == LCG_HintedEmboldenHook;
}

void FVEmbolden(FontViewBase *fv,enum embolden_type type,struct lcg_zones *zones) {
    int i, gid;
    SplineChar *sc;

    LCG_ZoneInit(fv->sf,fv->active_layer,zones,type);

    for ( i=0; i<fv->map->enccount; ++i ) if ( fv->selected[i] &&
	    (gid = fv->map->map[i])!=-1 && (sc=fv->sf->glyphs[gid])!=NULL ) {
	PerGlyphInit(sc,zones,type);
	SCEmbolden(sc, zones, -2);		/* -2 => all foreground layers */
    }
}

void CVEmbolden(CharViewBase *cv,enum embolden_type type,struct lcg_zones *zones) {
    SplineChar *sc = cv->sc;

    if ( cv->drawmode == dm_grid )
return;

    LCG_ZoneInit(sc->parent,CVLayer(cv),zones,type);

    PerGlyphInit(sc,zones,type);
    SCEmbolden(sc, zones, CVLayer(cv));
}

void ScriptSCEmbolden(SplineChar *sc,int layer,enum embolden_type type,struct lcg_zones *zones) {

    LCG_ZoneInit(sc->parent,layer,zones,type);

    PerGlyphInit(sc,zones,type);
    SCEmbolden(sc, zones, layer);
}

/* ************************************************************************** */
/* ***************************** Small Capitals ***************************** */
/* ************************************************************************** */


extern int autohint_before_generate;

struct smallcaps {
    double lc_stem_width, uc_stem_width;
    double stem_factor;
    double xheight, capheight;
    SplineFont *sf;
    int layer;
    double italic_angle, tan_ia;
};

static double MajorVerticalStemWidth(SplineChar *sc, int layer) {
    double best_len, best_width;
    StemInfo *s;

    if ( sc==NULL )
return( -1 );
    if ( autohint_before_generate && sc->changedsincelasthinted &&
	    !sc->manualhints )
	SplineCharAutoHint(sc,layer,NULL);
    best_len = 0; best_width = -1;
    for ( s = sc->vstem; s!=NULL; s=s->next ) if ( !s->ghost ) {
	double len = HIlen(s);
	if ( len>best_len )
	    best_width = s->width;
    }
return( best_width );
}

/* Ok, we couldn't hint it, or it was italic. Make a copy of it, deskew it, */
/*  and hint that */
static double ReallyMajorVerticalStemWidth(SplineChar *sc, int layer,double tan_ia) {
    double best_len, best_width;
    StemInfo *s;
    SplineChar dummy;
    Layer layers[2];
    real deskew[6];

    if ( sc==NULL )
return( -1 );
    memset(deskew,0,sizeof(deskew));
    deskew[0] = deskew[3] = 1;
    deskew[2] = tan_ia;
    memset(&dummy,0,sizeof(dummy));
    memset(layers,0,sizeof(layers));
    dummy.color = COLOR_DEFAULT;
    dummy.layer_cnt = 2;
    dummy.layers = layers;
    dummy.parent = sc->parent;
    dummy.name = copy("Fake");

    dummy.layers[ly_fore].order2 = sc->layers[layer].order2;
    dummy.layers[ly_fore].splines = SplinePointListTransform(
	    SplinePointListCopy(LayerAllSplines(&sc->layers[layer])),
	    deskew,true);
    LayerUnAllSplines(&sc->layers[ly_fore]);
    
    SplineCharAutoHint(&dummy,ly_fore,NULL);
    best_len = 0; best_width = -1;
    for ( s = dummy.vstem; s!=NULL; s=s->next ) if ( !s->ghost ) {
	double len = HIlen(s);
	if ( len>best_len )
	    best_width = s->width;
    }
    SCClearContents(&dummy,ly_fore);
return( best_width );
}

static double CharHeight(SplineChar *sc, int layer) {
    DBounds b;

    if ( sc==NULL )
return( 0 );

    SplineCharLayerFindBounds(sc,layer,&b);
return( b.maxy );
}

static void SmallCapsFindConstants(struct smallcaps *small, SplineFont *sf,int layer) {
    int i, cnt;
    double sum, val;
    static unichar_t lc_stem_str[] = { 'l', 'l', 'l', 'm', 'f', 't', 0x438, 0x43D,
	    0x43f, 0x448, 0x3c0, 0x3bc, 0 };
    static unichar_t uc_stem_str[] = { 'I', 'L', 'T', 'H', 0x3a0, 0x397, 0x399,
	    0x406, 0x418, 0x41d, 0x41f, 0x422, 0x428, 0 };
    static unichar_t xheight_str[] = { 'x', 'u', 'v', 'w', 'y', 'z', 0x3b9, 0x3ba,
	    0x3bc, 0x3c0, 0x3c7, 0x438, 0x43f, 0x43d, 0x442, 0x448, 0 };
    static unichar_t capheight_str[] = { 'X', 'I', 'L', 'U', 'V', 'W', 0x397,
	    0x399, 0x3a0, 0x3a4, 0x406, 0x408, 0x418, 0x41d, 0x41f, 0x422, 0 };

    memset(small,0,sizeof(*small));

    small->sf = sf; small->layer = layer;
    small->italic_angle = sf->italicangle * 3.1415926535897932/180.0;
    small->tan_ia = tan( small->italic_angle );

    cnt = 0;
    small->lc_stem_width = small->uc_stem_width = -1;
    if ( small->tan_ia==0 ) {
	for ( i=0, sum=0; lc_stem_str[i]!=0; ++i ) {
	    val = MajorVerticalStemWidth(SFGetChar(sf,lc_stem_str[i],NULL),layer);
	    if ( val!=-1 ) {
		sum += val;
		++cnt;
	    }
	}
	if ( cnt!=0 )
	    small->lc_stem_width = sum/cnt;
    }
    if ( cnt==0 ) {
	for ( i=0, sum=0; lc_stem_str[i]!=0 ; ++i ) {
	    val = ReallyMajorVerticalStemWidth(SFGetChar(sf,lc_stem_str[i],NULL),layer, small->tan_ia);
	    if ( val!=-1 ) {
		small->lc_stem_width = val;
	break;
	    }
	}
    }

    cnt = 0;
    if ( small->tan_ia==0 ) {
	for ( i=0, sum=0; uc_stem_str[i]!=0; ++i ) {
	    val = MajorVerticalStemWidth(SFGetChar(sf,uc_stem_str[i],NULL),layer);
	    if ( val!=-1 ) {
		sum += val;
		++cnt;
	    }
	}
	if ( cnt!=0 )
	    small->uc_stem_width = sum/cnt;
    }
    if ( cnt==0 ) {
	for ( i=0, sum=0; uc_stem_str[i]!=0 ; ++i ) {
	    val = ReallyMajorVerticalStemWidth(SFGetChar(sf,uc_stem_str[i],NULL),layer, small->tan_ia);
	    if ( val!=-1 ) {
		small->uc_stem_width = val;
	break;
	    }
	}
    }
    if ( small->uc_stem_width<=small->lc_stem_width || small->lc_stem_width==-1 )
	small->stem_factor = 1;
    else
	small->stem_factor = small->lc_stem_width / small->uc_stem_width;

    for ( i=cnt=0, sum=0; xheight_str[i]!=0; ++i ) {
	val = CharHeight(SFGetChar(sf,xheight_str[i],NULL),layer);
	if ( val>0 ) {
	    sum += val;
	    ++cnt;
	}
    }
    small->xheight = cnt==0 ? 0 : sum/cnt;

    for ( i=cnt=0, sum=0; capheight_str[i]!=0; ++i ) {
	val = CharHeight(SFGetChar(sf,capheight_str[i],NULL),layer);
	if ( val>0 ) {
	    sum += val;
	    ++cnt;
	}
    }
    small->capheight = cnt==0 ? 0 : sum/cnt;
}

static void MakeLookups(SplineFont *sf,OTLookup **lookups,int ltn,int crl,int grk,
	uint32 ftag) {
    OTLookup *any = NULL;
    int i;
    struct lookup_subtable *sub;

    for ( i=0; i<3; ++i ) {
	if ( any==NULL )
	    any = lookups[i];
	else if ( lookups[i]!=NULL && lookups[i]!=any )
	    any = (OTLookup *) -1;
    }

    if ( any==(OTLookup *) -1 ) {
	/* Each script has it's own lookup. So if we are missing a script we */
	/*  should create a new lookup for it */
	if ( lookups[0]==NULL && ltn ) {
	    sub = SFSubTableFindOrMake(sf,ftag,CHR('l','a','t','n'),gsub_single);
	    lookups[0] = sub->lookup;
	}
	if ( lookups[1]==NULL && crl ) {
	    sub = SFSubTableFindOrMake(sf,ftag,CHR('c','y','r','l'),gsub_single);
	    lookups[1] = sub->lookup;
	}
	if ( lookups[2]==NULL && grk ) {
	    sub = SFSubTableFindOrMake(sf,ftag,CHR('g','r','e','k'),gsub_single);
	    lookups[2] = sub->lookup;
	}
    } else {
	if ( any!=NULL ) {
	    /* There's only one lookup, let's extend it to deal with any script */
	    /*  we need for which there is no lookup */
	} else {
	    /* No lookup. Create one for all the scripts we need */
	    sub = SFSubTableFindOrMake(sf,ftag,
		    ltn?CHR('l','a','t','n'):
		    crl?CHR('c','y','r','l'):
			CHR('g','r','e','k'),gsub_single);
	    any = sub->lookup;
	}
	if ( lookups[0]==NULL && ltn ) {
	    lookups[0] = any;
	    FListAppendScriptLang(FindFeatureTagInFeatureScriptList(ftag,any->features),CHR('l','a','t','n'),DEFAULT_LANG);
	}
	if ( lookups[1]==NULL && crl ) {
	    lookups[1] = any;
	    FListAppendScriptLang(FindFeatureTagInFeatureScriptList(ftag,any->features),CHR('c','y','r','l'),DEFAULT_LANG);
	}
	if ( lookups[2]==NULL && grk ) {
	    lookups[2] = any;
	    FListAppendScriptLang(FindFeatureTagInFeatureScriptList(ftag,any->features),CHR('g','r','e','k'),DEFAULT_LANG);
	}
    }
    for ( i=0; i<3; ++i ) {
	if ( lookups[i]!=NULL && lookups[i]->subtables==NULL ) {
	    lookups[i]->subtables = chunkalloc(sizeof(struct lookup_subtable));
	    lookups[i]->subtables->lookup = lookups[i];
	    lookups[i]->subtables->per_glyph_pst_or_kern = true;
	    NameOTLookup(lookups[i],sf);
	}
    }
}

static void MakeSCLookups(SplineFont *sf,struct lookup_subtable **c2sc,
	struct lookup_subtable **smcp,
	int ltn,int crl,int grk) {
    OTLookup *test;
    FeatureScriptLangList *fl;
    struct scriptlanglist *sl;
    OTLookup *lc2sc[3], *lsmcp[3];
    int i;

    memset(lc2sc,0,sizeof(lc2sc)); memset(lsmcp,0,sizeof(lsmcp));

    if ( sf->cidmaster ) sf=sf->cidmaster;
    for ( test=sf->gsub_lookups; test!=NULL; test=test->next ) if ( test->lookup_type==gsub_single ) {
	for ( fl=test->features; fl!=NULL; fl=fl->next ) {
	    if ( fl->featuretag==CHR('c','2','s','c') || fl->featuretag==CHR('s','m','c','p')) {
		for ( sl=fl->scripts; sl!=NULL; sl=sl->next ) {
		    if ( sl->script==CHR('l','a','t','n')) {
			if ( fl->featuretag==CHR('c','2','s','c') )
			    lc2sc[0] = test;
			else
			    lsmcp[0] = test;
		    } else if ( sl->script==CHR('c','y','r','l')) {
			if ( fl->featuretag==CHR('c','2','s','c') )
			    lc2sc[1] = test;
			else
			    lsmcp[1] = test;
		    } else if ( sl->script==CHR('g','r','e','k')) {
			if ( fl->featuretag==CHR('c','2','s','c') )
			    lc2sc[2] = test;
			else
			    lsmcp[2] = test;
		    }
		}
	    }
	}
    }

    MakeLookups(sf,lc2sc,ltn,crl,grk,CHR('c','2','s','c'));
    MakeLookups(sf,lsmcp,ltn,crl,grk,CHR('s','c','m','p'));

    for ( i=0; i<3; ++i ) {
	if ( lc2sc[i]!=NULL )
	    c2sc[i] = lc2sc[i]->subtables;
	if ( lsmcp[i]!=NULL )
	    smcp[i] = lsmcp[i]->subtables;
    }
}

static SplineChar *MakeSmallCapGlyphSlot(SplineFont *sf,SplineChar *cap_sc,
	uint32 script,struct lookup_subtable **c2sc,struct lookup_subtable **smcp,
	FontViewBase *fv) {
    SplineChar *sc_sc, *lc_sc;
    char buffer[200];
    PST *pst;
    int enc;

    lc_sc = SFGetChar(sf,tolower(cap_sc->unicodeenc),NULL);
    if ( lc_sc!=NULL )
	snprintf(buffer,sizeof(buffer),"%s.sc", lc_sc->name );
    else {
	const char *pt = StdGlyphName(buffer,tolower(cap_sc->unicodeenc),sf->uni_interp,sf->for_new_glyphs);
	if ( pt!=buffer )
	    strcpy(buffer,pt);
	strcat(buffer,".sc");
    }
    sc_sc = SFGetChar(sf,-1,buffer);
    if ( sc_sc!=NULL ) {
	SCPreserveLayer(sc_sc,fv->active_layer,false);
	SCClearLayer(sc_sc,fv->active_layer);
return( sc_sc );
    }
    enc = SFFindSlot(sf, fv->map, -1, buffer );
    if ( enc==-1 )
	enc = fv->map->enccount;
    sc_sc = SFMakeChar(sf,fv->map,enc);
    free(sc_sc->name);
    sc_sc->name = copy(buffer);

    pst = chunkalloc(sizeof(PST));
    pst->next = cap_sc->possub;
    cap_sc->possub = pst;
    pst->subtable = c2sc[script==CHR('l','a','t','n')?0:script==CHR('c','y','r','l')?1:2];
    pst->type = pst_substitution;
    pst->u.subs.variant = copy(buffer);

    if ( lc_sc!=NULL ) {
	pst = chunkalloc(sizeof(PST));
	pst->next = lc_sc->possub;
	lc_sc->possub = pst;
	pst->subtable = smcp[script==CHR('l','a','t','n')?0:script==CHR('c','y','r','l')?1:2];
	pst->type = pst_substitution;
	pst->u.subs.variant = copy(buffer);
    }
return( sc_sc );
}

static void SmallCapsRemoveSpace(SplineSet *ss,AnchorPoint *aps,StemInfo *hints,int coord,double remove,
	double min_coord, double max_coord) {
    struct overlaps { double start, stop, new_start, new_stop; } *overlaps;
    struct ptpos { double old, new; int hint_index; } *ptpos;
    int cnt, i, j, tot, order2, set;
    StemInfo *h;
    double counter_len, val, shrink;
    SplineSet *spl;
    SplinePoint *sp, *first, *start, *last;
    AnchorPoint *ap;

    if ( remove > max_coord-min_coord )
return;

    /* Coalesce overlapping hint zones. These won't shrink, but the counters */
    /*  between them will */
    for ( h=hints, cnt=0; h!=NULL; h=h->next ) if ( !h->ghost )
	++cnt;

    overlaps = galloc((cnt+3)*sizeof(struct overlaps));
    overlaps[0].start = min_coord; overlaps[0].stop = min_coord;
    overlaps[1].start = max_coord; overlaps[1].stop = max_coord;
    tot = 2;

    for ( h=hints; h!=NULL; h=h->next ) if ( !h->ghost ) {
	for ( i=0; i<tot && overlaps[i].stop<h->start; ++i );
	if ( i==tot )	/* Can't happen */
    continue;
	/* So h->start<=overlaps[i].stop */
	if ( h->start+h->width<overlaps[i].start ) {
	    /* New entry */
	    for ( j=tot; j>i; --j )
		overlaps[j] = overlaps[j-1];
	    overlaps[i].start = h->start;
	    overlaps[i].stop  = h->start+h->width;
	    ++tot;
	} else {
	    if ( h->start<overlaps[i].start )
		overlaps[i].start = h->start;
	    if ( h->start+h->width > overlaps[i].stop )
		overlaps[i].stop = h->start+h->width;
	    while ( i+1<tot && overlaps[i].stop>=overlaps[i+1].start ) {
		overlaps[i].stop = overlaps[i+1].stop;
		--tot;
		for ( j=i+1; j<tot; ++j )
		    overlaps[j] = overlaps[j+1];
	    }
	}
    }
    for ( i=0, counter_len=0; i<tot-1; ++i )
	counter_len += overlaps[i+1].start-overlaps[i].stop;

    if ( 2*remove >counter_len ) {
	/* The amount we need to remove is disproportionate to the counter */
	/*  space we have available from which to remove it. So just scale */
	/*  everything linearly between min_coord and max */
	overlaps[0].start = min_coord-100; overlaps[0].stop = min_coord;
	overlaps[1].start = max_coord    ; overlaps[1].stop = max_coord+100;
	tot = 2;
	counter_len = max_coord - min_coord;
    }
    if ( counter_len==0 || counter_len<remove ) {
	free( overlaps );
return;
    }

    shrink = (counter_len-remove)/counter_len;
    /* 0 is a fixed point */
    for ( i=0; i<tot && overlaps[i].stop<0; ++i );
    if ( i==tot ) {
	/* glyph is entirely <0 */
	set = tot-1;
	overlaps[set].new_stop = shrink*overlaps[set].stop;
	overlaps[set].new_start = overlaps[set].new_stop - (overlaps[set].stop - overlaps[set].start);
    } else if ( overlaps[i].start>0 ) {
	set = i;
	overlaps[set].new_start = shrink*overlaps[set].start;
	overlaps[set].new_stop  = overlaps[set].new_start + (overlaps[set].stop - overlaps[set].start);
    } else {
	set = i;
	overlaps[set].new_start = overlaps[set].start;
	overlaps[set].new_stop = overlaps[set].stop;
    }
    for ( i=set+1; i<tot; ++i ) {
	overlaps[i].new_start = overlaps[i-1].new_stop +
		(overlaps[i].start - overlaps[i-1].stop)*shrink;
	overlaps[i].new_stop  = overlaps[i].new_start +
		(overlaps[i].stop  -overlaps[i].start);
    }
    for ( i=set-1; i>=0; --i ) {
	overlaps[i].new_stop = overlaps[i+1].new_start -
		(overlaps[i+1].start - overlaps[i].stop)*shrink;
	overlaps[i].new_start = overlaps[i].new_stop - (overlaps[i].stop - overlaps[i].start);
    }

    cnt = NumberLayerPoints(ss);
    ptpos = gcalloc(cnt,sizeof(struct ptpos));

    /* Position any points which lie within a hint zone */
    order2 = false;
    for ( spl=ss; spl!=NULL; spl=spl->next ) {
	for ( sp=spl->first; ; ) {
	    sp->ticked = false;
	    val = (&sp->me.x)[coord];
	    ptpos[sp->ptindex].old = val;
	    ptpos[sp->ptindex].hint_index = -2;
	    for ( i=0; i<tot; ++i ) {
		if ( val>=overlaps[i].start && val<=overlaps[i].stop ) {
		    for ( h=hints; h!=NULL; h=h->next ) {
			if ( RealNear(val,h->start) || RealNear(val,h->start+h->width)) {
			    sp->ticked = true;
			    ptpos[sp->ptindex].hint_index = i;
			    ptpos[sp->ptindex].new = overlaps[i].new_start +
				    (val-overlaps[i].start);
		    break;
			}
		    }
	    break;
		}
	    }
	    if ( sp->next==NULL )
	break;
	    order2 = sp->next->order2;
	    sp = sp->next->to;
	    if ( sp==spl->first )
	break;
	}
    }

    /* Look for any local minimum or maximum points */
    for ( spl=ss; spl!=NULL; spl=spl->next ) {
	for ( sp=spl->first; ; ) {
	    if ( sp->next==NULL )
	break;
	    val = (&sp->me.x)[coord];
	    if ( !sp->ticked && sp->prev!=NULL &&
		    ((val>=(&sp->prev->from->me.x)[coord] && val>=(&sp->next->to->me.x)[coord]) ||
		     (val<=(&sp->prev->from->me.x)[coord] && val<=(&sp->next->to->me.x)[coord]))) {
		for ( i=0; i<tot; ++i ) {
		    if ( val>=overlaps[i].start && val<overlaps[i].stop ) {
			sp->ticked = true;
			ptpos[sp->ptindex].new = overlaps[i].new_start +
				(val-overlaps[i].start) *
				    (overlaps[i].new_stop - overlaps[i].new_start)/
				    (overlaps[i].stop - overlaps[i].start);
		break;
		    } else if ( i>0 && val>=overlaps[i-1].stop && val<=overlaps[i].start ) {
			sp->ticked = true;
			ptpos[sp->ptindex].new = overlaps[i-1].new_stop +
				(val-overlaps[i-1].stop) *
				    (overlaps[i].new_start - overlaps[i-1].new_stop)/
				    (overlaps[i].start - overlaps[i-1].stop);
		break;
		    }
		}
	    }
	    sp = sp->next->to;
	    if ( sp==spl->first )
	break;
	}
    }

    /* Position any points which line between points already positioned */
    for ( spl=ss; spl!=NULL; spl=spl->next ) {
	for ( sp=spl->first; !sp->ticked; ) {
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==spl->first )
	break;
	}
	if ( !sp->ticked )		/* Nothing on this contour positioned */
    continue;
	first = sp;
	do {
	    start = sp;
	    if ( sp->next==NULL )
	break;
	    /* Find the next point which has been positioned */
	    for ( sp=sp->next->to; !sp->ticked; ) {
		if ( sp->next==NULL )
	    break;
		sp = sp->next->to;
		if ( sp==first )
	    break;
	    }
	    if ( !sp->ticked )
	break;
	    /* Interpolate any points between these two */
	    last = sp;
	    /* Make sure all the points are BETWEEN */
	    for ( sp=start->next->to; sp!=last; sp=sp->next->to ) {
		if (( (&sp->me.x)[coord] < (&start->me.x)[coord] && (&sp->me.x)[coord] < (&last->me.x)[coord]) ||
			( (&sp->me.x)[coord] > (&start->me.x)[coord] && (&sp->me.x)[coord] > (&last->me.x)[coord]))
	    break;
	    }
	    if ( sp==last ) {
		for ( sp=start->next->to; sp!=last; sp=sp->next->to ) {
		    ptpos[sp->ptindex].new = ptpos[start->ptindex].new +
			    ((&sp->me.x)[coord] - ptpos[start->ptindex].old) *
			      (ptpos[last->ptindex].new - ptpos[start->ptindex].new) /
			      (ptpos[last->ptindex].old - ptpos[start->ptindex].old);
		    sp->ticked = true;
		}
	    } else
		sp = last;
	} while ( sp!=first );
    }
	    
    /* Any points which aren't currently positioned, just interpolate them */
    /*  between the hint zones between which they lie */
    /* I don't think this can actually happen... but do it just in case */
    for ( spl=ss; spl!=NULL; spl=spl->next ) {
	for ( sp=spl->first; ; ) {
	    if ( !sp->ticked ) {
		val = (&sp->me.x)[coord];
		for ( i=0; i<tot; ++i ) {
		    if ( val>=overlaps[i].start && val<overlaps[i].stop ) {
			sp->ticked = true;
			ptpos[sp->ptindex].new = overlaps[i].new_start +
				(val-overlaps[i].start) *
				    (overlaps[i].new_stop - overlaps[i].new_start)/
				    (overlaps[i].stop - overlaps[i].start);
		break;
		    } else if ( i>0 && val>=overlaps[i-1].stop && val<=overlaps[i].start ) {
			sp->ticked = true;
			ptpos[sp->ptindex].new = overlaps[i-1].new_stop +
				(val-overlaps[i-1].stop) *
				    (overlaps[i].new_start - overlaps[i-1].new_stop)/
				    (overlaps[i].start - overlaps[i-1].stop);
		break;
		    }
		}
		if ( !sp->ticked ) {
		    IError( "Unticked point in smallcaps" );
		    ptpos[sp->ptindex].new = ptpos[sp->ptindex].old;
		}
	    }
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==spl->first )
	break;
	}
    }
    /* And do the same for anchor points */
    for ( ap=aps; ap!=NULL; ap=ap->next ) {
	val = (&ap->me.x)[coord];
	for ( i=0; i<tot; ++i ) {
	    if ( val>=overlaps[i].start && val<overlaps[i].stop ) {
		(&ap->me.x)[coord] = overlaps[i].new_start +
			(val-overlaps[i].start) *
			    (overlaps[i].new_stop - overlaps[i].new_start)/
			    (overlaps[i].stop - overlaps[i].start);
	break;
	    } else if ( i>0 && val>=overlaps[i-1].stop && val<=overlaps[i].start ) {
		(&ap->me.x)[coord] = overlaps[i-1].new_stop +
			(val-overlaps[i-1].stop) *
			    (overlaps[i].new_start - overlaps[i-1].new_stop)/
			    (overlaps[i].start - overlaps[i-1].stop);
	break;
	    }
	}
	/* Anchor points might be outside the bounding box */
	if ( i==tot ) {
	    if ( val<overlaps[0].start )
		(&ap->me.x)[coord] += overlaps[0].new_start - overlaps[0].start;
	    else
		(&ap->me.x)[coord] += overlaps[tot-1].new_stop - overlaps[tot-1].stop;
	}
    }

    /* Interpolate the control points. More complex in order2. We want to */
    /*  preserve interpolated points, but simplified as we only have one cp */
    if ( !order2 ) {
	for ( spl=ss; spl!=NULL; spl=spl->next ) {
	    for ( sp=spl->first; ; ) {
		if ( sp->prev!=NULL ) {
		    if ( ptpos[sp->prev->from->ptindex].old == ptpos[sp->ptindex].old )
			(&sp->prevcp.x)[coord] = ptpos[sp->ptindex].new;
		    else
			(&sp->prevcp.x)[coord] = ptpos[sp->ptindex].new +
				((&sp->prevcp.x)[coord] - ptpos[sp->ptindex].old)*
				(ptpos[sp->prev->from->ptindex].new-ptpos[sp->ptindex].new)/
				(ptpos[sp->prev->from->ptindex].old-ptpos[sp->ptindex].old);
		}
		if ( sp->next==NULL )
	    break;
		if ( ptpos[sp->next->to->ptindex].old == ptpos[sp->ptindex].old )
		    (&sp->nextcp.x)[coord] = ptpos[sp->ptindex].new;
		else
		    (&sp->nextcp.x)[coord] = ptpos[sp->ptindex].new +
			    ((&sp->nextcp.x)[coord] - ptpos[sp->ptindex].old)*
			    (ptpos[sp->next->to->ptindex].new-ptpos[sp->ptindex].new)/
			    (ptpos[sp->next->to->ptindex].old-ptpos[sp->ptindex].old);
		sp = sp->next->to;
		if ( sp==spl->first )
	    break;
	    }
	}
    } else {
	for ( spl=ss; spl!=NULL; spl=spl->next ) {
	    for ( sp=spl->first; ; ) {
		sp->ticked = SPInterpolate(sp);
		if ( sp->next==NULL )
	    break;
		if ( ptpos[sp->next->to->ptindex].old == ptpos[sp->ptindex].old )
		    (&sp->nextcp.x)[coord] = ptpos[sp->ptindex].new;
		else
		    (&sp->nextcp.x)[coord] = ptpos[sp->ptindex].new +
			    ((&sp->nextcp.x)[coord] - ptpos[sp->ptindex].old)*
			    (ptpos[sp->next->to->ptindex].new-ptpos[sp->ptindex].new)/
			    (ptpos[sp->next->to->ptindex].old-ptpos[sp->ptindex].old);
		(&sp->next->to->prevcp.x)[coord] = (&sp->nextcp.x)[coord];
		sp = sp->next->to;
		if ( sp==spl->first )
	    break;
	    }
	    for ( sp=spl->first; ; ) {
		if ( sp->ticked ) {
		    ptpos[sp->ptindex].new = ((&sp->nextcp.x)[coord] + (&sp->prevcp.x)[coord])/2;
		}
		if ( sp->next==NULL )
	    break;
		sp = sp->next->to;
		if ( sp==spl->first )
	    break;
	    }
	}
    }

    /* Finally move every point to its new location */
    for ( spl=ss; spl!=NULL; spl=spl->next ) {
	for ( sp=spl->first; ; ) {
	    (&sp->me.x)[coord] = ptpos[sp->ptindex].new;
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==spl->first )
	break;
	}
    }

    free(ptpos);
    free(overlaps);
}

static void SplineSetRefigure(SplineSet *ss) {
    Spline *s, *first;

    while ( ss!=NULL ) {
	first = NULL;
	for ( s=ss->first->next; s!=NULL && s!=first; s=s->to->next ) {
	    if ( first == NULL ) first = s;
	    SplineRefigure(s);
	}
	ss = ss->next;
    }
}

static void BuildSmallCap(SplineChar *sc_sc,SplineChar *cap_sc,int layer,
	struct smallcaps *small) {
    real scale[6];
    DBounds cap_b, sc_b;
    double remove_y, remove_x;
    extern int no_windowing_ui;
    int nwi = no_windowing_ui;
    AnchorPoint *ap;

    memset(scale,0,sizeof(scale));
    scale[0] = scale[3] = small->stem_factor;
    scale[2] = small->stem_factor * small->tan_ia;

    sc_sc->layers[layer].splines = SplinePointListTransform(SplinePointListCopy(
	    cap_sc->layers[layer].splines),scale,true);
    sc_sc->width = small->stem_factor * cap_sc->width;
    sc_sc->anchor = AnchorPointsCopy(cap_sc->anchor);
    for ( ap = sc_sc->anchor; ap!=NULL; ap=ap->next ) {
	BasePoint me;
	me.x = scale[0]*ap->me.x + scale[2]*ap->me.y + scale[4];
	me.y = scale[1]*ap->me.x + scale[3]*ap->me.y + scale[5];
	ap->me = me;
    }
    no_windowing_ui = true;		/* Turn off undoes */
    SplineCharAutoHint(sc_sc,layer,NULL);
    no_windowing_ui = nwi;
    if ( RealNear( small->stem_factor, small->xheight/small->capheight ))
return;

    SplineCharLayerFindBounds(cap_sc,layer,&cap_b);
    SplineCharLayerFindBounds(sc_sc,layer,&sc_b);
    remove_y = sc_b.maxy - small->xheight + small->stem_factor*(cap_b.maxy - small->capheight);
    if ( remove_y>-1 && remove_y<1 )
return;
    if ( cap_b.maxy == cap_b.miny )
	remove_x = 0;
    else
	remove_x = (sc_b.maxx - sc_b.minx) * remove_y / (cap_b.maxy - cap_b.miny);
    sc_sc->width -= remove_x;
    SmallCapsRemoveSpace(sc_sc->layers[layer].splines,sc_sc->anchor,sc_sc->vstem,0,remove_x,sc_b.minx,sc_b.maxx);
    SmallCapsRemoveSpace(sc_sc->layers[layer].splines,sc_sc->anchor,sc_sc->hstem,1,remove_y,sc_b.miny,sc_b.maxy);
    SplineSetRefigure(sc_sc->layers[layer].splines);

    if ( small->tan_ia!=0 ) {
	scale[0] = scale[3] = 1;
	scale[2] = -small->tan_ia;
	sc_sc->layers[layer].splines =
		SplinePointListTransform(sc_sc->layers[layer].splines,scale,true);
	for ( ap = sc_sc->anchor; ap!=NULL; ap=ap->next ) {
	    BasePoint me;
	    me.x = scale[0]*ap->me.x + scale[2]*ap->me.y + scale[4];
	    me.y = scale[1]*ap->me.x + scale[3]*ap->me.y + scale[5];
	    ap->me = me;
	}
    }
    StemInfosFree(sc_sc->hstem); sc_sc->hstem = NULL;
    StemInfosFree(sc_sc->vstem); sc_sc->vstem = NULL;
    SCRound2Int(sc_sc,layer, 1.0);		/* This calls SCCharChangedUpdate(sc_sc,layer); */
}

void FVAddSmallCaps(FontViewBase *fv) {
    int gid, enc, cnt, ltn,crl,grk;
    SplineFont *sf = fv->sf;
    SplineChar *sc, *sc_sc, *rsc;
    RefChar *ref, *r;
    struct lookup_subtable *c2sc[3], *smcp[3];
    char buffer[200];
    const unichar_t *alts;
    struct smallcaps small;

    if ( sf->cidmaster!=NULL )
return;		/* Can't randomly add things to a CID keyed font */
    for ( gid=0; gid<sf->glyphcnt; ++gid ) if ( (sc=sf->glyphs[gid])!=NULL )
	sc->ticked = false;
    cnt=ltn=crl=grk=0;
    memset(c2sc,0,sizeof(c2sc)); memset(smcp,0,sizeof(smcp));

    for ( enc=0; enc<fv->map->enccount; ++enc ) {
	if ( (gid=fv->map->map[enc])!=-1 && fv->selected[enc] && (sc=sf->glyphs[gid])!=NULL ) {
	    if ( sc->unicodeenc<0x10000 &&
		    (isupper(sc->unicodeenc) || islower(sc->unicodeenc)) ) {
		uint32 script = SCScriptFromUnicode(sc);
		if ( script==CHR('l','a','t','n'))
		    ++ltn, ++cnt;
		else if ( script==CHR('g','r','e','k'))
		    ++grk, ++cnt;
		else if ( script==CHR('c','y','r','l'))
		    ++crl, ++cnt;
	    }
	}
    }
    if ( cnt==0 )
return;
    SmallCapsFindConstants(&small,sf,fv->active_layer);
    if ( small.xheight==0 || small.capheight==0 ) {
	ff_post_error(_("Unknown scale"),_("Could not figure out scaling factor for small caps"));
return;
    }

    MakeSCLookups(sf,c2sc,smcp,ltn,crl,grk);
    ff_progress_start_indicator(10,_("Small Capitals"),
	_("Building small capitals"),NULL,cnt,1);
    for ( enc=0; enc<fv->map->enccount; ++enc ) {
	if ( (gid=fv->map->map[enc])!=-1 && fv->selected[enc] && (sc=sf->glyphs[gid])!=NULL ) {
	    if ( sc->unicodeenc<0x10000 &&
		    (isupper(sc->unicodeenc) || islower(sc->unicodeenc)) ) {
		uint32 script = SCScriptFromUnicode(sc);
		if ( script!=CHR('l','a','t','n') &&
			script!=CHR('g','r','e','k') &&
			script!=CHR('c','y','r','l'))
    continue;
		if ( islower(sc->unicodeenc)) {
		    sc = SFGetChar(sf,toupper(sc->unicodeenc),NULL);
		    if ( sc==NULL )
      goto end_loop;
		}
		if ( sc->ticked )
      goto end_loop;
		if ( sc->layers[fv->active_layer].refs!=NULL ||
			((alts = SFGetAlternate(sf,sc->unicodeenc,sc,true))!=NULL &&
			 alts[1]!=0 ))
    continue;	/* Deal with these later */
		sc->ticked = true;
		sc_sc = MakeSmallCapGlyphSlot(sf,sc,script,c2sc,smcp,fv);
		if ( sc_sc==NULL )
      goto end_loop;
		BuildSmallCap(sc_sc,sc,fv->active_layer,&small);
      end_loop:
		if ( !ff_progress_next())
    break;
	    }
	}
    }
    /* OK. Here we have done all the base glyphs we are going to do. Now let's*/
    /*  look at things which depend on references */
    for ( enc=0; enc<fv->map->enccount; ++enc ) {
	if ( (gid=fv->map->map[enc])!=-1 && fv->selected[enc] && (sc=sf->glyphs[gid])!=NULL ) {
	    if ( sc->unicodeenc<0x10000 &&
		    (isupper(sc->unicodeenc) || islower(sc->unicodeenc)) ) {
		uint32 script = SCScriptFromUnicode(sc);
		if ( script!=CHR('l','a','t','n') &&
			script!=CHR('g','r','e','k') &&
			script!=CHR('c','y','r','l'))
    continue;
		if ( islower(sc->unicodeenc)) {
		    sc = SFGetChar(sf,toupper(sc->unicodeenc),NULL);
		    if ( sc==NULL )
      goto end_loop2;
		}
		if ( sc->ticked )
    continue;
		sc->ticked = true;
		sc_sc = MakeSmallCapGlyphSlot(sf,sc,script,c2sc,smcp,fv);
		if ( sc_sc==NULL )
      goto end_loop2;
		if ( SFGetAlternate(sf,sc->unicodeenc,sc,false)!=NULL ) 
		    SCBuildComposit(sf,sc_sc,fv->active_layer,true);
		else if ( sc->layers[fv->active_layer].splines==NULL ) {
		    RefChar *rlast = NULL;
		    for ( ref=sc->layers[fv->active_layer].refs; ref!=NULL; ref=ref->next ) {
			snprintf(buffer,sizeof(buffer),"%s.sc", ref->sc->name );
			rsc = SFGetChar(sf,-1,buffer);
			if ( rsc==NULL && (ref->sc->unicodeenc<0x10000 && iscombining(ref->sc->unicodeenc)) )
			    rsc = ref->sc;
			if ( rsc!=NULL ) {
			    r = RefCharCreate();
			    *r = *ref;
#ifdef FONTFORGE_CONFIG_TYPE3
			    r->layers = NULL;
			    r->layer_cnt = 0;
#else
			    r->layers[0].splines = NULL;
#endif
			    r->next = NULL;
			    r->sc = rsc;
			    SCReinstanciateRefChar(sc_sc,r,fv->active_layer);
			    if ( rlast==NULL )
				sc_sc->layers[fv->active_layer].refs=r;
			    else
				rlast->next = r;
			    rlast = r;
			}
		    }
		}
      end_loop2:
		if ( !ff_progress_next())
    break;
	    }
	}
    }
    ff_progress_end_indicator();
}
