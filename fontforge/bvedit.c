/* Copyright (C) 2000-2004 by George Williams */
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
#include "pfaeditui.h"
#include <math.h>
#include "ustring.h"

static int askfraction(int *xoff, int *yoff) {
    static int lastx=1, lasty = 3;
    char buffer[30];
    unichar_t ubuffer[30];
    unichar_t *ret, *end, *end2;
    int xv, yv;

    sprintf( buffer, "%d:%d", lastx, lasty );
    uc_strcpy(ubuffer,buffer);
    ret = GWidgetAskStringR(_STR_Skew,ubuffer,_STR_SkewRatio);
    if ( ret==NULL )
return( 0 );
    xv = u_strtol(ret,&end,10);
    yv = u_strtol(end+1,&end2,10);
    if ( xv==0 || xv>10 || xv<-10 || yv<=0 || yv>10 || *end!=':' || *end2!='\0' ) {
	GWidgetErrorR( _STR_BadNumber,_STR_BadNumber );
	free(ret);
return( 0 );
    }
    *xoff = lastx = xv; *yoff = lasty = yv;
return( 1 );
}

static void BCTransFunc(BDFChar *bc,enum bvtools type,int xoff,int yoff) {
    int i, j;
    uint8 *pt, *end, *pt2, *bitmap;
    int bpl, temp;
    int xmin, xmax;

    BCFlattenFloat(bc);
    if ( type==bvt_transmove ) {
	bc->xmin += xoff; bc->xmax += xoff;
	bc->ymin += yoff; bc->ymax += yoff;
	bitmap = NULL;
    } else if ( type==bvt_flipv ) {
	for ( i=0, j=bc->ymax-bc->ymin; i<j; ++i, --j ) {
	    pt = bc->bitmap + i*bc->bytes_per_line;
	    pt2 = bc->bitmap + j*bc->bytes_per_line;
	    end = pt+bc->bytes_per_line;
	    while ( pt<end ) {
		*pt ^= *pt2;
		*pt2 ^= *pt;
		*pt++ ^= *pt2++;
	    }
	}
	bitmap = NULL;
    } else if ( !bc->byte_data ) {
	if ( type==bvt_fliph ) {
	    bitmap = gcalloc((bc->ymax-bc->ymin+1)*bc->bytes_per_line,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		pt2 =    bitmap + i*bc->bytes_per_line;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    if ( pt[j>>3]&(1<<(7-(j&7))) ) {
			int nj = bc->xmax-bc->xmin-j;
			pt2[nj>>3] |= (1<<(7-(nj&7)));
		    }
		}
	    }
	} else if ( type==bvt_rotate180 ) {
	    bitmap = gcalloc((bc->ymax-bc->ymin+1)*bc->bytes_per_line,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		pt2 = bitmap + (bc->ymax-bc->ymin-i)*bc->bytes_per_line;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    if ( pt[j>>3]&(1<<(7-(j&7))) ) {
			int nj = bc->xmax-bc->xmin-j;
			pt2[nj>>3] |= (1<<(7-(nj&7)));
		    }
		}
	    }
	} else if ( type==bvt_rotate90cw ) {
	    bpl = ((bc->ymax-bc->ymin)>>3) + 1;
	    bitmap = gcalloc((bc->xmax-bc->xmin+1)*bpl,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    if ( pt[j>>3]&(1<<(7-(j&7))) ) {
			int nx = bc->ymax-bc->ymin-i;
			bitmap[j*bpl+(nx>>3)] |= (1<<(7-(nx&7)));
		    }
		}
	    }
	    bc->bytes_per_line = bpl;
	    temp = bc->xmax; bc->xmax = bc->ymax; bc->ymax = temp;
	    temp = bc->xmin; bc->xmin = bc->ymin; bc->ymin = temp;
	} else if ( type==bvt_rotate90ccw ) {
	    bpl = ((bc->ymax-bc->ymin)>>3) + 1;
	    bitmap = gcalloc((bc->xmax-bc->xmin+1)*bpl,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    if ( pt[j>>3]&(1<<(7-(j&7))) ) {
			int ny = bc->xmax-bc->xmin-j;
			bitmap[ny*bpl+(i>>3)] |= (1<<(7-(i&7)));
		    }
		}
	    }
	    bc->bytes_per_line = bpl;
	    temp = bc->xmax; bc->xmax = bc->ymax; bc->ymax = temp;
	    temp = bc->xmin; bc->xmin = bc->ymin; bc->ymin = temp;
	} else /* if ( type==bvt_skew ) */ {
	    if ( xoff>0 ) {
		xmin = bc->xmin+(xoff*bc->ymin)/yoff;
		xmax = bc->xmax+(xoff*bc->ymax)/yoff;
	    } else {
		xmin = bc->xmin+(xoff*bc->ymax)/yoff;
		xmax = bc->xmax+(xoff*bc->ymin)/yoff;
	    }
	    bpl = ((xmax-xmin)>>3) + 1;
	    bitmap = gcalloc((bc->ymax-bc->ymin+1)*bpl,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		pt2 = bitmap + i*bpl;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    if ( pt[j>>3]&(1<<(7-(j&7))) ) {
			int nj = j+bc->xmin-xmin + (xoff*(bc->ymax-i))/yoff;
			pt2[nj>>3] |= (1<<(7-(nj&7)));
		    }
		}
	    }
	    bc->xmax = xmax; bc->xmin = xmin; bc->bytes_per_line = bpl;
	}
    } else {		/* Byte data */
	if ( type==bvt_fliph ) {
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		for ( j=0; j<=(bc->xmax-bc->xmin)/2; ++j ) {
		    int nj = bc->xmax-bc->xmin-j;
		    int temp = pt[nj];
		    pt[nj] = pt[j];
		    pt[j] = temp;
		}
	    }
	    bitmap = NULL;
	} else if ( type==bvt_rotate180 ) {
	    bitmap = gcalloc((bc->ymax-bc->ymin+1)*bc->bytes_per_line,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		pt2 = bitmap + (bc->ymax-bc->ymin-i)*bc->bytes_per_line;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    int nj = bc->xmax-bc->xmin-j;
		    pt2[nj] = pt[j];
		}
	    }
	} else if ( type==bvt_rotate90cw ) {
	    bpl = bc->ymax-bc->ymin + 1;
	    bitmap = gcalloc((bc->xmax-bc->xmin+1)*bpl,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    int nx = bc->ymax-bc->ymin-i;
		    bitmap[j*bpl+nx] = pt[j];
		}
	    }
	    bc->bytes_per_line = bpl;
	    temp = bc->xmax; bc->xmax = bc->ymax; bc->ymax = temp;
	    temp = bc->xmin; bc->xmin = bc->ymin; bc->ymin = temp;
	} else if ( type==bvt_rotate90ccw ) {
	    bpl = bc->ymax-bc->ymin + 1;
	    bitmap = gcalloc((bc->xmax-bc->xmin+1)*bpl,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    int ny = bc->xmax-bc->xmin-j;
		    bitmap[ny*bpl+i] = pt[j];
		}
	    }
	    bc->bytes_per_line = bpl;
	    temp = bc->xmax; bc->xmax = bc->ymax; bc->ymax = temp;
	    temp = bc->xmin; bc->xmin = bc->ymin; bc->ymin = temp;
	} else /* if ( type==bvt_skew ) */ {
	    if ( xoff>0 ) {
		xmin = bc->xmin+(xoff*bc->ymin)/yoff;
		xmax = bc->xmax+(xoff*bc->ymax)/yoff;
	    } else {
		xmin = bc->xmin+(xoff*bc->ymax)/yoff;
		xmax = bc->xmax+(xoff*bc->ymin)/yoff;
	    }
	    bpl = xmax-xmin + 1;
	    bitmap = gcalloc((bc->ymax-bc->ymin+1)*bpl,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		pt2 = bitmap + i*bpl;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    int nj = j+bc->xmin-xmin + (xoff*(bc->ymax-i))/yoff;
		    pt2[nj] = pt[j];
		}
	    }
	    bc->xmax = xmax; bc->xmin = xmin; bc->bytes_per_line = bpl;
	}
    }
    if ( bitmap!=NULL ) {
	free(bc->bitmap);
	bc->bitmap = bitmap;
    }
    BCCompressBitmap(bc);
}

