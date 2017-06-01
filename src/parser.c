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

/** \file parser.c

    \brief The base parser class

    The base parser class (edi_parser_t) holds a deal of state
    information for the parse run.

*/

/**
   \defgroup edi_parser edi_parser
   \{
*/



/**
   \brief Initialises a freshly allocated parser structure.
   \param self pointer to the edi_parser_s structure to initialise.

   Mainly internal function (to be run by edi_parser_create) to
   initialise flags, counters, function pointers and ADTs.

*/

static void edi_parser_init_handlers(edi_parser_t *self)
{
  edi_parser_set_token_handler (self, NULL);
  edi_parser_set_segment_handler (self, NULL);

  edi_parser_set_error_handler (self, NULL);
  edi_parser_set_warning_handler (self, NULL);

  edi_parser_set_directory_handler (self, NULL);

  edi_parser_set_user_data (self, NULL);

  edi_parser_set_start_handler (self, NULL);
  edi_parser_set_end_handler (self, NULL);

  edi_parser_set_text_handler (self, NULL);
  edi_parser_set_separator_handler (self, NULL);
  edi_parser_set_default_handler (self, NULL);

  edi_parser_set_complete_handler (self, NULL);
}

static void edi_parser_init_tokeniser(edi_parser_t *self)
{
  edi_tokeniser_init(&(self->tokeniser));
  self->tokeniser.user_data = self;
  self->tokeniser.itype_handler = edi_parser_itype_handler;
  self->tokeniser.cmplt_handler = edi_parser_cmplt_handler;
  self->tokeniser.token_handler = edi_parser_token_handler;
  self->tokeniser.error_handler = (edi_error_handler_t) edi_parser_raise_error;
}

static void edi_parser_init_state(edi_parser_t *self)
{
  self->done = 0;
  self->de = 0;
  self->cde = 0;
  self->segment_count = 0;
  self->error = EDI_ENONE;
  self->interchange_type = EDI_UNKNOWN;
}

static void edi_parser_init_dynamic(edi_parser_t *self)
{
  edi_list_init(&(self->token_queue));
  edi_buffer_init (&(self->parse_buffer));
  edi_stack_init (&(self->stack));
  self->advice = &(self->tokeniser.advice);
  self->segment = edi_segment_create ();
}

static void edi_parser_init_syntax(edi_parser_t *self)
{
  self->syntax_fini = NULL;
  self->sgmnt_handler = NULL;
}

void
edi_parser_init (edi_parser_t *self)
{
  memset(self, 0, sizeof(edi_parser_t)); /* mitigate bugs */
  
  self->pragma = EDI_PCHARSET | EDI_PTUNKNOWN | EDI_PSEGMENT;
  edi_parser_init_handlers(self);
  
  edi_parser_init_state(self);
  edi_parser_init_tokeniser(self);
  edi_parser_init_dynamic(self);
  edi_parser_init_syntax(self);
}

/* must avoid clearing stuff the app has set up - such as callback handlers */
void edi_parser_reset (edi_parser_t *self)
{
  edi_parser_fini (self);

  edi_parser_init_state(self);
  edi_parser_init_tokeniser(self);
  edi_parser_init_dynamic(self);
  edi_parser_init_syntax(self);
}













/**********************************************************************
 * User-level functions for creating, parsing and freeing
 **********************************************************************/

/**
   \brief Creates a parser structure and returns a pointer to it.
   \param type Interchange type if known, otherwise EDI_ANY.
   \return Pointer to the new parser structure, or NULL on failure.

   The type should be EDI_ANY to create a parser which will accept any
   of the implemented syntaxes. Using a value other than EDI_ANY
   (eg. EDI_X12) will create a parser which will only parse a stream
   of that type - streams with other syntaxes, though valid and
   implemented will cause a parse failure.

   If a NULL value is returned this will almost certainly be because
   of a failure to allocate memory; you should consult errno(3) or
   whatever your platform uses for details.

*/

