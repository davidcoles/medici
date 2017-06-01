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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "frncsc.h"

static edi_directory_t *IMP(void);

#define self ((edi_imp_t *)SELF)

#define SERVICE SELF->service
#define MESSAGE SELF->message

typedef enum
{
  NONE = 0,
  STX,
  BAT,
  MHD,
  MTR,
  EOB,
  END
}
imp_code_t;

typedef struct
{
  char *string;
  imp_code_t code;
}
imp_table_t;

static imp_table_t code_table[] = {
  {"STX", STX},
  {"BAT", BAT},
  {"MHD", MHD},
  {"MTR", MTR},
  {"EOB", EOB},
  {"END", END},
  {NULL, NONE}
};


static imp_code_t
imp_get_segment_code (edi_segment_t *segment)
{
  int n;

  if (!segment)
    return NONE;
  
  for (n = 0; code_table[n].string; n++)
    if (!edi_segment_cmp_code (segment, code_table[n].string))
      return code_table[n].code;
  return NONE;
}


static void set_params
(edi_parameters_t *p, edi_directory_t *d, edi_segment_t *s, ...)
{
  int key;
  va_list ap;
  char *element, *subelement, *value;

  if(!p)
    return;
  
  /*edi_parameters_set (p, Standard, "IMP", LastParameter);*/
  edi_parameters_set (p, LastParameter);
  
  if (!d || !s)
    return;

  va_start (ap, s);
  while ((key = va_arg (ap, edi_parameter_t)))
    {
      if(key == LastParameter)
        break;

      element = va_arg (ap, char *);
      subelement = va_arg (ap, char *);
      
      value = edi_get_element_by_name (d, s, element, subelement);
      
      if (value && strlen (value))
        edi_parameters_set_one (p, (edi_parameter_t) key, value);

    }
  va_end (ap);
}











static int
imp_is_in_charset (unsigned char c)
{
  return ((c == ASCII_SPACE ||
	   c == ASCII_COLON ||
	   c == ASCII_EQUALS ||
	   c == ASCII_QUESTIONMARK ||
	   c == ASCII_CIRCUMFLEX) ||
	  (c >= ASCII_PERCENT && c <= ASCII_SOLIDUS) ||
	  (c >= ASCII_0 && c <= ASCII_9) ||
	  (c >= ASCII_a && c <= ASCII_z) ||
	  (c >= ASCII_A && c <= ASCII_Z)) ? 1 : 0;
}


static void imp_set_parameters
(edi_parser_t *SELF, edi_segment_t *segment, edi_parameters_t *parameters)
{
  switch (imp_get_segment_code (segment))
    {
      
    case STX:
      set_params (parameters, SERVICE, segment,
		  SyntaxIdentifier, "STDS", "STDS01",
		  SyntaxVersionNumber, "STDS", "STDS02",
		  SendersId, "FROM", "FROM01",
		  SendersName, "FROM", "FROM02",
		  RecipientsId, "UNTO", "UNTO01",
		  RecipientsName, "UNTO", "UNTO02",
		  Date, "TRDT", "TRDT01",
		  Time, "TRDT", "TRDT02",
		  InterchangeControlReference, "SNRF", "NULL",
		  SendersReference, "SNRF", "NULL", /* FIXME */
		  RecipientsReference, "RCRF", "NULL",
		  ApplicationReference, "APRF", "NULL",
		  ProcessingPriorityCode, "PRCD", "NULL",
		  LastParameter);
      edi_parameters_set_one (parameters, Standard, "IMP");
      break;
      
    case BAT:
      edi_parameters_set (parameters,
			  FunctionalGroupReferenceNumber,
			  edi_segment_get_element (segment, 0, 0),
			  LastParameter);
      break;
      
    case MHD:
      set_params (parameters, SERVICE, segment,
		  MessageReferenceNumber, "MSRF", "NULL",
		  MessageType, "TYPE", "TYPE01",
		  MessageVersionNumber, "TYPE", "TYPE02",
		  LastParameter);
      break;
      
    case MTR:
      set_params (parameters, SERVICE, segment,
		  NumberOfSegmentsInTheMessage, "NOSG", "NULL",
		  LastParameter);
      break;
      
    case EOB:
      edi_parameters_set (parameters,
			  NumberOfMessages,
			  edi_segment_get_element (segment, 0, 0),
			  LastParameter);
      break;
      
    case END:
      set_params (parameters, SERVICE, segment,
		  NumberOfMessages, "NMST", "NULL",
		  LastParameter);
      break;
      
    default:
      edi_parameters_set (parameters, LastParameter);
      break;
    }
}


