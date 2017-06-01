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
#include <stdio.h>

#include "internal.h"



/** \file drctry.c

    \brief The base directory (tsg) class

    The base directory class (edi_directory_t) holds the transaction
    set guidelines for a number of messages and state information for
    parsing the structure of a message.

*/

/**
   \defgroup edi_directory edi_directory
   \{
*/







/**
   \brief Prepare to parse a transaction

   \param self directory to use
   \param transaction name of the transaction (eg. "ORDERS")

   Initialise state for parsing the structure of a transaction
*/

edi_error_t edi_directory_start (edi_directory_t *self, char *transaction)
{
  edi_error_t err = EDI_ENONE;
  if(self && self->start)
    err = self->start(self, transaction);  
  return err;
}


/**
   \brief Parse the stucture of a transaction

   \param self directory to use
   \param code segment code to parse
   \param done non-zero if this is the last segment in the transaction
   \param userdata caller-supplied parameter
   \param start callback for start events
   \param end callback for end events
   \param segment callback for segment handling
   \param err callback for errors

*/

edi_error_t edi_directory_parse (edi_directory_t *self, char *code, int done,
				 void *userdata,
				 edi_eventh_t start,
				 edi_eventh_t end,
				 edi_sgmnth_t segment,
				 edi_eventh_t err)
{

  if(self && self->parse)
    self->parse(self, code, userdata, start, end, segment);
  else if(segment)
    segment(userdata, NULL, self);
  
  if(done && self && self->end)
    self->end(self);

  return EDI_ENONE;
}



































void edi_directory_free(edi_directory_t *self) {
  if(self && self->free)
    self->free(self);
}


int
edi_directory_element_index (edi_directory_t *self, char *key, int *x, int *y)
{
  int n;
  char code[256], *element = NULL, *subelement = NULL;
  
  strncpy (code, key, 256);
  code[255] = '\0';

  for (n = 0; code[n] != '\0'; n++)
    if (code[n] == '/')
      {
	code[n] = '\0';
	if (element)
	  subelement = code + n + 1;
	else
	  element = code + n + 1;
      }

  return self->element_indx ?
    self->element_indx (self, code, element, subelement, x, y) : -1;
}


char *
edi_directory_codelist_value
(edi_directory_t *self,
 char *element,
 char *value)
{
  if (!self || !element || !value)
    return NULL;

  return edi_directory_codelist_name (self, element, value);
}

















char *
edi_directory_element_name (edi_directory_t *self, char *ref)
{
  return self && self->element_name ? self->element_name (self, ref) : NULL;
}

char *
edi_directory_element_desc (edi_directory_t *self, char *ref)
{
  return self && self->element_desc ? self->element_desc (self, ref) : NULL;
}

char *
edi_directory_element_note (edi_directory_t *self, char *ref)
{
  return self && self->element_note ? self->element_note (self, ref) : NULL;
}

char *
edi_directory_segment_name (edi_directory_t *self, char *ref)
{
  return self && self->segment_name ? self->segment_name (self, ref) : NULL;
}

char *
edi_directory_segment_desc (edi_directory_t *self, char *ref)
{
  return self && self->segment_desc ? self->segment_desc (self, ref) : NULL;
}

char *
edi_directory_segment_note (edi_directory_t *self, char *ref)
{
  return self && self->segment_note ? self->segment_note (self, ref) : NULL;
}

char *
edi_directory_composite_name (edi_directory_t *self, char *ref)
{
  return self && self->composite_name ? self->composite_name (self,ref) : NULL;
}

char *
edi_directory_composite_desc (edi_directory_t *self, char *ref)
{
  return self && self->composite_desc ? self->composite_desc (self,ref) : NULL;
}

char *
edi_directory_composite_note (edi_directory_t *self, char *ref)
{
  return self && self->composite_note ? self->composite_note (self,ref) : NULL;
}

int
edi_directory_is_composite (edi_directory_t *self, char *t, char *ref)
{
  return self && self->is_composite ? self->is_composite (self, t, ref) : 0;
}

char
edi_directory_segment_reqr (edi_directory_t *self, char *t, char *ref)
{
  return self && self->segment_reqr ? self->segment_reqr (self, t, ref) : 'C';
}

char
edi_directory_composite_reqr (edi_directory_t *self, char *t, char *ref)
{
  return self && self->composite_reqr ?
    self->composite_reqr (self, t, ref) : 'C';
}

char *
edi_directory_codelist_name (edi_directory_t *self, char *c, char *v)
{
  return self && self->codelist_name ? self->codelist_name (self, c, v) : NULL;
}

char *
edi_directory_codelist_desc (edi_directory_t *self, char *c, char *v)
{
  return self && self->codelist_desc ? self->codelist_desc (self, c, v) : NULL;
}

char *
edi_directory_codelist_note (edi_directory_t *self, char *c, char *v)
{
  return self && self->codelist_note ? self->codelist_note (self, c, v) : NULL;
}


unsigned int edi_directory_segment_size (edi_directory_t *self, char *code)
{
  return self && self->segment_size ? self->segment_size(self, code) : 0;
}

unsigned int edi_directory_composite_size (edi_directory_t *self, char *code)
{
  return self && self->composite_size ? self->composite_size(self, code) : 0;
}