edi_parser_t *
edi_parser_create (edi_interchange_type_t type)
{
  edi_parser_t *self;

  if (!(self = (edi_parser_t *) malloc (sizeof (edi_parser_t))))
    return NULL;
  
  edi_parser_init (self);
  /*edi_parser_itype_handler(self, type);*/

  return self;
}


void edi_parser_fini (edi_parser_t *self)
{
  if (!self)
    return;

  if (self->syntax_fini)
    self->syntax_fini (self);

  edi_buffer_clear (&(self->parse_buffer));
  edi_stack_clear (&(self->stack), free);
  edi_list_clear (&(self->token_queue), free);

  if (self->segment)
    edi_segment_free (self->segment);
}

/**
   \brief Frees all resources allocated to this parser.
   \param self Pointer to the parser to be freed.
*/

void
edi_parser_free (edi_parser_t *self)
{
  edi_parser_fini (self);
  free (self);
}


/**
   \brief Parse a chunk of an EDI stream,
   \param self Pointer to the parser which should parse the chunk.
   \param buffer Pointer to the buffer containing the chunk.
   \param length Length in characters of the chunk.
   \param done This should be set to non-zero if this is know to be the
   last chunk in the stream (zero otherwise).
   \return Number of characters consumed by the parser.

   If the number of characters consumed by the parser is less than the
   size of the chunk passed in then either the parser encountered an
   error and aborted or the interchange parsed successfully and there
   is trailing garbage (eg. another interchange concatenated on the
   end).
*/

long
edi_parser_parse (edi_parser_t *self, char *buffer, long length, int done)
{  
  return edi_tokeniser_parse(&(self->tokeniser), buffer, length, done);
}











/**********************************************************************
 * Some statistical/informational reporting functions
 **********************************************************************/

/** \brief Returns the byte offset of the position in the stream. */
unsigned long
edi_parser_get_byte_index (edi_parser_t *self)
{
  return self ? self->tokeniser.byte_count : 0;
}

/** \brief Returns the segment index of the position in the stream. */
unsigned long
edi_parser_get_segment_index (edi_parser_t *self)
{
  return self ? self->segment_count : 0;
}

/** \brief Returns the type of error that has occured. */
int
edi_parser_get_error_code (edi_parser_t *self)
{
  return self ? (int) self->error : 0;
}

/** \brief Returns the interchange type of the stream. */
edi_interchange_type_t
edi_parser_interchange_type (edi_parser_t *self)
{
  return self ? self->interchange_type : EDI_UNKNOWN;
}

/** \brief Obsolete. */
edi_parameters_t* edi_parser_info (edi_parser_t *self)
{
  /*return (self && self->parser_info) ? self->parser_info (self) : NULL;*/
  return NULL;
}

/** \brief Returns a pointer to the service directory */
edi_directory_t *edi_parser_service(edi_parser_t *self)
{
  return self->service;
}

/** \brief Returns a pointer to the message directory */
edi_directory_t *edi_parser_message(edi_parser_t *self)
{
  return self->message;
}

edi_pragma_t
edi_set_pragma_t (edi_parser_t *p, edi_pragma_t pragma)
{
  edi_pragma_t old = p->pragma;
  p->pragma = pragma;
  return (old);
}


int edi_parser_is_complete(edi_parser_t *self)
{
  return self->done;
}











/**********************************************************************
 * Deal with potential errors raised by various subsystems
 **********************************************************************/

/** \brief Error handling is not currently well-defined */
edi_error_t
edi_parser_raise_error (edi_parser_t *self, edi_error_t error)
{
  int warning = 0;

  if(error == EDI_ENONE)
    return error;

  if(self->pragma & (1<<error))
    warning = 1;
  
  if (warning && self->warning_handler)
    self->warning_handler (self->user_data, error);
  
  if (!warning && self->error_handler)
    self->error_handler (self->user_data, error);
  
  if (error == EDI_ESYNTAX || !warning)
    {
      self->error = error;
      self->tokeniser.error = error;
    }

  
  return warning ? EDI_ENONE : error;
}















