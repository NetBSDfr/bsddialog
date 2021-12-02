/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2011 Nathan Whitehorn
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <stdio.h>
#include <unistd.h>
#include <libutil.h>
#include <bsddialog.h>
#include <stdlib.h>
#include <string.h>

#include "diskeditor.h"

int
diskeditor_show(const char *title, const char *cprompt,
    struct partedit_item *items, int nitems, int *selected, int *nscroll)
{
	int i, output;
	struct bsddialog_conf conf;
	struct bsddialog_menuitem *bsditems;
	char desc[1024];

	bsddialog_initconf(&conf);
	conf.title = title;
	char *text = cprompt;

	bsditems = malloc(nitems * sizeof(struct bsddialog_menuitem));
	for (i=0; i<nitems; i++) {
		bsditems[i].prefix = "";
		bsditems[i].depth = items[i].indentation;
		bsditems[i].name = items[i].name;
		snprintf(desc, 1024, "%jd %s %s", items[i].size, items[i].type, items[i].mountpoint);
		bsditems[i].desc = strdup(desc);
		bsditems[i].bottomdesc = "";
		bsditems[i].on = false;
	}

	conf.menu.align_left = true;

	conf.button.ok_label = "Create";
	
	conf.button.extra_button = true;
	conf.button.extra_label = "Delete";
	
	conf.button.no_label = "Cancel";
	
	conf.button.help_button = true;
	conf.button.help_label = "Revert";
	
	conf.button.generic1_label = "Auto";
	conf.button.generic2_label = "Finish";
	
	conf.button.default_label= "Finish";
	
	if (bsddialog_init() < 0)
		return -1;

	output = bsddialog_menu(conf, text, 20, 0, 10, 5, bsditems, NULL);

	for (i=0; i<nitems; i++){
		if (bsditems[i].on == true) {
			*selected = i;
			break;
		}
	}

	free(bsditems);
	
	return output;
}

