/*
   Copyright (c) 1991 - 1993 Heinz W. Werntges.  All rights reserved.
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

/** to_pcl.c: PCL converter part of project "hp2xx"
 **
 ** 91/01/19  V 1.00  HWW  Reorganized
 ** 91/01/29  V 1.01  HWW  Tested on SUN
 ** 91/02/01  V 1.02  HWW  Deskjet specials acknowledged
 ** 91/02/15  V 1.03  HWW  VAX_C support added
 ** 91/02/20  V 1.04b HWW  x & y positioning: Now absolute!
 **			  Some VAX_C changes
 ** 91/06/09  V 1.05  HWW  New options added
 ** 91/10/15  V 1.06  HWW  ANSI_C
 ** 91/10/25  V 1.07  HWW  VAX: fopen() augmentations used, open() removed
 ** 92/05/17  V 1.07b HWW  Output to stdout if outfile == '-'
 ** 92/05/19  V 1.07c HWW  Abort if color mode
 ** 92/12/23  V 1.08a HWW  Color for Deskjet (beginning)
 ** 93/04/02  V 1.08b HWW  DotBlock --> Byte
 ** 93/04/13  V 1.09a HWW  CMYK supported
 **/

#include <stdio.h>
#include <stdlib.h>
#include "bresnham.h"
#include "hp2xx.h"


#define	PCL_FIRST 	1			/* Bit mask! 	*/
#define	PCL_LAST 	2			/* Bit mask!	*/



static	Byte	*p_K, *p_C, *p_M, *p_Y;		/* Buffer ptrs (CMYK bits) */



void	Buf_to_PCL (Byte *buf, int nb, int mode, FILE *fd)
/**
 ** Output the raw bit stream
 **   (This should be an ideal place for data compression)
 **/
{
  if (mode & PCL_FIRST)
	fprintf(fd,"\033*b");

  if (mode & PCL_LAST)
	fprintf(fd,"%dW", nb);
  else
	fprintf(fd,"%1dv", nb);

/*  Following change keeps the VAX people happy:	*/
/*fwrite (buf, 1, nb, fd);		*/
  fwrite (buf, nb, 1, fd);
}




void	KCMY_Buf_to_PCL (int nb, int is_KCMY, FILE *fd)
{
  if (is_KCMY)
  {
	Buf_to_PCL (p_K, nb, PCL_FIRST, fd);
	Buf_to_PCL (p_C, nb, 0,         fd);
  }
  else			/* is only CMY:	*/
	Buf_to_PCL (p_C, nb, PCL_FIRST, fd);

  Buf_to_PCL (p_M, nb, 0,        fd);
  Buf_to_PCL (p_Y, nb, PCL_LAST, fd);
}




void	KCMY_to_K (int nb)
/**
 ** Color -> B/W conversion:
 ** Any set bit will show up black
 **/
{
int	i;
Byte	*pK = p_K, *pC = p_C, *pM = p_M, *pY = p_Y;

  for (i=0; i < nb; i++)
	*pK++ |= ((*pC++ | *pM++) | *pY++);
}




void	K_to_CMY (int nb)
/**
 ** CMYK-to-CMY conversion:
 ** Any set bit in the "black" layer sets all C,M,Y bits to emulate "black"
 **/
{
int	i;
Byte	*pK = p_K, *pC = p_C, *pM = p_M, *pY = p_Y;

  for (i=0; i < nb; i++, pK++)
  {
	*pC++ |= *pK;
	*pM++ |= *pK;
	*pY++ |= *pK;
  }
}




void	init_printer (FILE *fd)
{
  fputc(ESC,fd); fputc('E',fd);	/* Esc-E */
}




void	start_graphmode (PAR *p, int width, FILE *fd)
{
/**
 ** X & Y offsets: Use "decipoints" as unit to stick to PCL level 3
 **		1 dpt = 0.1 pt = 1/720 in
 **/
  if (p->yoff != 0.0)
	fprintf(fd,"\033&a%dV",(int)(p->yoff * 720.0 / 25.4) );
  if (p->xoff != 0.0)
	fprintf(fd,"\033&a%dH",(int)(p->xoff * 720.0 / 25.4) );

/**
 ** Set Graphics Resolution (300 / 150 / 100 / 75):
 ** This is NO PCL level 3 feature, but LaserjetII and compatibles
 ** seem to accept it.
 **/
  fprintf(fd,"\033*t%dR", p->dpi_x);

/**
 ** Set Raster Width (in dots)
 **	Deskjet feature, good for saving internal memory!
 **/
  if (p->specials)
  {
	fprintf(fd,"\033*r%dS", width);
	switch (p->specials)
	{
	  case 4:	/* KCMY 			*/
		fprintf(fd,"\033*r-4U");
		break;
	  case 3:	/* CMY				*/
		fprintf(fd,"\033*r-3U");
		break;
	  default:	/* Single color plane		*/
		fprintf(fd,"\033*r1U");
		break;
	}
  }

/**
 ** Start Raster Graphics at current position
 ** This is NO PCL level 3 feature, but LaserjetII and compatibles
 ** seem to accept it.
 **/
  fprintf(fd,"\033*r1A");
}




