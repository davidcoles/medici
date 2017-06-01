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
#include <stdio.h>
#include <string.h>

#include "internal.h"

void SYNTAX_init (FSAutomaton * fsa);


/*! \file token.c */

/**
   \defgroup edi_token edi_token

   \brief An EDI lexical token

   \{
*/

/**
   \brief Initialises an edi_token_s structure.
   \param self Pointer to the structure to be initialised.
*/
void
edi_token_init (edi_token_t * self)
{
  memset (self, 0, sizeof (edi_token_t));	/* mitigate bugs */
  self->type = EDI_TEL;
  self->offset = 0;
  self->csize = 0;
  self->rsize = 0;
  self->cdata[0] = '\0';
  self->rdata[0] = '\0';
  self->first = 1;
  self->last = 0;
}

/** \} */





/**
   \defgroup edi_tokeniser edi_tokeniser

   \brief The lexical analyser component of an EDI stream parser.

   edi_tokeniser breaks a stream of characters down into a stream of
   edi_token_s structs. The edi_tokeniser_s object contains state
   information and callback handlers for passing tokens to the next
   level of parsing.

   edi_tokeniser_t contains a single edi_token_s struct which is
   reused as each token is read. Long data elements will be
   represented as multiple callbacks to the token handler with first
   and last flags set on the edi_token_s as appropiate. This means
   that no memory allocation from the heap is needed at this level of
   the parser.

   \{ */

/**
   \brief Initialises an edi_tokeniser_s structure.
   \param self Pointer to the structure to be initialised.
*/
void edi_tokeniser_init (edi_tokeniser_t * self)
{
  if (!self)
    return;
  memset (self, 0, sizeof (edi_tokeniser_t));	/* mitigate bugs */
  self->offset = 0;
  self->token_process = NULL;
  self->itype_handler = NULL;
  self->token_handler = NULL;
  self->error_handler = NULL;
  edi_token_init (&(self->token));
  edi_advice_init (&(self->advice));
  self->state = 0;
  self->error = EDI_ENONE;
  self->release = 1;
  self->user_data = NULL;
  self->byte_count = 0;
  SYNTAX_init (&(self->fsa));
}


/**
   \brief Causes the current token to be passed to callback handlers.
   \param self Pointer to the edi_tokeniser_s structure.

   Hands off processing of the current token to the registered
   callback handlers and initialises the next token.
*/
int
edi_tokeniser_handle_token (edi_tokeniser_t * self, int last)
{
  int error = 0;

  self->token.last = last;

  if (self->token_handler)
    error = self->token_handler (self->user_data, &(self->token));

  /* clear data from the token and update the offset to reflect the */
  /* position in the stream of the following token */

  self->token.offset += self->token.rsize;
  self->token.rsize = self->token.csize = 0;

  if (last)
    self->token.type = EDI_TEL;

  /* If we just handled the last token in a series the next one will be */
  /* the first in a new series of tokens. */
  self->token.first = last;
  self->token.last = 0;
  self->token.is_se = 0;
  return error;
}


void
edi_tokeniser_handle_error (edi_tokeniser_t * self, edi_error_t error)
{
  self->error = error;
  if (self->error_handler)
    self->error_handler (self->user_data, error);
}



/**
   \brief Parse a string of characters.
   \param self Pointer to the edi_tokeniser_s structure.
   \param string The string of characters to be parsed.
   \param size The size of the buffer (in characters).
   \param done Set to non-zero to indicate that the stream is complete.
   \return The number of characters processed. Note that a return value
   of zero does not indicate an error - the error status of the
   tokeniser should be explicitly checked.

*/
unsigned int edi_tokeniser_parse
  (edi_tokeniser_t * self, char *string, unsigned int length, int done)
{
  edi_advice_t *advice = &(self->advice);
  char c;
  int event, status;
  unsigned int n;

  for (n = 0; n < length && !edi_tokeniser_error (self); n++)
    {
      c = string[n];
      self->byte_count++;

      /* separator/release characters can't be hardcoded as they can
         be defined at the begining of the document */

      if (edi_advice_is_ri (advice, c))
	event = SYNTAX_RI;
      else if (edi_advice_is_ss (advice, c))
	event = SYNTAX_SS;
      else if (edi_advice_is_es (advice, c))
	event = SYNTAX_ES;
      else if (edi_advice_is_ts (advice, c))
	event = SYNTAX_TS;
      else if (edi_advice_is_st (advice, c))
	event = SYNTAX_ST;
      else
	{
	  switch (c)
	    {
	    case ASCII_A:
	      event = SYNTAX_A;
	      break;
	    case ASCII_B:
	      event = SYNTAX_B;
	      break;
	    case ASCII_F:
	      event = SYNTAX_F;
	      break;
	    case ASCII_H:
	      event = SYNTAX_H;
	      break;
	    case ASCII_I:
	      event = SYNTAX_I;
	      break;
	    case ASCII_L:
	      event = SYNTAX_L;
	      break;
	    case ASCII_N:
	      event = SYNTAX_N;
	      break;
	    case ASCII_S:
	      event = SYNTAX_S;
	      break;
	    case ASCII_T:
	      event = SYNTAX_T;
	      break;
	    case ASCII_U:
	      event = SYNTAX_U;
	      break;
	    case ASCII_X:
	      event = SYNTAX_X;
	      break;
	    case ASCII_CR:
	      event = SYNTAX_CR;
	      break;
	    case ASCII_LF:
	      event = SYNTAX_LF;
	      break;
	    case ASCII_GS:
	      event = SYNTAX_IS3;
	      break;
	    default:
	      event = SYNTAX_DEFAULT;
	      break;
	    }
	}

      /* non-zero return value means "done". -1 indicates error */
      if ((status = FSAProcess (&(self->fsa), self, c, event)))
	{
	  if(status == -1)
	    {
	      edi_tokeniser_handle_error(self, EDI_ESYNTAX);
	      return 0;
	    }

	  if (self->cmplt_handler)
	    self->cmplt_handler (self->user_data);
	  
	  /* FIXME - obiwan? */
	  return n + 1;
	}

    }

  /* FIXME - obiwan? */
  return n;
}

edi_error_t edi_tokeniser_error (edi_tokeniser_t * self)
{
  return self->error;
}



int
edi_token_append (edi_token_t * t, char c, int d)
{
  t->ri[t->rsize] = !d;
  t->rdata[t->rsize++] = c;
  t->rdata[t->rsize] = '\0';
  if (d)
    {
      t->cdata[t->csize++] = c;
      t->cdata[t->csize] = '\0';
    }
  return t->rsize == (EDI_TOKEN_MAX - 1);
}


/** \} */