void BCTrans(BDFFont *bdf,BDFChar *bc,BVTFunc *bvts,FontView *fv ) {
    int xoff=0, yoff=0, i;

    if ( bvts[0].func==bvt_none )
return;
    BCPreserveState(bc);
    for ( i=0; bvts[i].func!=bvt_none; ++i ) {
	if ( bvts[i].func==bvt_transmove ) {
	    xoff = rint(bvts[i].x*bdf->pixelsize/(fv->sf->ascent+fv->sf->descent));
	    yoff = rint(bvts[i].y*bdf->pixelsize/(fv->sf->ascent+fv->sf->descent));
	} else if ( bvts[i].func==bvt_skew ) {
	    xoff = bvts[i].x;
	    yoff = bvts[i].y;
	}
	BCTransFunc(bc,bvts[i].func,xoff,yoff);
    }
    BCCharChangedUpdate(bc);
}

void BCRotateCharForVert(BDFChar *bc,BDFChar *from, BDFFont *frombdf) {
    /* Take the image in from, make a copy of it, put it in bc, rotate it */
    /*  shift it around slightly so it is aligned properly to work as a CJK */
    /*  vertically displayed latin letter */
    int xmin, ymax;

    BCPreserveState(bc);
    BCFlattenFloat(from);
    free(bc->bitmap);
    bc->xmin = from->xmin; bc->xmax = from->xmax; bc->ymin = from->ymin; bc->ymax = from->ymax;
    bc->width = from->width; bc->bytes_per_line = from->bytes_per_line;
    bc->bitmap = galloc(bc->bytes_per_line*(bc->ymax-bc->ymin+1));
    memcpy(bc->bitmap,from->bitmap,bc->bytes_per_line*(bc->ymax-bc->ymin+1));
    BCTransFunc(bc,bvt_rotate90cw,0,0);
    xmin = frombdf->descent + from->ymin;
    ymax = frombdf->ascent - from->xmin;
    bc->xmax += xmin-bc->xmin; bc->xmin = xmin;
    bc->ymin += ymax-bc->ymax-1; bc->ymax = ymax-1;
    bc->width = frombdf->pixelsize;
}