/**********************************************************************
 * Wrapper functions for handing events off to the client application
 **********************************************************************/

/** \brief Notifies the client of a complete segment */
void edi_parser_handle_segment
(edi_parser_t *self, edi_parameters_t *p, edi_directory_t *d)
{
  /* in edi_parser_segment_events */
  /*if (self->segment_handler)
    self->segment_handler (self->user_data, p, self->segment, d);*/

    edi_parser_segment_events (self, d);
}

/** \brief Requests the client for a directory (to parse transaction) */
edi_directory_t *
edi_parser_handle_directory (edi_parser_t *self, edi_parameters_t *p)
{
  return (self->directory_handler ?
	  self->directory_handler (self->user_data, p) : NULL);
}

/** \brief Notifies the client of the start of a structural event */
void edi_parser_handle_start
(edi_parser_t *self, edi_event_t event, edi_parameters_t *p)
{
  if (self->start_handler)
    self->start_handler (self->user_data, event, p);
}

/** \brief Notifies the client of the end of a structural event */
void edi_parser_handle_end
(edi_parser_t *self, edi_event_t event, edi_parameters_t *p)
{
  if (self->end_handler)
    self->end_handler (self->user_data, event);
}

/** \brief Notifies the client of a (possibly partial) token */
void
edi_parser_handle_token (edi_parser_t *self, edi_token_t *token)
{
  if (self->token_handler)
    self->token_handler (self->user_data, token);
}

void
edi_parser_handle_text (edi_parser_t *self, char *text, int size)
{
  if (self->text_handler)
    self->text_handler (self->user_data, text, size);
  else
    edi_parser_handle_default(self, text, size);
}

void
edi_parser_handle_default
(edi_parser_t *self, char *text, int size)
{
  if (self->default_handler)
    self->default_handler (self->user_data, text, size);
}

void edi_parser_handle_separator
(edi_parser_t *self, edi_event_t event, char separator)
{
  if(self->separator_handler)
    self->separator_handler(self->user_data, event, separator);
  else
    edi_parser_handle_default(self, &separator, 1);
}









/**********************************************************************
 * Pass segments to tsg for message structure parsing
 **********************************************************************/

/* define this at some point */
#define edi_parser_handle_error NULL


void edi_parser_transaction_head
(edi_parser_t *self,
 edi_segment_t *segment,
 edi_directory_t *directory,
 char *transaction)
{
  edi_directory_start(directory, transaction);  
  edi_directory_parse(directory, edi_segment_get_code(segment), 0, self,
		      (edi_eventh_t) edi_parser_handle_start,
		      (edi_eventh_t) edi_parser_handle_end,
		      (edi_sgmnth_t) edi_parser_handle_segment,
		      edi_parser_handle_error);
}

void edi_parser_transaction_body
(edi_parser_t *self,
 edi_segment_t *segment,
 edi_directory_t *directory)
{
  edi_directory_parse(directory, edi_segment_get_code(segment), 0, self,
		      (edi_eventh_t) edi_parser_handle_start,
		      (edi_eventh_t) edi_parser_handle_end,
		      (edi_sgmnth_t) edi_parser_handle_segment,
		      edi_parser_handle_error);
}

void edi_parser_transaction_tail
(edi_parser_t *self,
 edi_segment_t *segment,
 edi_directory_t *directory)
{

  edi_directory_parse(directory, edi_segment_get_code(segment), 1, self,
                      (edi_eventh_t) edi_parser_handle_start,
                      (edi_eventh_t) edi_parser_handle_end,
                      (edi_sgmnth_t) edi_parser_handle_segment,
                      edi_parser_handle_error);
}














/**********************************************************************
 * Functions for building up the segment data structure
 **********************************************************************/

static void edi_parser_end_tag (edi_parser_t *self)
{
  edi_segment_set_code (self->segment, (char *)
			edi_buffer_data (&(self->parse_buffer)),
			edi_buffer_size (&(self->parse_buffer)));
  edi_buffer_clear (&(self->parse_buffer));
  self->de = 0;
  self->cde = 0;
}