static edi_error_t
edi_imp_segment (edi_parser_t *SELF)
{
  edi_parameters_t parameters;
  edi_segment_t *segment, *context;
  imp_code_t segment_code, context_code;
  char *str1;

  segment = SELF->segment;
  context = edi_parser_peek_segment (SELF);
  segment_code = imp_get_segment_code (segment);
  context_code = imp_get_segment_code (context);

  imp_set_parameters (SELF, segment, &parameters);

  edi_parser_handle_segment (SELF, &parameters, SERVICE);
  return EDI_ENONE;

  /* 
     if(segment_code)
     edi_parameters_set_one (&parameters, ServiceSegment, "Yes");
  */

  /*if (!EDI_isSegmentValid (MESSAGE && !segment_code ?
    MESSAGE : SERVICE,
    segment))
    EDIParser_raiseError (SELF, EDI_ESEGMENT); */

  /* FIXME - check trailer counts */

  /*printf("*** %d %s\n", segment_code, edi_segment_get_code(segment));*/

  switch (segment_code)
    {

    case STX:
      if (context_code)
	return edi_parser_raise_error (SELF, EDI_EENVELOPE);
      edi_parser_push_segment (SELF, segment);
      edi_parser_handle_start (SELF, EDI_INTERCHANGE, &parameters);
      edi_parser_handle_segment (SELF, &parameters, SERVICE);
      self->transactions = 0;
      self->groups = 0;
      break;

    case BAT:
      if (context_code != STX)
	return EDI_EENVELOPE;
      edi_parser_push_segment (SELF, segment);
      edi_parser_handle_start (SELF, EDI_GROUP, &parameters);
      edi_parser_handle_segment (SELF, &parameters, SERVICE);
      self->groups++;
      break;
      
    case MHD:
      if (context_code != STX && context_code != BAT)
	return EDI_EENVELOPE;
      edi_parser_push_segment (SELF, segment);
      edi_parser_handle_start (SELF, EDI_TRANSACTION, &parameters);
      str1 = edi_segment_get_element (segment, 1, 0);
      MESSAGE = edi_parser_handle_directory (SELF, &parameters);
      /*edi_directory_head (MESSAGE, segment, &parameters, SELF, str1);*/
      edi_parser_transaction_head(SELF, segment, MESSAGE, str1);
      self->transactions++;
      self->segments = 1;
      break;
      
    case MTR:
      if (context_code != MHD)
	return EDI_EENVELOPE;
      edi_parser_pop_segment (SELF);      
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) ||
	  atoi(str1) != ++self->segments)
        return edi_parser_raise_error (SELF, EDI_ETTC);
      /*edi_directory_tail (MESSAGE, segment, &parameters, SELF);*/
      edi_parser_transaction_tail(SELF, segment, MESSAGE);
      edi_parser_handle_end (SELF, EDI_TRANSACTION, &parameters);
      MESSAGE = NULL;
      break;
      
    case EOB:
      if (context_code != BAT)
	return EDI_EENVELOPE;
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) ||
	  atoi(str1) != self->transactions)
        return edi_parser_raise_error (SELF, EDI_EGTC);
      edi_parser_pop_segment (SELF);
      edi_parser_handle_segment (SELF, &parameters, SERVICE);
      edi_parser_handle_end (SELF, EDI_GROUP, &parameters);
      break;
      
    case END:
      if (context_code != BAT && context_code != STX)
	return EDI_EENVELOPE;
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) ||
	  atoi(str1) != self->transactions)
        return edi_parser_raise_error (SELF, EDI_EITC);
      edi_parser_pop_segment (SELF);
      edi_parser_handle_segment (SELF, &parameters, SERVICE);
      edi_parser_handle_end (SELF, EDI_INTERCHANGE, &parameters);
      SELF->done = 1;
      break;
      
    default:
      if (context_code != MHD)
	return EDI_EENVELOPE;
      /*edi_directory_body (MESSAGE, segment, &parameters, SELF);*/
      edi_parser_transaction_body(SELF, segment, MESSAGE);
      self->segments++;
    }

  return EDI_ENONE;
}


static void
imp_fini (edi_parser_t *SELF)
{
}


edi_error_t
edi_imp_init (edi_parser_t *SELF)
{
  memset(self, 0, sizeof(edi_imp_t)); /* mitigate bugs */
  
  SERVICE = IMP();
  MESSAGE = NULL;
  
  SELF->syntax_fini = imp_fini;
  SELF->sgmnt_handler = edi_imp_segment;
       
  return EDI_ENONE;
}

































































static edi_francesco_element_info_t elementinfo[] = {
  {NULL, 0, 0, 0, NULL, NULL}
};

static edi_francesco_segment_info_t segmentinfo[] = {
  {NULL, NULL, NULL}
};

static edi_francesco_segment_list_t segmentcontents[] = {
  {NULL, NULL, 0, 0}
};

static edi_francesco_cmpsite_info_t compositeinfo[] = {
  { NULL, NULL, NULL }
};

static edi_francesco_cmpsite_list_t compositecontents[] = {
  { NULL, NULL, 0 }
};

static edi_francesco_trnsctn_rule_t transactionsetrule[] = {
  {NULL, NULL, NULL, NULL, 0, 0, 0 }
};

static edi_francesco_codelst_info_t codelistinfo[] = {
  { NULL, NULL, NULL, NULL }
};

static edi_directory_t *IMP(void)
{
  return edi_francesco_create(elementinfo,
			      compositeinfo,
			      segmentinfo,
			      compositecontents,
			      segmentcontents,
			      codelistinfo,
			      transactionsetrule);
}