void BVRotateBitmap(BitmapView *bv,enum bvtools type ) {
    int xoff=0, yoff=0;

    if ( type==bvt_skew )
	if ( !askfraction(&xoff,&yoff))
return;
    BCPreserveState(bv->bc);
    BCTransFunc(bv->bc,type,xoff,yoff);
    BCCharChangedUpdate(bv->bc);
}

static void BCExpandBitmap(BDFChar *bc, int x, int y) {
    int xmin, xmax, bpl, ymin, ymax;
    uint8 *bitmap;
    int i,j,nj;
    uint8 *pt, *npt;

    if ( x<bc->xmin || x>bc->xmax || y<bc->ymin || y>bc->ymax ) {
	xmin = x<bc->xmin?x:bc->xmin;
	xmax = x>bc->xmax?x:bc->xmax;
	ymin = y<bc->ymin?y:bc->ymin;
	ymax = y>bc->ymax?y:bc->ymax;
	if ( !bc->byte_data ) {
	    bpl = ((xmax-xmin)>>3) + 1;
	    bitmap = gcalloc((ymax-ymin+1)*bpl,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		npt = bitmap + (i+ymax-bc->ymax)*bpl;
		for ( j=0; j<=bc->xmax-bc->xmin; ++j ) {
		    if ( pt[j>>3] & (1<<(7-(j&7))) ) {
			nj = j+bc->xmin-xmin;
			npt[nj>>3] |= (1<<(7-(nj&7)));
		    }
		}
	    }
	} else {
	    bpl = xmax-xmin + 1;
	    bitmap = gcalloc((ymax-ymin+1)*bpl,sizeof(uint8));
	    for ( i=0; i<=bc->ymax-bc->ymin; ++i ) {
		pt = bc->bitmap + i*bc->bytes_per_line;
		npt = bitmap + (i+ymax-bc->ymax)*bpl;
		memcpy(npt+bc->xmin-xmin,pt,bc->bytes_per_line);
	    }
	}
	free(bc->bitmap);
	bc->bitmap = bitmap;
	bc->xmin = xmin; bc->xmax = xmax; bc->bytes_per_line = bpl;
	bc->ymin = ymin; bc->ymax = ymax;
    }
}