void	end_graphmode (FILE *fd)
{
/**
 ** End Raster Graphics
 **/
  fprintf(fd,"\033*rB");
}





void	PicBuf_to_PCL (PicBuf *picbuf, PAR *p)
/**
 ** Main interface routine
 **/
{
FILE	*fd = stdout;
RowBuf	*row;
int	row_c, i, x, color_index, offset;
Byte	mask;

  if (!p->quiet)
	fprintf(stderr, "\nWriting PCL output\n");

  if (picbuf->depth > 1 && p->specials < 3)
	fprintf(stderr,
	    "\nWARNING: Monochrome output despite active colors selected!\n");

  if (picbuf->depth > 1)	/* Allocate buffers for CMYK conversion	*/
  {
	p_K = calloc (picbuf->nb, sizeof(Byte));
	p_C = calloc (picbuf->nb, sizeof(Byte));
	p_M = calloc (picbuf->nb, sizeof(Byte));
	p_Y = calloc (picbuf->nb, sizeof(Byte));
	if (p_K == NULL || p_C == NULL || p_M == NULL || p_Y == NULL)
	{
		fprintf(stderr,
			"\nCannot 'calloc' CMYK memory -- sorry, use B/W!\n");
		goto PCL_exit;
	}
  }

  if (*p->outfile != '-')
  {
#ifdef VAX
	if ((fd = fopen(p->outfile, WRITE_BIN, "rfm=var","mrs=512")) == NULL)
	{
#else
	if ((fd = fopen(p->outfile, WRITE_BIN)) == NULL)
	{
#endif
		perror ("hp2xx -- opening output file");
		free_PicBuf (picbuf, p->swapfile);
		exit (ERROR);
	}
  }

  if (p->init_p)
	init_printer (fd);

  start_graphmode (p, picbuf->nc, fd);

  /**
   ** Loop for all rows:
   ** Counting back since highest index is lowest line on paper...
   **/

  for (row_c = picbuf->nr - 1; row_c >= 0; row_c--)
  {
	if ((!p->quiet) && (row_c % 10 == 0))
		  /* For the impatients among us ...	*/
		putc('.',stderr);

	row = get_RowBuf(picbuf, row_c);

	if (picbuf->depth == 1)
		Buf_to_PCL (row->buf, picbuf->nb, PCL_FIRST | PCL_LAST, fd);
	else
	{
		for (x=0; x < picbuf->nb; x++)
			p_K[x] = p_C[x] = p_M[x] = p_Y[x] = 0;

		for (x=offset=0; x < (picbuf->nb << 3); x++, offset = (x >> 3))
		{
			color_index = index_from_RowBuf(row, x, picbuf);

			if (color_index == xxBackground)
				continue;
			else
			{
				mask = 0x80;
				if ((i = x & 0x07) != 0)
					mask >>= i;
				switch (color_index)
				{
				  case xxForeground:
					*(p_K + offset) |= mask;
					break;
				  case xxRed:
					*(p_M + offset) |= mask;
					*(p_Y + offset) |= mask;
					break;
				  case xxGreen:
					*(p_C + offset) |= mask;
					*(p_Y + offset) |= mask;
					break;
				  case xxBlue:
					*(p_C + offset) |= mask;
					*(p_M + offset) |= mask;
					break;
				  case xxCyan:
					*(p_C + offset) |= mask;
					break;
				  case xxMagenta:
					*(p_M + offset) |= mask;
					break;
				  case xxYellow:
					*(p_Y + offset) |= mask;
					break;
				  default:
					break;
				}
			}
		}

		switch (p->specials)
		{
		  case 3:
			K_to_CMY (picbuf->nb);
			/* drop thru	*/
		  case 4:
			KCMY_Buf_to_PCL (picbuf->nb, (p->specials == 4), fd);
			break;
		  default:
			KCMY_to_K (picbuf->nb);
			Buf_to_PCL (p_K, picbuf->nb, PCL_FIRST | PCL_LAST, fd);
			break;
		}
	}
  }

  end_graphmode (fd);
  if (p->formfeed)
	putc (FF, fd);
  if (!p->quiet)
	fputc ('\n', stderr);
  if (fd != stdout)
	fclose (fd);

PCL_exit:
  if (p_Y != NULL)	free(p_Y);
  if (p_M != NULL)	free(p_M);
  if (p_C != NULL)	free(p_C);
  if (p_K != NULL)	free(p_K);

  p_K = p_C = p_M = p_Y = NULL;
}
