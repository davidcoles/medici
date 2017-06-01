/*  
    
  The MEDICI Electronic Data Interchange Library
  Copyright (C) 2002  David Coles
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
*/

#include <stdlib.h>
#include <string.h>

#include "adt.h"
#include "segment.h"

void
edi_segment_init (edi_segment_t *s)
{
  int n, m;

  s->de = 0;
  for (n = 0; n < EDI_NELEMS; n++)
    s->cde[n] = 0;

  for (n = 0; n < EDI_NELEMS; n++)
    for (m = 0; m < EDI_NELEMS; m++)
      s->defined[n][m] = 0;
}

void
edi_segment_clear (edi_segment_t *self)
{
  int n, m;
  
  if(!self)
    return;
  
  self->de = 0;
  for (n = 0; n < EDI_NELEMS; n++)
    self->cde[n] = 0;
  
  for (n = 0; n < EDI_NELEMS; n++)
    for (m = 0; m < EDI_NELEMS; m++)
      if(self->defined[n][m])
	{
	  self->defined[n][m] = 0;
	  edi_buffer_clear(&(self->elements[n][m]));
	}
}


void
edi_segment_free (edi_segment_t *self)
{
  if (!self)
    return;
  
  edi_segment_clear (self);
  
  free (self);
}

edi_segment_t *
edi_segment_create (void)
{
  edi_segment_t *s;

  if ((s = (edi_segment_t *) malloc (sizeof (edi_segment_t))))
    edi_segment_init (s);
  return s;
}

char *
edi_segment_get_code (edi_segment_t *s)
{
  return s->tag;
}

int
edi_segment_cmp_code (edi_segment_t *s, char *c)
{
  return strncmp (s->tag, c, strlen (c));
}

char *
edi_segment_get_element (edi_segment_t *s, int x, int y)
{
  return s->defined[x][y] ?
    (char *) edi_buffer_data(&(s->elements[x][y])) : NULL;
}

void
edi_segment_set_code (edi_segment_t *s, char *c, int l)
{
  int len = (l > EDI_BUFFER) ? EDI_BUFFER : l;
  strncpy (s->tag, c, len);
  s->tag[len] = '\0';
}

void
edi_segment_set_element (edi_segment_t *s, int x, int y, char *c, int l)
{
  /* FIXME - temporary bounds check */
  if(x >= EDI_NELEMS || y >= EDI_NELEMS)
    return;
  
  /**********************************************************************
   * NB: The element MUST set 'defined' to 1 if this function is
   * called, even if the pointer is NULL or length is zero.  An
   * element may be MANDATORY, but also able to have ZERO length.  It
   * is up to the syntax handling code to decide which elements are to
   * be defined and which are not.
  **********************************************************************/

  if(s->defined[x][y])
    edi_buffer_clear(&(s->elements[x][y]));
  else
    edi_buffer_init(&(s->elements[x][y]));

  s->defined[x][y] = 1;
  
  if(c && l)
    edi_buffer_append(&(s->elements[x][y]), c, l);
  
  if (s->de < (x + 1))
    s->de = (x + 1);
  
  if (s->cde[x] < (y + 1))
    s->cde[x] = (y + 1);
}

int
edi_segment_get_element_count (edi_segment_t *s)
{
  return s->de;
}

int
edi_segment_get_subelement_count (edi_segment_t *s, int n)
{
  return s->cde[n];
}

void
edi_segment_copy (edi_segment_t *dst, edi_segment_t *src)
{
  int x, y;

  *dst = *src;

  for (x = 0; x < EDI_NELEMS; x++)
    for (y = 0; y < EDI_NELEMS; y++)
      if(src->defined[x][y])
	{
	  dst->defined[x][y] = 0;
	  edi_segment_set_element(dst, x, y, (char *)
				  edi_buffer_data(&(src->elements[x][y])),
				  edi_buffer_size(&(src->elements[x][y])));
      }
}

edi_segment_t *
edi_segment_dup (edi_segment_t *src)
{
  edi_segment_t *dst = NULL;

  if((dst = edi_segment_create()))
     edi_segment_copy (dst, src);
	
  return dst;
}



