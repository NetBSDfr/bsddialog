/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Alfonso Sabato Siciliano
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
 */

#ifndef _LIBBSDDIALOG_H_
#define _LIBBSDDIALOG_H_

extern int bsddialog_interrupt; /*	Set to TRUE in interrupt handler */
extern int bsddialog_abort;     /*	Set to true in callback	to abort */
extern int dpv_overall_read;

struct dpv_config {
	/*uint8_t           keep_tite;     use conf.clear */ 
	/*enum dpv_display  display_type;  only ncurses   */
	/*enum dpv_output   output_type;   unimplemetd    */
	/*int               debug;         unimnpleted    */
	int		display_limit; /* Files/page. Default -1 */
	int		label_size;    /* Label	size. Default 28 */
	int		pbar_size;     /* Mini-progress	size */
	int		dialog_updates_per_second; /* Default 16 */
	int		status_updates_per_second; /* Default 2	*/
	uint16_t		options;       /* Default 0 (none) */
	/*char          *title;            use conf.title */
	/*char		*backtitle;        use bsddialog_backtitle() */
	/*char		*aprompt;      /* Overall Progress: is sufficient */
	/*char		*pprompt;           use text */
	char		*msg_done;     /* Default `Done' */
	char		*msg_fail;     /* Default `Fail' */
	char		*msg_pending;  /* Default `Pending' */
	/*char		*output;           unimplemed */
	/*const char	*status_solo;      unimplemetd */
	/*const char	*status_many;      unimplented */
       /* Function pointer; action to perform data transfer */
	int (*action)(struct dpv_file_node *file, int out);
};

/*enum dpv_status { 
	DPV_STATUS_RUNNING = 0, 7 In progress
	DPV_STATUS_DONE,        5 Done
	DPV_STATUS_FAILED,      2 Failed
};*/

struct bsddialog_file_minibar {
	/*enum dpv_status	  status; use mixedgaug 0-7 */
	int     perc; /*replace dpv_status */
	char		  *msg;	  /* display instead of	"Done/Fail" */
	char		  *name;  /* name of file to read */
	char		  *path;  /* path to file */
	long long	  length; /* expected size */
	long long	  read;	  /* number units read (e.g., bytes) */
	/*struct dpv_file_node *next; use nminibar for helping autosize */
};

int
bsddialog_progressview(struct bsddialog_conf *conf, char* text, int rows,
    int cols, unsigned int mainperc, unsigned int nminibars, char **minilabels,
    int *minipercs);


#endif