void BCSetPoint(BDFChar *bc, int x, int y, int color ) {

    if ( x<bc->xmin || x>bc->xmax || y<bc->ymin || y>bc->ymax ) {
	if ( color==0 )
return;		/* Already clear */
	BCExpandBitmap(bc,x,y);
    }
    y = bc->ymax-y;
    x -= bc->xmin;
    if ( bc->byte_data )
	bc->bitmap[y*bc->bytes_per_line+x] = color;
    else if ( color==0 )
	bc->bitmap[y*bc->bytes_per_line+(x>>3)] &= ~(1<<(7-(x&7)));
    else
	bc->bitmap[y*bc->bytes_per_line+(x>>3)] |= (1<<(7-(x&7)));
}

static void BCBresenhamLine(BitmapView *bv,
	void (*SetPoint)(BitmapView *,int x, int y, void *data),void *data) {
    /* Draw a line from (pressed_x,pressed_y) to (info_x,info_y) */
    /*  and call SetPoint for each point */
    int dx,dy,incr1,incr2,d,x,y,xend;
    int x1 = bv->pressed_x, y1 = bv->pressed_y;
    int x2 = bv->info_x, y2 = bv->info_y;
    int up;

    if ( y2<y1 ) {
	y2 ^= y1; y1 ^= y2; y2 ^= y1;
	x2 ^= x1; x1 ^= x2; x2 ^= x1;
    }
    dy = y2-y1;
    if (( dx = x2-x1)<0 ) dx=-dx;

    if ( dy<=dx ) {
	d = 2*dy-dx;
	incr1 = 2*dy;
	incr2 = 2*(dy-dx);
	if ( x1>x2 ) {
	    x = x2; y = y2;
	    xend = x1;
	    up = -1;
	} else {
	    x = x1; y = y1;
	    xend = x2;
	    up = 1;
	}
	(SetPoint)(bv,x,y,data);
	while ( x<xend ) {
	    ++x;
	    if ( d<0 ) d+=incr1;
	    else {
		y += up;
		d += incr2;
	    }
	    (SetPoint)(bv,x,y,data);
	}
    } else {
	d = 2*dx-dy;
	incr1 = 2*dx;
	incr2 = 2*(dx-dy);
	x = x1; y = y1;
	if ( x1>x2 ) up = -1; else up = 1;
	(SetPoint)(bv,x,y,data);
	while ( y<y2 ) {
	    ++y;
	    if ( d<0 ) d+=incr1;
	    else {
		x += up;
		d += incr2;
	    }
	    (SetPoint)(bv,x,y,data);
	}
    }
}

static void CirclePoints(BitmapView *bv,int x, int y, int ox, int oy, int xmod, int ymod,
	void (*SetPoint)(BitmapView *,int x, int y, void *data),void *data) {
    /* we draw the quadrant between Pi/2 and 0 */
    if ( bv->active_tool == bvt_filledelipse ) {
	int j;
	for ( j=2*oy+ymod-y; j<=y; ++j ) {
	    SetPoint(bv,x,j,data);
	    SetPoint(bv,2*ox+xmod-x,j,data);
	}
    } else {
	SetPoint(bv,x,y,data);
	SetPoint(bv,x,2*oy+ymod-y,data);
	SetPoint(bv,2*ox+xmod-x,y,data);
	SetPoint(bv,2*ox+xmod-x,2*oy+ymod-y,data);
    }
}