static void edi_parser_end_element (edi_parser_t *self)
{
  edi_segment_set_element (self->segment, self->de++, self->cde, (char *)
			   edi_buffer_data (&(self->parse_buffer)),
			   edi_buffer_size (&(self->parse_buffer)));
  edi_buffer_clear (&(self->parse_buffer));
  self->cde = 0;
}

static void edi_parser_end_subelement (edi_parser_t *self)
{
  edi_segment_set_element (self->segment, self->de, self->cde++, (char *)
			   edi_buffer_data (&(self->parse_buffer)),
			   edi_buffer_size (&(self->parse_buffer)));
  edi_buffer_clear (&(self->parse_buffer));
}

static void edi_parser_new_segment (edi_parser_t *self)
{
  edi_buffer_clear (&(self->parse_buffer));
  edi_segment_clear (self->segment);
  self->segment_count++;
  self->cde = 0;
  self->de = 0;
}


























/**********************************************************************
 * Functions for building an internal progress stack
 **********************************************************************/

int
edi_parser_push_segment (edi_parser_t *self, edi_segment_t *segment)
{
  edi_segment_t *copy;
  
  if((copy = edi_segment_dup (segment)))
    edi_stack_push (&(self->stack), copy);
  
  return copy ? 1 : 0;
}

void
edi_parser_pop_segment (edi_parser_t *self)
{
  edi_segment_free ((edi_segment_t *) edi_stack_pop (&(self->stack)));
}

edi_segment_t *
edi_parser_peek_segment (edi_parser_t *self)
{
  return (edi_segment_t *) edi_stack_peek (&self->stack);
}


























/**********************************************************************
 * Functions for the client application to set its handler functions
 **********************************************************************/

edi_start_handler_t
edi_parser_set_start_handler (edi_parser_t *self, edi_structure_handler_t h)
{
  edi_start_handler_t old = self->start_handler;
  self->start_handler = h;
  return old;
}

edi_end_handler_t
edi_parser_set_end_handler (edi_parser_t *self, edi_end_handler_t h)
{
  edi_end_handler_t old = self->end_handler;
  self->end_handler = h;
  return old;
}

edi_error_handler_t
edi_parser_set_error_handler (edi_parser_t *self, edi_error_handler_t h)
{
  edi_error_handler_t old = self->error_handler;
  self->error_handler = h;
  return old;
}

edi_error_handler_t
edi_parser_set_warning_handler (edi_parser_t *self, edi_error_handler_t h)
{
  edi_error_handler_t old = self->warning_handler;
  self->warning_handler = h;
  return old;
}

edi_token_handler_t
edi_parser_set_token_handler (edi_parser_t *self, edi_token_handler_t h)
{
  edi_token_handler_t old = self->token_handler;
  self->token_handler = h;
  return old;
}

edi_directory_handler_t
edi_parser_set_directory_handler (edi_parser_t *self, edi_directory_handler_t d)
{
  edi_directory_handler_t old = self->directory_handler;
  self->directory_handler = d;
  return old;
}

edi_segment_handler_t
edi_parser_set_segment_handler (edi_parser_t *self, edi_segment_handler_t h)
{
  edi_segment_handler_t old = self->segment_handler;
  self->segment_handler = h;
  return old;
}

edi_character_handler_t
edi_parser_set_text_handler (edi_parser_t *self, edi_character_handler_t h)
{
  edi_character_handler_t old = self->text_handler;
  self->text_handler = h;
  return old;
}

edi_character_handler_t
edi_parser_set_default_handler (edi_parser_t *self, edi_character_handler_t h)
{
  edi_character_handler_t old = self->default_handler;
  self->default_handler = h;
  return old;
}

edi_complete_handler_t
edi_parser_set_complete_handler (edi_parser_t *self, edi_complete_handler_t h)
{
  edi_complete_handler_t old = self->complete_handler;
  self->complete_handler = h;
  return old;
}

