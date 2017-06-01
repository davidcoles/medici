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

static edi_directory_t *UNGTDI_1_ANA(void);

#define self ((edi_ungtdi_t *)SELF)

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
ungtdi_code_t;

typedef struct
{
  char *string;
  ungtdi_code_t code;
}
ungtdi_table_t;

static ungtdi_table_t code_table[] = {
  {"STX", STX},
  {"BAT", BAT},
  {"MHD", MHD},
  {"MTR", MTR},
  {"EOB", EOB},
  {"END", END},
  {NULL, NONE}
};


static ungtdi_code_t
ungtdi_get_segment_code (edi_segment_t *segment)
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
  
  /*edi_parameters_set (p, Standard, "UNGTDI", LastParameter);*/
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
ungtdi_is_in_charset (unsigned char c)
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


static void ungtdi_set_parameters
(edi_parser_t *SELF, edi_segment_t *segment, edi_parameters_t *parameters)
{
  switch (ungtdi_get_segment_code (segment))
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
      edi_parameters_set_one (parameters, Standard, "UNGTDI");
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
edi_ungtdi_segment (edi_parser_t *SELF)
{
  edi_parameters_t parameters;
  edi_segment_t *segment, *context;
  ungtdi_code_t segment_code, context_code;
  char *str1;

  segment = SELF->segment;
  context = edi_parser_peek_segment (SELF);
  segment_code = ungtdi_get_segment_code (segment);
  context_code = ungtdi_get_segment_code (context);

  ungtdi_set_parameters (SELF, segment, &parameters);
  
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
ungtdi_fini (edi_parser_t *SELF)
{
  if (MESSAGE)
    edi_directory_free (MESSAGE);
  MESSAGE = NULL;

  if (SERVICE)
      edi_directory_free (SERVICE);
  SERVICE = NULL;
}


edi_error_t
edi_ungtdi_init (edi_parser_t *SELF)
{
  memset(self, 0, sizeof(edi_ungtdi_t)); /* mitigate bugs */
  
  SERVICE = UNGTDI_1_ANA();
  MESSAGE = NULL;
  
  SELF->syntax_fini = ungtdi_fini;
  SELF->sgmnt_handler = edi_ungtdi_segment;
       
  return EDI_ENONE;
}

































































static edi_francesco_element_info_t elementinfo[] = {
  { "STDS01", EDI_ISO2382X,  1,  4, "Syntax Rules Identifier", NULL },
  { "STDS02", EDI_ISO2382X,  1,  4, "Syntax Rules Version", NULL },
  { "FROM01", EDI_ISO2382X,  1, 14, "Transmission Sender Code", NULL },
  { "FROM02", EDI_ISO2382X,  1, 35, "Transmission Sender Name", NULL },
  { "UNTO01", EDI_ISO2382X,  1, 14, "Transmission Recipient Code", NULL },
  { "UNTO02", EDI_ISO2382X,  1, 35, "Transmission Recipient Name", NULL },
  { "TRDT01", EDI_ISO2382N,  1,  6, "Transmission Date", NULL },
  { "TRDT02", EDI_ISO2382N,  1,  6, "Transmission Time", NULL },
  { "SNRF"  , EDI_ISO2382X,  1, 14, "Sender's Transmission Reference", NULL },
  { "RCRF"  , EDI_ISO2382X,  1, 14, "Recipient's Transmission Reference", NULL },
  { "APRF"  , EDI_ISO2382X,  1, 14, "Application Reference", NULL },
  { "PRCD"  , EDI_ISO2382X,  1,  1, "Transmission Priority Code", NULL },
  { "MSRF"  , EDI_ISO2382N,  1, 12, "Message Reference", NULL },
  { "TYPE01", EDI_ISO2382X,  1,  6, "Message Type", NULL },
  { "TYPE02", EDI_ISO2382N,  1,  1, "Message Version Number", NULL },
  { "NOSG"  , EDI_ISO2382N,  1, 10, "Number of Segments in Message", NULL },
  { "NMST"  , EDI_ISO2382N,  1,  5, "Number of Messages in Transmission", NULL },
  {NULL, 0, 0, 0, NULL, NULL}
};

static edi_francesco_segment_info_t segmentinfo[] = {
  { "STX", "START OF TRANSMISSION", NULL },
  { "MHD", "MESSAGE HEADER", NULL },
  { "MTR", "MESSAGE TRAILER", NULL },
  { "END", "END OF TRANSMISSION", NULL },
  {NULL, NULL, NULL}
};

static edi_francesco_segment_list_t segmentcontents[] = {
  { "STX", "STDS" , 'M', EDI_COMPOSITE },
  { "STX", "FROM" , 'M', EDI_COMPOSITE },
  { "STX", "UNTO" , 'M', EDI_COMPOSITE },
  { "STX", "TRDT" , 'M', EDI_COMPOSITE },
  { "STX", "SNRF" , 'M', EDI_ELEMENT   },
  { "STX", "RCRF" , 'C', EDI_ELEMENT   },
  { "STX", "APRF" , 'C', EDI_ELEMENT   },
  { "STX", "PRCD" , 'C', EDI_ELEMENT   },
  { "MHD", "MSRF" , 'M', EDI_ELEMENT   },
  { "MHD", "TYPE" , 'M', EDI_COMPOSITE },
  { "MTR", "NOSG" , 'M', EDI_ELEMENT   },
  { "END", "NMST" , 'M', EDI_ELEMENT   },
  {NULL, NULL, 0, 0}
};

static edi_francesco_cmpsite_info_t compositeinfo[] = {
  { "STDS", "Syntax Rules Identifier", NULL },
  { "FROM", "Identification of Transmission Sender", NULL },
  { "UNTO", "Identification of Transmission Recipient", NULL },
  { "TYPE", "Type of Message", NULL },
  { "TRDT", "Date and Time of Transmission", NULL },
  { NULL, NULL, NULL }
};

static edi_francesco_cmpsite_list_t compositecontents[] = {
  { "STDS", "STDS01", 'M' },
  { "STDS", "STDS02", 'M' },
  { "FROM", "FROM01", 'C' },
  { "FROM", "FROM02", 'C' },
  { "UNTO", "UNTO01", 'C' },
  { "UNTO", "UNTO02", 'C' },
  { "TRDT", "TRDT01", 'M' },
  { "TRDT", "TRDT02", 'C' },
  { "TYPE", "TYPE01", 'M' },
  { "TYPE", "TYPE02", 'M' },
  { NULL, NULL, 0 }
};

static edi_francesco_trnsctn_rule_t transactionsetrule[] = {
  {NULL, NULL, NULL, NULL, 0, 0, 0 }
};

static edi_francesco_codelst_info_t codelistinfo[] = {
  { "APRF", "ACKHDR", "Acknowledgement of Order", NULL },
  { "APRF", "AVLHDR", "Availability Report", NULL },
  { "APRF", "BTOHDR", "Book Order File", NULL },
  { "APRF", "CAKHDR", "Claims Acknowledgement File", NULL },
  { "APRF", "CLAHDR", "Claims File", NULL },
  { "APRF", "CORHDR", "Complex Order", NULL },
  { "APRF", "CRAHDR", "Credit Advice File", NULL },
  { "APRF", "CREHDR", "Credit Note File", NULL },
  { "APRF", "CUSHDR", "Customer Information File", NULL },
  { "APRF", "DELHDR", "Delivery Notification File", NULL },
  { "APRF", "DLCHDR", "Delivery Confirmation File", NULL },
  { "APRF", "DRAHDR", "Debit Advice File", NULL },
  { "APRF", "DYEHDR", "Dye Instruction File", NULL },
  { "APRF", "EFTHDR", "Electronic Funds Transaction File", NULL },
  { "APRF", "EXCHDR", "Exception Condition File", NULL },
  { "APRF", "GENHDR", "General Communications File", NULL },
  { "APRF", "HOTHDR", "Hot Card File", NULL },
  { "APRF", "HSOHDR", "Home shopping Order File", NULL },
  { "APRF", "INTHDR", "Interchange Acknowledgement File", NULL },
  { "APRF", "INVFIL", "Invoice File", NULL },
  { "APRF", "ISSUES", "Issues File", NULL },
  { "APRF", "LPRHDR", "Location Planning Report file", NULL },
  { "APRF", "ORDHDR", "Order File", NULL },
  { "APRF", "PAYHDR", "Payment Order File", NULL },
  { "APRF", "PICHDR", "Picking Instructions File", NULL },
  { "APRF", "PPRHDR", "Product Planning Report File", NULL },
  { "APRF", "PRIHDR", "Price Information File", NULL },
  { "APRF", "PROHDR", "Product Information File", NULL },
  { "APRF", "PVUHDR", "Price and Availability Updates File", NULL },
  { "APRF", "SADHDR", "Stock Adjustment File", NULL },
  { "APRF", "SNPHDR", "Stock Snapshot File", NULL },
  { "APRF", "SORHDR", "Supply and Return File", NULL },
  { "APRF", "SRMHDR", "Statement/Remittance Details File", NULL },
  { "APRF", "UCNHDR", "Uplift Confirmation File", NULL },
  { "APRF", "UPLHDR", "Uplift Instruction File", NULL },
  { "APRF", "UTLHDR", "Utility Bill File", NULL },

  { "TYPE01", "ACKHDR", "Acknowledgement of Order", NULL },
  { "TYPE01", "AVLHDR", "Availability Report", NULL },
  { "TYPE01", "BTOHDR", "Book Order File", NULL },
  { "TYPE01", "CAKHDR", "Claims Acknowledgement File", NULL },
  { "TYPE01", "CLAHDR", "Claims File", NULL },
  { "TYPE01", "CORHDR", "Complex Order", NULL },
  { "TYPE01", "CRAHDR", "Credit Advice File", NULL },
  { "TYPE01", "CREHDR", "Credit Note File", NULL },
  { "TYPE01", "CUSHDR", "Customer Information File", NULL },
  { "TYPE01", "DELHDR", "Delivery Notification File", NULL },
  { "TYPE01", "DLCHDR", "Delivery Confirmation File", NULL },
  { "TYPE01", "DRAHDR", "Debit Advice File", NULL },
  { "TYPE01", "DYEHDR", "Dye Instruction File", NULL },
  { "TYPE01", "EFTHDR", "Electronic Funds Transaction File", NULL },
  { "TYPE01", "EXCHDR", "Exception Condition File", NULL },
  { "TYPE01", "GENHDR", "General Communications File", NULL },
  { "TYPE01", "HOTHDR", "Hot Card File", NULL },
  { "TYPE01", "HSOHDR", "Home shopping Order File", NULL },
  { "TYPE01", "INTHDR", "Interchange Acknowledgement File", NULL },
  { "TYPE01", "INVFIL", "Invoice File", NULL },
  { "TYPE01", "ISSUES", "Issues File", NULL },
  { "TYPE01", "LPRHDR", "Location Planning Report file", NULL },
  { "TYPE01", "ORDHDR", "Order File", NULL },
  { "TYPE01", "PAYHDR", "Payment Order File", NULL },
  { "TYPE01", "PICHDR", "Picking Instructions File", NULL },
  { "TYPE01", "PPRHDR", "Product Planning Report File", NULL },
  { "TYPE01", "PRIHDR", "Price Information File", NULL },
  { "TYPE01", "PROHDR", "Product Information File", NULL },
  { "TYPE01", "PVUHDR", "Price and Availability Updates File", NULL },
  { "TYPE01", "SADHDR", "Stock Adjustment File", NULL },
  { "TYPE01", "SNPHDR", "Stock Snapshot File", NULL },
  { "TYPE01", "SORHDR", "Supply and Return File", NULL },
  { "TYPE01", "SRMHDR", "Statement/Remittance Details File", NULL },
  { "TYPE01", "UCNHDR", "Uplift Confirmation File", NULL },
  { "TYPE01", "UPLHDR", "Uplift Instruction File", NULL },
  { "TYPE01", "UTLHDR", "Utility Bill File", NULL },

  { "PRCD", "A"   , "Urgent", NULL },
  { "PRCD", "B"   , "Normal", NULL },
  { "PRCD", "C"   , "Low", NULL },

  { NULL, NULL, NULL, NULL }
};

static edi_directory_t *UNGTDI_1_ANA(void)
{
  return edi_francesco_create(elementinfo,
			      compositeinfo,
			      segmentinfo,
			      compositecontents,
			      segmentcontents,
			      codelistinfo,
			      transactionsetrule);
}