void BCGeneralFunction(BitmapView *bv,
	void (*SetPoint)(BitmapView *,int x, int y, void *data),void *data) {
    int i, j;
    int xmin, xmax, ymin, ymax;
    int ox, oy, modx, mody;
    int dx, dy, c,d,dx2,dy2,xp,yp;
    int x,y;

    if ( bv->pressed_x<bv->info_x ) {
	xmin = bv->pressed_x; xmax = bv->info_x;
    } else {
	xmin = bv->info_x; xmax = bv->pressed_x;
    }
    if ( bv->pressed_y<bv->info_y ) {
	ymin = bv->pressed_y; ymax = bv->info_y;
    } else {
	ymin = bv->info_y; ymax = bv->pressed_y;
    }

    switch ( bv->active_tool ) {
      case bvt_line:
	BCBresenhamLine(bv,SetPoint,data);
      break;
      case bvt_rect:
	for ( i=xmin; i<=xmax; ++i ) {
	    SetPoint(bv,i,bv->pressed_y,data);
	    SetPoint(bv,i,bv->info_y,data);
	}
	for ( i=ymin; i<=ymax; ++i ) {
	    SetPoint(bv,bv->pressed_x,i,data);
	    SetPoint(bv,bv->info_x,i,data);
	}
      break;
      case bvt_filledrect:
	for ( i=xmin; i<=xmax; ++i ) {
	    for ( j=ymin; j<=ymax; ++j )
		SetPoint(bv,i,j,data);
	}
      break;
      case bvt_elipse: case bvt_filledelipse:
	if ( xmax==xmin || ymax==ymin )		/* degenerate case */
	    BCBresenhamLine(bv,SetPoint,data);
	else {
	    ox = floor( (xmin+xmax)/2.0 );
	    oy = floor( (ymin+ymax)/2.0 );
	    modx = (xmax+xmin)&1; mody = (ymax+ymin)&1;
	    dx = ox-xmin;
	    dy = oy-ymin;
	    dx2 = dx*dx; dy2 = dy*dy;
	    xp = 0; yp = 4*dy*dx2;
	    c = dy2+(2-4*dy)*dx2; d = 2*dy2 + (1-2*dy)*dx2;
	    x = ox+modx; y = ymax;
	    CirclePoints(bv,x,y,ox,oy,modx,mody,SetPoint,data);
	    while ( x!=xmax ) {
#define move_right() (c += 4*dy2+xp, d += 6*dy2+xp, ++x, xp += 4*dy2 )
#define move_down() (c += 6*dx2-yp, d += 4*dx2-yp, --y, yp -= 4*dx2 )
		if ( d<0 || y==0 )
		    move_right();
		else if ( c > 0 )
		    move_down();
		else {
		    move_right();
		    move_down();
		}
#undef move_right
#undef move_down
		if ( y<oy )		/* degenerate cases */
	    break;
		CirclePoints(bv,x,y,ox,oy,modx,mody,SetPoint,data);
	    }
	    if ( bv->active_tool==bvt_elipse ) {
		/* there may be quite a gap between the the two semi-circles */
		/*  because the tangent is nearly vertical here. So just fill */
		/*  it in */
		int j;
		for ( j=2*oy+mody-y; j<=y; ++j ) {
		    SetPoint(bv,x,j,data);
		    SetPoint(bv,2*ox+modx-x,j,data);
		}
	    }
	}
      break;
    }
}

void BDFFloatFree(BDFFloat *sel) {
    if ( sel==NULL )
return;
    free(sel->bitmap);
    free(sel);
}