edi_separator_handler_t
edi_parser_set_separator_handler
(edi_parser_t *self, edi_separator_handler_t h)
{
  edi_separator_handler_t old = self->separator_handler;
  self->separator_handler = h;
  return old;
}


void *
edi_parser_set_user_data (edi_parser_t *self, void *user_data)
{
  void *old = self->user_data;
  self->user_data = user_data;
  return old;
}




























/**********************************************************************
 * Handlers for events caused by lower level parser
 **********************************************************************/


/**
   \brief Called when the tokeniser completes a token
   \param v Pointer to the edi_parser_t structure
   \param token Pointer to the completed token

   When a token is completed by the low-level tokeniser this function
   is called with a pointer to the completed token. A sequence of
   tokens is used to build an edi_segment_t structure and tokens are
   also saved in a queue for processing after any events caused by
   (but logically preceeding) the current segment have been
   dispatched.
*/

int
edi_parser_token_handler (void *v, edi_token_t *token)
{
  edi_parser_t *self = (edi_parser_t *) v;
  edi_token_t *copy_of_token;
  
  if((copy_of_token = (edi_token_t *) malloc(sizeof(edi_token_t))))
    {
      *copy_of_token = *token;
      edi_queue_queue(&(self->token_queue), copy_of_token);
    }
  
  switch(token->type)
    {
    case EDI_TST:
      edi_parser_end_element(self);
      if(self->sgmnt_handler)
	self->sgmnt_handler(self);
      edi_parser_new_segment(self);
      /* FIXME - this should now be handled within the segment/events */
      /* handler but it doesn't hurt to mop up here for now just in case */
      while((token = (edi_token_t *) edi_queue_dequeue(&(self->token_queue))))
	{
	  fprintf(stderr, "DEBUG: Cleaning up stray token (%.*s)!\n",
		  (int) token->csize, token->cdata);
	  free(token);
	}
      break;
      
    case EDI_TSS:
      edi_parser_end_subelement(self);
      break;
      
    case EDI_TES:
      edi_parser_end_element(self);
      break;
      
    case EDI_TTS:	  
      edi_parser_end_tag(self);
      break;
      
    case EDI_TTG:
    case EDI_TEL:
      edi_buffer_append(&(self->parse_buffer), token->cdata, token->csize);
      break;
      
    default:
      /* FIXME unknown token type - do nothing */
      break;
    }

  return self->done;
}



/**
   \brief Called when the tokeniser recognises the interchange type (syntax)
   \param v Pointer to the edi_parser_t structure
   \param type Interchange type of the stream

   When the low-level tokeniser recognises the syntax that is in use
   this function is called and is used to "morph" the parser in to the
   respective syntax parser. This is done by calling the appropriate
   syntax "_init" function which sets various function pointers on the
   edi_parser_s structure.
*/

void edi_parser_itype_handler
(void *v, edi_interchange_type_t type)
{
  edi_parser_t *self = (edi_parser_t *) v;

  self->interchange_type = type;

  switch(type) {
  case EDI_EDIFACT:
    edi_edifact_init(self);
    break;
    
  case EDI_UNGTDI:
    edi_ungtdi_init(self);
    break;
    
  case EDI_X12:
    edi_x12_init(self);
    break;

  case EDI_IMP:
    edi_imp_init(self);
    break;
    
  case EDI_UNKNOWN:
    return;
  }
}

void edi_parser_cmplt_handler (void *v)
{
  edi_parser_t *self = (edi_parser_t *) v;
  
  if(self->complete_handler)
    self->complete_handler(self->user_data);
}






