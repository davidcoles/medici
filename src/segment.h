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

#ifndef SEGMENT_H
#define SEGMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define EDI_BUFFER  1024
#define EDI_NELEMS  32

/* FIXME - handle explicit nesting/repetition in EDIFACT */
typedef struct edi_segment_s
{
  char tag[EDI_BUFFER];
  edi_buffer_t elements[EDI_NELEMS][EDI_NELEMS];
  int defined[EDI_NELEMS][EDI_NELEMS];
  int de, cde[EDI_NELEMS];
}
edi_segment_t;


void edi_segment_init(edi_segment_t *s);
void edi_segment_clear(edi_segment_t *);
void edi_segment_free(edi_segment_t *);
edi_segment_t *edi_segment_create(void);
char *edi_segment_get_code(edi_segment_t *s);
int edi_segment_cmp_code(edi_segment_t *s, char *c);
char *edi_segment_get_element(edi_segment_t *s, int x, int y);
void edi_segment_set_code(edi_segment_t *s, char *c, int l);
void edi_segment_set_element(edi_segment_t *s, int x, int y, char *c, int l);
int edi_segment_get_element_count(edi_segment_t *s);
int edi_segment_get_subelement_count(edi_segment_t *s, int n);
void edi_segment_copy(edi_segment_t *dst, edi_segment_t *src);
edi_segment_t *edi_segment_dup(edi_segment_t *src);

#ifdef __cplusplus
}
#endif

#endif /*SEGMENT_H*/
