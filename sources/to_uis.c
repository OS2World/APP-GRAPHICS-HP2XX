/*
   Copyright (c) 1992  Heinz W. Werntges.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of HP2xx.

HP2xx is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the GNU General Public License, Version 2 or later, for full details.

Everyone is granted permission to copy, modify and redistribute
HP2xx, but only under the conditions described in the GNU General Public
License.  A copy of this license is supposed to have been
given to you along with HP2xx so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/

/** to_uis.c: VAX/VMS UIS previewer of project "hp2xx"
 **
 ** 92/04/15  V 1.00  HWW  Originating, based on mandel.c (V 1.02)
 **                        Raw coding, not tested yet!
 ** 92/04/24  V 1.01  HWW  Tested and accelerated on uVAX, VMS 4.7
 ** 92/04/27  V 1.02  HWW  Cleaned up
 ** 92/05/25  V 1.02b HWW  Abort if color mode (due to lack of
 **			   test facilities) -- Color support desired!
 **
 ** NOTE: Due to lack of testing facilities, I will not be able to maintain
 **       this file any longer. Volunteers are welcome!
 **       If none will be found, I'll move this file to ../extras soon.
 **/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <descrip.h>
#include <uisentry.h>
#include <uisusrdef.h>
#include "bresnham.h"
#include "hp2xx.h"



void	PicBuf_to_UIS (PicBuf *picbuf, PAR *p)
{
int		byte_c, xoff, yoff;
unsigned long	row_c, x1, x2, rw, rh, bpp, zero=0, two=2;
RowBuf		*row;

float		x0f, y0f, x1f, y1f, w, h;
int		c_old, c_new, i;
unsigned	vd_id, wd_id;
char		*target = "sys$workstation";
static float	intens[2] = {0.0, 1.0};
static unsigned	atb	= 1;

struct dsc$descriptor_s	s_desc;


  if (picbuf->depth > 1)
  {
	fprintf(stderr, "\nUIS preview does not support colors yet -- sorry\n");
	free_PicBuf (picbuf, p->swapfile);
	exit (ERROR);
  }

  if (!p->quiet)
  {
	fprintf(stderr, "\nUIS preview follows\n");
	fprintf(stderr, "Press <return> to end\n");
  }

  xoff = p->xoff * p->dpi_x / 25.4;
  yoff = p->yoff * p->dpi_y / 25.4;

  if ((!p->quiet) &&
      (((picbuf->nb << 3) + xoff > 1024) || (picbuf->nr + yoff > 1024)) )
  {
	fprintf(stderr, "\n\007WARNING: Picture won't fit!\n");
	fprintf(stderr, "Current range: (%d..%d) x (%d..%d) pels\n",
		xoff, (picbuf->nb << 3) + xoff, yoff, picbuf->nr + yoff);
	fprintf(stderr, "Continue anyway (y/n)?: ");
	if (toupper(getchar()) != 'Y')
		return;
  }

  x0f = y0f = 0.0;			/* No offsets yet	*/
  x1f = (float) (picbuf->nb << 3);
  y1f = (float) picbuf->nr;
  w   = (float) p->width / 10.0;	/* VAX needs cm, not mm */
  h   = (float) p->height/ 10.0;

  vd_id = uis$create_display (&x0f, &y0f, &x1f, &y1f,&w, &h);
  uis$disable_display_list (&vd_id);
  uis$set_intensities (&vd_id, &zero, &two, intens);

  s_desc.dsc$w_length = strlen (target);
  s_desc.dsc$a_pointer= target;
  s_desc.dsc$b_class  = DSC$K_CLASS_S;
  s_desc.dsc$b_dtype  = DSC$K_DTYPE_T;
  wd_id = uis$create_window (&vd_id,&s_desc);

  x1 = 0;
  x2 = picbuf->nc;
  rw = picbuf->nc;
  rh = 1;
  bpp = 1;

  for (row_c = 0; row_c < picbuf->nr; row_c++)	/* for all pixel rows */
  {
/**
 ** Unfortunately, we need a bit reversal in each byte here:
 **/
	row = get_RowBuf (picbuf, row_c);
	for (byte_c=0; byte_c < picbuf->nb; byte_c++)
	{
		c_old = row->buf[byte_c];

		if (c_old == 0)		/* all white	*/
		    continue;
		if (c_old == 0xff)	/* all black	*/
		    continue;

		for (i=0, c_new=0; ; )
		{
		    if (c_old & 1)
			c_new |= 1;
		    if (++i == 8)	/* 8 bits, 7 shifts	*/
			break;
		    c_new <<= 1;
		    c_old >>= 1;
		}
		row->buf[byte_c] = c_new;
	}

	uisdc$image(&wd_id, &atb, &x1, &row_c, &x2, &row_c,
			&rw, &rh, &bpp, row->buf);
  }
  getchar();
  uis$delete_display (&vd_id);
}