static void handle_advice(edi_parser_t *self, edi_token_t *token)
{
  edi_parameters_t parameters, *p;
  char advice[7][2];

  p = &parameters;
  edi_parameters_set(p, NULL);

  memset(&advice, 0, sizeof(advice));
  
  if(edi_advice_get_rs(self->advice, advice[0]))
    edi_parameters_set_one(p, RepetitionSeparator, advice[0]);
  
  if(edi_advice_get_ts(self->advice, advice[1]))
    edi_parameters_set_one(p, TagSeparator, advice[1]);
  
  if(edi_advice_get_es(self->advice, advice[2]))
    edi_parameters_set_one(p, ElementSeparator, advice[2]);
  
  if(edi_advice_get_ss(self->advice, advice[3]))
      edi_parameters_set_one(p, SubelementSeparator, advice[3]);
  
  if(edi_advice_get_st(self->advice, advice[4]))
    edi_parameters_set_one(p, SegmentTerminator, advice[4]);
  
  if(edi_advice_get_ri(self->advice, advice[5]))
    edi_parameters_set_one(p, ReleaseIndicator, advice[5]);
  
  if(edi_advice_get_dn(self->advice, advice[6]))
    edi_parameters_set_one(p, DecimalNotation, advice[6]);
  
  edi_parser_handle_start (self, EDI_ADVICE, p);
  edi_parser_handle_default (self, token->cdata, token->csize);
  edi_parser_handle_end (self, EDI_ADVICE, NULL);  
}




typedef struct
{
  char tag[EDI_TOKEN_MAX+1];
  edi_directory_t *d;
  int e;
  int s;
} edi_element_info_t;




static void
handle_chars(edi_parser_t *self, edi_event_t event, edi_token_t *token,
	     edi_element_info_t info)
{
  unsigned int n = 0, x = 0;
  char ei[EDI_TOKEN_MAX], si[EDI_TOKEN_MAX];
  char *code = NULL, *name = NULL, *desc = NULL, *note = NULL, *list = NULL;
  edi_directory_t *d = info.d;
  edi_parameters_t parameters, *p;
  
  p = &parameters;
  
  edi_parameters_set(p, NULL);

  if(token->first)
    {
      ei[0] = '\0';
      si[0] = '\0';
      
      if(token->type != EDI_TTG) {
	code = edi_directory_find_element(d, info.tag, info.e, info.s);
	name = edi_directory_element_name(d, code);
	desc = edi_directory_element_desc(d, code);
	note = edi_directory_element_note(d, code);
	list = edi_directory_codelist_value(d, code, token->cdata);
	
	sprintf(ei, "%d", info.e);
	sprintf(si, "%d", info.s);
	
	p->element = info.e;
	p->subelement = info.s;
      }

      edi_parameters_set_one(p, Element, ei[0] ? ei : NULL);
      edi_parameters_set_one(p, Subelement, si[0] ? si : NULL);
      
      edi_parameters_set_one(p, Code, code);
      edi_parameters_set_one(p, Name, name);
      edi_parameters_set_one(p, Desc, desc);
      edi_parameters_set_one(p, Note, note);
      edi_parameters_set_one(p, List, list);
    }



  if(token->first)
    edi_parser_handle_start (self, event, p);
  

  /* if the raw and cooked buffer sizes are not the same then there
     must be release indicator characters in the element. */
  
  if(token->rsize != token->csize)
    {
      for(n = 0; n < token->rsize; n++)
	{
	  if(token->ri[n])
	    {
	      if(n > x)
		edi_parser_handle_text (self, token->rdata + x, n - x);
	      edi_parser_handle_separator(self, EDI_RI, token->rdata[n]);
	      
	      x = n + 1;
	    }
	}
      
      if(n > x)
	edi_parser_handle_text (self, token->rdata + x, n - x);
      
    }
  else
    {
      if(token->csize)
	edi_parser_handle_text (self, token->cdata, token->csize);
    }
  
  
  if(token->last)
    edi_parser_handle_end (self, event, NULL);
}