edi_item_t edi_directory_composite_item
(edi_directory_t *self, char *code, unsigned int i)
{
  edi_item_t item = EDI_NULL_ITEM;
  return self && self->composite_item && code ?
    self->composite_item(self, code, i) : item;
}

edi_item_t edi_directory_segment_item
(edi_directory_t *self, char *code, unsigned int i)
{
  edi_item_t item = EDI_NULL_ITEM;
  return self && self->segment_item && code ?
    self->segment_item(self, code, i) : item;
}


edi_item_t edi_directory_element_repr
(edi_directory_t *self, char *code)
{
  edi_item_t item = EDI_NULL_ITEM;
  return self && self->element_repr && code ?
    self->element_repr(self, code) : item;
}







char *edi_get_element_by_name
(edi_directory_t *directory, edi_segment_t *segment,
 char *element, char *subelement)
{
  edi_item_t item;
  unsigned int ss, cs, e, s;
  char *tag = edi_segment_get_code(segment);

  if(!(ss = edi_directory_segment_size(directory, tag)))
    return NULL;
  
  for(e = 0; e < ss; e++)
    {
      item = edi_directory_segment_item(directory, tag, e);
      if(item.code && !strcmp(item.code, element))
	goto found_element;
    }

  return NULL;

 found_element:
  
  /* is this a simple element, or composite? */
  if(!item.type)
    return edi_segment_get_element(segment, e, 0);

  if(!(cs = edi_directory_composite_size(directory, element)))
    return NULL;
  
  for(s = 0; s < cs; s++)
    {
      item = edi_directory_composite_item(directory, element, s);
      if(item.code && !strcmp(item.code, subelement))
        goto found_subelement;
    }

  return NULL;

 found_subelement:
  return edi_segment_get_element(segment, e, s);
}

/** \} */





char *edi_directory_find_element
(edi_directory_t *d, char *s, int e, int se)
{
  edi_item_t element;

  element = edi_directory_segment_item(d, s, e);

  if(!element.code)
    return NULL;
  
  if(element.type)
    {
      element = edi_directory_composite_item(d, element.code, se);
    }
  else
    {
      if(se)
	return NULL;
    }

  return element.code;
}


char *edi_directory_find_composite
(edi_directory_t *d, char *s, int e)
{
  edi_item_t element;

  element = edi_directory_segment_item(d, s, e);

  if(!element.code || !element.type)
    return NULL;

  return element.code;
}




















/**********************************************************************
 * OBSOLETE
 **********************************************************************/



/**
   \brief Start parsing the stucture of a transaction with the initial segment

   \param self directory to use
   \param segment the first segment in the transaction
   \param parameters parameters to pass to the client application
   \param parser parser object used by the client application
   \param transaction name of the transaction (eg. ORDERS)

   Initialise state for parsing the structure of a transaction
*/

static edi_error_t parse_segment
(edi_directory_t *self,
 edi_parser_t *parser,
 edi_segment_t *segment,
 edi_parameters_t *parameters)
{
  if(!self)
    {
      edi_parser_handle_segment (parser, NULL, self);
      return EDI_ENONE;
    }
  
  if(self->parse)
    return self->parse(self, edi_segment_get_code(segment), parser,
		       (edi_eventh_t) edi_parser_handle_start,
		       (edi_eventh_t) edi_parser_handle_end,
		       (edi_sgmnth_t) edi_parser_handle_segment);
  
  /*if(self->old_parse)
    return self->old_parse(self, segment, parameters, parser, self);*/

  edi_parser_handle_segment (parser, parameters, self);
  return EDI_ENONE;
}

edi_error_t edi_directory_head
(edi_directory_t *self,
 edi_segment_t *segment,
 edi_parameters_t *parameters,
 edi_parser_t *parser,
 char *transaction)
{
  edi_error_t error = EDI_ENONE;
  if(self && self->start)
    error = self->start(self, transaction);

  if(error)
    edi_parser_raise_error(parser, error);
  
  return parse_segment(self, parser, segment, parameters);
}


/**
   \brief Continue parsing the stucture of a transaction with the
   next segment

   \param self directory to use
   \param segment the next segment in the transaction
   \param parameters parameters to pass to the client application
   \param parser parser object used by the client application

*/

edi_error_t edi_directory_body
(edi_directory_t *self,
 edi_segment_t *segment,
 edi_parameters_t *parameters,
 edi_parser_t *parser)
{
  edi_error_t error = EDI_ENONE;

  error = parse_segment(self, parser, segment, parameters);  
  
  if(error)
    edi_parser_raise_error(parser, error);

  return error;
}


/**
   \brief Finish parsing the stucture of a transaction with the final segment
     
   \param self directory to use
   \param segment the final segment in the transaction
   \param parameters parameters to pass to the client application
   \param parser parser object used by the client application
*/

edi_error_t edi_directory_tail
(edi_directory_t *self,
 edi_segment_t *segment,
 edi_parameters_t *parameters,
 edi_parser_t *parser)
{
  parse_segment(self, parser, segment, parameters);  

  if(self && self->end)
    return self->end(self);
  else
    return EDI_ENONE;
}