BDFFloat *BDFFloatCopy(BDFFloat *sel) {
    BDFFloat *new;
    if ( sel==NULL )
return(NULL);
    new = galloc(sizeof(BDFFloat));
    *new = *sel;
    new->bitmap = galloc(sel->bytes_per_line*(sel->ymax-sel->ymin+1));
    memcpy(new->bitmap,sel->bitmap,sel->bytes_per_line*(sel->ymax-sel->ymin+1));
return( new );
}

BDFFloat *BDFFloatConvert(BDFFloat *sel,int todepth,int fromdepth) {
    BDFFloat *new;
    int i,j,fdiv,tdiv;
    if ( sel==NULL )
return(NULL);

    if ( todepth==fromdepth )
return( BDFFloatCopy(sel));

    new = galloc(sizeof(BDFFloat));
    *new = *sel;
    new->byte_data = todepth!=1;
    new->depth = todepth;
    new->bytes_per_line = new->byte_data ? new->xmax-new->xmin+1 : ((new->xmax-new->xmin)>>3)+1;
    new->bitmap = gcalloc(new->bytes_per_line*(sel->ymax-sel->ymin+1),1);
    if ( fromdepth==1 ) {
	tdiv = ((1<<todepth)-1);
	for ( i=0; i<=sel->ymax-sel->ymin; ++i ) {
	    for ( j=0; j<=sel->xmax-sel->xmin; ++j ) {
		if ( sel->bitmap[i*sel->bytes_per_line+(j>>3)]&(0x80>>(j&7)) )
		    new->bitmap[i*new->bytes_per_line+j] = tdiv;
	    }
	}
    } else if ( todepth==1 ) {
	fdiv = (1<<fromdepth)/2;
	for ( i=0; i<=sel->ymax-sel->ymin; ++i ) {
	    for ( j=0; j<=sel->xmax-sel->xmin; ++j ) {
		if ( sel->bitmap[i*sel->bytes_per_line+j]>=fdiv )
		    new->bitmap[i*new->bytes_per_line+(j>>3)] |= (0x80>>(j&7));
	    }
	}
    } else {
	fdiv = 255/((1<<fromdepth)-1); tdiv = 255/((1<<todepth)-1);
	memcpy(new->bitmap,sel->bitmap,sel->bytes_per_line*(sel->ymax-sel->ymin+1));
	for ( i=0 ; i<sel->bytes_per_line*(sel->ymax-sel->ymin+1); ++i )
	    new->bitmap[i] = (sel->bitmap[i]*fdiv + tdiv/2)/tdiv;
    }
return( new );
}

/* Creates a floating selection, and clears out the underlying bitmap */
BDFFloat *BDFFloatCreate(BDFChar *bc,int xmin,int xmax,int ymin,int ymax, int clear) {
    BDFFloat *new;
    int x,y;
    uint8 *bpt, *npt;

    if ( bc->selection!=NULL ) {
	BCFlattenFloat(bc);
	bc->selection = NULL;
    }
    BCCompressBitmap(bc);

    if ( xmin>xmax ) {
	xmin ^= xmax; xmax ^= xmin; xmin ^= xmax;
    }
    if ( ymin>ymax ) {
	ymin ^= ymax; ymax ^= ymin; ymin ^= ymax;
    }
    if ( xmin<bc->xmin ) xmin = bc->xmin;
    if ( xmax>bc->xmax ) xmax = bc->xmax;
    if ( ymin<bc->ymin ) ymin = bc->ymin;
    if ( ymax>bc->ymax ) ymax = bc->ymax;
    if ( xmin>xmax || ymin>ymax )
return( NULL );
    new = galloc(sizeof(BDFFloat));
    new->xmin = xmin;
    new->xmax = xmax;
    new->ymin = ymin;
    new->ymax = ymax;
    new->byte_data = bc->byte_data;
    new->depth = bc->depth;
    if ( bc->byte_data ) {
	new->bytes_per_line = xmax-xmin+1;
	new->bitmap = gcalloc(new->bytes_per_line*(ymax-ymin+1),sizeof(uint8));
	for ( y=ymin; y<=ymax; ++y ) {
	    bpt = bc->bitmap + (bc->ymax-y)*bc->bytes_per_line;
	    npt = new->bitmap + (ymax-y)*new->bytes_per_line;
	    memcpy(npt,bpt+xmin-bc->xmin,xmax-xmin+1);
	    if ( clear )
		memset(bpt+xmin-bc->xmin,0,xmax-xmin+1);
	}
    } else {
	new->bytes_per_line = ((xmax-xmin)>>3)+1;
	new->bitmap = gcalloc(new->bytes_per_line*(ymax-ymin+1),sizeof(uint8));
	for ( y=ymin; y<=ymax; ++y ) {
	    bpt = bc->bitmap + (bc->ymax-y)*bc->bytes_per_line;
	    npt = new->bitmap + (ymax-y)*new->bytes_per_line;
	    for ( x=xmin; x<=xmax; ++x ) {
		int bx = x-bc->xmin, nx = x-xmin;
		if ( bpt[bx>>3]&(1<<(7-(bx&7))) ) {
		    npt[nx>>3] |= (1<<(7-(nx&7)));
		    if ( clear )
			bpt[bx>>3] &= ~(1<<(7-(bx&7)));
		}
	    }
	}
    }
    if ( clear )
	bc->selection = new;
return( new );
}