/* FIXME - EXPERIMENTAL */
void edi_parser_segment_events
(edi_parser_t *self, edi_directory_t *d)
{
  char *c;
  char ei[EDI_TOKEN_MAX];
  unsigned int composite = 0;
  edi_token_t *token;
  edi_event_t event;
  edi_parameters_t pxx, *px;
  edi_element_info_t elem_info;

  memset(&elem_info, 0, sizeof(elem_info));

  px = &pxx;
  edi_parameters_set(px, NULL);
  
  /* FIXME - temporary */
  if (self->segment_handler)
    self->segment_handler (self->user_data, px, self->segment, d);
  
  while((token = (edi_token_t *) edi_queue_dequeue(&(self->token_queue))))
    {
      /* higher level token handler - mostly obsolete really */
      edi_parser_handle_token(self, token);
      
      
      /* initialise parameters struct */
      edi_parameters_set(px, NULL);
      
      /* some code to start/end sections depending on the token types */


      /* if we are at the start of a tag then we need to start the segment */
      if(token->type == EDI_TTG && token->first)
	{
	  if(!token->last)
	    {
	      /* unusually long tag data - this is surely bogus */
	      /* FIXME - flag a warning or error here */
	    }
	  
	  c = token->cdata;
	  edi_parameters_set_one(px, Code, c);
          edi_parameters_set_one(px, Name, edi_directory_segment_name(d, c));
          edi_parameters_set_one(px, Desc, edi_directory_segment_desc(d, c));
          edi_parameters_set_one(px, Note, edi_directory_segment_note(d, c));
	  
	  edi_parser_handle_start (self, EDI_SEGMENT, px);
	  strncpy(elem_info.tag, token->cdata, EDI_TOKEN_MAX);
	  elem_info.tag[EDI_TOKEN_MAX] = '\0';
	}
      
      
      /* if we are at the start of a composite then we need to signal it */
      if(token->type == EDI_TEL && token->first && elem_info.s == 0 &&
	 ((c = edi_directory_find_composite(d, elem_info.tag, elem_info.e)) ||
	  token->is_se))
	{
	  sprintf(ei, "%d", elem_info.e);
	  
	  px->element = elem_info.e;
	  
	  edi_parameters_set_one(px, Element, ei);
	  
	  edi_parameters_set_one(px, Code, c);
	  edi_parameters_set_one(px, Name, edi_directory_composite_name(d, c));
	  edi_parameters_set_one(px, Desc, edi_directory_composite_desc(d, c));
	  edi_parameters_set_one(px, Note, edi_directory_composite_note(d, c));
	  
	  edi_parser_handle_start (self, EDI_COMPOSITE, px);
	  composite = 1;
	}

      /* end a composite? */
      if((token->type == EDI_TES || token->type == EDI_TST) && token->first &&
	 composite)
	{
	  edi_parser_handle_end (self, EDI_COMPOSITE, NULL);
	  composite = 0;
	}
      





      /* now handle the content of each token ... */


      switch(token->type)
	{
	case EDI_TTG:
	  elem_info.d = d;
	  handle_chars(self, EDI_TAG, token, elem_info);
	  break;
	  
	case EDI_TEL:
	  elem_info.d = d;
	  handle_chars(self, EDI_ELEMENT, token, elem_info);
	  break;
	  
	case EDI_TSA:
	  event = EDI_ADVICE;
	  handle_advice(self, token);
	  break;

	case EDI_TTS:
	  edi_parser_handle_separator(self, EDI_TS, token->rdata[0]);
	  break;

	case EDI_TES:
	  if(token->first)
	    {
	      elem_info.e++;
	      elem_info.s = 0;
	    }
	  edi_parser_handle_separator(self, EDI_ES, token->rdata[0]);
	  break;
	  
	case EDI_TSS:
	  if(token->first)
	    elem_info.s++;
	  edi_parser_handle_separator(self, EDI_SS, token->rdata[0]);
	  break;
	  
	case EDI_TST:
	  edi_parser_handle_separator(self, EDI_ST, token->rdata[0]);

	  if(token->last)
	    edi_parser_handle_end (self, EDI_SEGMENT, NULL);
	  break;
	  
	default:
	  break;
	}      

      free(token);
    }
}





/** \} */





