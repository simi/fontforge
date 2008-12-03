/* Copyright (C) 2000-2008 by George Williams */
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
#ifndef _GRESEDIT_H
#define _GRESEDIT_H

#include "basics.h"
#include "gresource.h"
#include "ggadget.h"

enum res_type2 { rt_image = rt_string+1, rt_font };

typedef struct gresinfo {
    struct gresinfo *next;
    struct gresinfo *inherits_from;
    struct gresinfo *seealso1, *seealso2;
    GBox *boxdata;
    GFont **font;
    GGadgetCreateData *examples;
    struct resed { char *name, *resname; enum res_type type; void *val; char *popup; union { int ival; double dval; char *sval; GFont *fontval; } orig; int cid; } *extras;
    char *name;
    char *initialcomment;
    char *resname;
    char *progname;
    uint8 is_button;		/* Activate default button border flag */
    GBox orig_state;
} GResInfo;

extern void GResEdit(GResInfo *additional,const char *def_res_file,void (*change_res_filename)(const char *));
#endif /* _GRESEDIT_H */