void BCFlattenFloat(BDFChar *bc ) {
    /* flatten any floating selection */
    BDFFloat *sel = bc->selection;
    int x,y;
    uint8 *bpt, *spt;

    if ( sel!=NULL ) {
	BCExpandBitmap(bc,sel->xmin,sel->ymin);
	BCExpandBitmap(bc,sel->xmax,sel->ymax);
	if ( bc->byte_data ) {
	    for ( y=sel->ymin; y<=sel->ymax; ++y ) {
		bpt = bc->bitmap + (bc->ymax-y)*bc->bytes_per_line;
		spt = sel->bitmap + (sel->ymax-y)*sel->bytes_per_line;
		memcpy(bpt+sel->xmin-bc->xmin,spt,sel->xmax-sel->xmin+1);
	    }
	} else {
	    for ( y=sel->ymin; y<=sel->ymax; ++y ) {
		bpt = bc->bitmap + (bc->ymax-y)*bc->bytes_per_line;
		spt = sel->bitmap + (sel->ymax-y)*sel->bytes_per_line;
		for ( x=sel->xmin; x<=sel->xmax; ++x ) {
		    int bx = x-bc->xmin, sx = x-sel->xmin;
		    if ( spt[sx>>3]&(1<<(7-(sx&7))) )
			bpt[bx>>3] |= (1<<(7-(bx&7)));
		    else
			bpt[bx>>3] &= ~(1<<(7-(bx&7)));
		}
	    }
	}
	BDFFloatFree(sel);
	bc->selection = NULL;
    }
}

void BCPasteInto(BDFChar *bc,BDFChar *rbc,int ixoff,int iyoff, int invert, int cleartoo) {
    int x,y;
    uint8 *bpt, *rpt;

    x = 0;
    BCExpandBitmap(bc,rbc->xmin+ixoff,rbc->ymin+iyoff);
    BCExpandBitmap(bc,rbc->xmax+ixoff,rbc->ymax+iyoff);
    for ( y=rbc->ymin; y<=rbc->ymax; ++y ) {
	bpt = bc->bitmap + (bc->ymax-(y+iyoff))*bc->bytes_per_line;
	if ( invert )
	    rpt = rbc->bitmap + y*rbc->bytes_per_line;
	else
	    rpt = rbc->bitmap + (rbc->ymax-y)*rbc->bytes_per_line;
	if ( bc->byte_data )
	    memcpy(bpt+x+ixoff-bc->xmin,rpt,rbc->xmax-rbc->xmin+1);
	else {
	    for ( x=rbc->xmin; x<=rbc->xmax; ++x ) {
		int bx = x+ixoff-bc->xmin, rx = x-rbc->xmin;
		if ( rpt[rx>>3]&(1<<(7-(rx&7))) )
		    bpt[bx>>3] |= (1<<(7-(bx&7)));
		else if ( cleartoo )
		    bpt[bx>>3] &= ~(1<<(7-(bx&7)));
	    }
	}
    }
    BCCompressBitmap(bc);
}

static BDFChar *BCScale(BDFChar *old,int from, int to) {
    BDFChar *new;
    int x,y, ox,oy, oxs,oys, oxend, oyend;
    real tot, scale;
    real yscale, xscale;
    real dto = to;

    if ( old==NULL || old->byte_data )
return( NULL );
    new = chunkalloc(sizeof(BDFChar));
    new->sc = old->sc;
    new->xmin = rint(old->xmin*dto/from);
    new->ymin = rint(old->ymin*dto/from);
    new->xmax = new->xmin + rint((old->xmax-old->xmin+1)*dto/from-1);
    new->ymax = new->ymin + rint((old->ymax-old->ymin+1)*dto/from-1);
    if ( new->sc!=NULL && new->sc->width != new->sc->parent->ascent+new->sc->parent->descent )
	new->width = rint(new->sc->width*dto/(new->sc->parent->ascent+new->sc->parent->descent)+.5);
    else
	new->width = rint(old->width*dto/from+.5);
    new->bytes_per_line = (new->xmax-new->xmin)/8+1;
    new->bitmap = gcalloc((new->ymax-new->ymin+1)*new->bytes_per_line,sizeof(char));
    new->enc = old->enc;

    scale = dto/from;
    scale *= scale;
    scale /= 2;

    for ( y=0; y<=new->ymax-new->ymin; ++y ) for ( x=0; x<=new->xmax-new->xmin; ++x ) {
	tot = 0;
	oys = floor(y*from/dto); oyend = ceil((y+1)*from/dto);
	oxs = floor(x*from/dto); oxend = ceil((x+1)*from/dto);
	for ( oy = oys; oy<oyend && oy<=old->ymax-old->ymin; ++oy ) {
	    yscale = 1;
	    if ( oy==oys && oy==oyend-1 )
		yscale = 1-(oyend - (y+1)*dto/from) - (y*dto/from - oys);
	    else if ( oy==oys )
		yscale = 1-(y*dto/from-oys);
	    else if ( oy==oyend-1 )
		yscale = 1-(oyend - (y+1)*dto/from);
	    for ( ox = oxs; ox<oxend && ox<=old->xmax-old->xmin; ++ox ) {
		if ( old->bitmap[oy*old->bytes_per_line + (ox>>3)] & (1<<(7-(ox&7))) ) {
		    xscale = 1;
		    if ( ox==oxs && ox==oxend-1 )
			xscale = 1-(oxend - (x+1)*dto/from) - (x*dto/from - oxs);
		    else if ( ox==oxs )
			xscale = 1-(x*dto/from-oxs);
		    else if ( ox==oxend-1 )
			xscale = 1-(oxend - (x+1)*dto/from);
		    tot += yscale*xscale;
		}
	    }
	}
	if ( tot>=scale )
	    new->bitmap[y*new->bytes_per_line + (x>>3)] |= (1<<(7-(x&7)));
    }

return( new );
}

BDFFont *BitmapFontScaleTo(BDFFont *old, int to) {
    BDFFont *new = gcalloc(1,sizeof(BDFFont));
    int i;

    if ( old->clut!=NULL ) {
	fprintf( stderr, "Attempt to scale a greymap font, not supported\n" );
return( NULL );
    }

    new->sf = old->sf;
    new->charcnt = old->charcnt;
    new->chars = galloc(new->charcnt*sizeof(BDFChar *));
    new->pixelsize = to;
    new->ascent = (old->ascent*to+.5)/old->pixelsize;
    new->descent = to-new->ascent;
    new->encoding_name = old->encoding_name;
    new->foundry = copy(old->foundry);
    for ( i=0; i<old->charcnt; ++i )
	new->chars[i] = BCScale(old->chars[i],old->pixelsize,to);
return( new );
}
