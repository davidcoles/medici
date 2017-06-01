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
#include <string.h>
#include <stdio.h>

#include "internal.h"
#include "frncsc.h"

static edi_directory_t *EDIFACT_UNO(void);

/* 'SELF' is the "parent class" of 'self' */
#define self ((edi_edifact_t *) SELF)

#define MSGDIR SELF->message
#define SVCDIR SELF->service

/* FIXME - check this is correct sometime */
#define ISO9735_IS1 ASCII_US
#define ISO9735_IS2 ASCII_RS
#define ISO9735_IS3 ASCII_GS
#define ISO9735_IS4 ASCII_FS

typedef enum
{
  NONE = 0,
  UNB,
  UNG,
  UNH,
  UNT,
  UNE,
  UNZ,
  UNS,
  TXT
}
edifact_code_t;

typedef struct
{
  char *string;
  edifact_code_t code;
}
edifact_table_t;

edifact_table_t code_table[] = {
  {"UNB", UNB},
  {"UNG", UNG},
  {"UNH", UNH},
  {"UNT", UNT},
  {"UNE", UNE},
  {"UNZ", UNZ},
  {"UNS", UNS},
  {"TXT", TXT},
  {NULL, NONE}
};

static edifact_code_t
edifact_get_segment_code (edi_segment_t *segment)
{
  int n;

  if (!segment)
    return NONE;
  
  for (n = 0; code_table[n].string; n++)
    if (!edi_segment_cmp_code (segment, code_table[n].string))
      return code_table[n].code;
  return NONE;
}

static int
edifact_strcmp (char *s1, char *s2)
{
  if(!s1 && !s2)
    return 0;

  if(!s1)
    return 1;

  if(!s2)
    return -1;

  return strcmp(s1, s2);
}




/* LEVEL A CHARSET */
/* --------------- */
/* `` '' ``!'' ``"'' */
/* %&'()*+,-./0123456789:;<=>? */
/* ABCDEFGHIJKLMNOPQRSTUVWXYZ  */

/* LEVEL B CHARSET */
/* --------------- */
/* IS4 IS3 IS1 `` '' ``!'' ``"'' */
/* %&'()*+,-./0123456789:;<=>? */
/* ABCDEFGHIJKLMNOPQRSTUVWXYZ  */
/* abcdefghijklmnopqrstuvwxyz  */

static int
edifact_is_in_charset (edi_parser_t *SELF, unsigned char c)
{
  switch (self->syntax_level)
    {
    case ASCII_A:
      if (!((c >= ASCII_SPACE && c <= ASCII_QUOTE) ||
	    (c >= ASCII_PERCENT && c <= ASCII_QUESTIONMARK) ||
	    (c >= ASCII_A && c <= ASCII_Z)))
	return 0;
      break;

    case ASCII_B:
      if (!((c == ISO9735_IS4 || c == ISO9735_IS3 || c == ISO9735_IS1) ||
	    (c >= ASCII_SPACE && c <= ASCII_QUOTE) ||
	    (c >= ASCII_PERCENT && c <= ASCII_QUESTIONMARK) ||
	    (c >= ASCII_A && c <= ASCII_Z) ||
	    (c >= ASCII_a && c <= ASCII_z)))
	return 0;
      break;

      /*  C/D/E/F/... levels represent ISO 8895 character set encodings */
    }
  return 1;
}


static void set_params
(edi_parameters_t *p, edi_directory_t *d, edi_segment_t *s, ...)
{
  int key;
  va_list ap;
  char *element, *subelement, *value;

  if(!p)
    return;
  
  /*edi_parameters_set (p, Standard, "EDIFACT", LastParameter);*/
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


/* FIXME - this code could easily be generated */
static void edifact_set_parameters
(edi_parser_t *SELF, edi_segment_t *segment, edi_parameters_t *parameters)
{
  
  switch (edifact_get_segment_code (segment))
    {
    case UNB:
      set_params (parameters, SVCDIR, segment,
		  SyntaxIdentifier, "S001", "0001",
		  SyntaxVersionNumber, "S001", "0002",
		  SendersId, "S002", "0004",
		  SendersIdCodeQualifier, "S002", "0007",
		  AddressForReverseRouting, "S002", "0008",
		  RecipientsId, "S003", "0010",
		  RecipientsIdCodeQualifier, "S003", "0007",
		  RoutingAddress, "S003", "0014",
		  Date, "S004", "0017",
		  Time, "S004", "0019",
		  InterchangeControlReference, "0020", NULL,
		  RecipientsReferencePassword, "S005", "0022",
		  RecipientsReferencePasswordQualifier, "S005", "0025",
		  ApplicationReference, "0026", NULL,
		  ProcessingPriorityCode, "0029", NULL,
		  AcknowledgementRequest, "0031", NULL,
		  CommunicationsAgreementID, "0032", NULL,
		  TestIndicator, "0035", NULL,
		  LastParameter);
      edi_parameters_set_one (parameters, Standard, "EDIFACT");
      break;

    case UNG:
      set_params (parameters, SVCDIR, segment,
		  FunctionalGroupId, "0038", NULL,
		  SendersId, "S006", "0040",
		  PartnerIdCodeQualifier, "S006", "0007",
		  RecipientsId, "S007", "0044",
		  RecipientsIdCodeQualifier, "S007", "0007",
		  Date, "S004", "0017",
		  Time, "S004", "0019",
		  FunctionalGroupReferenceNumber, "0048", NULL,
		  ControllingAgency, "0051", NULL,
		  MessageVersionNumber, "S008", "0052",
		  MessageReleaseNumber, "S008", "0054",
		  AssociationAssignedCode, "S008", "0057",
		  ApplicationPassword, "0058", NULL,
		  LastParameter);
      break;

    case UNH:
      set_params (parameters, SVCDIR, segment,
		  MessageReferenceNumber, "0062", NULL,
		  MessageType, "S009", "0065",
		  MessageVersionNumber, "S009", "0052",
		  MessageReleaseNumber, "S009", "0054",
		  ControllingAgency, "S009", "0051",
		  AssociationAssignedCode, "S009", "0057",
		  CommonAccessReference, "0068", NULL,
		  SequenceOfTransfers, "S010", "0070",
		  FirstAndLastTransfer, "S010", "0073",
		  LastParameter);
      break;

    case UNE:
      set_params (parameters, SVCDIR, segment,
		  NumberOfMessages, "0060", NULL,
		  FunctionalGroupReferenceNumber, "0048", NULL,
		  LastParameter);
      break;

    case UNT:
      set_params (parameters, SVCDIR, segment,
		  NumberOfSegmentsInTheMessage, "0074", NULL,
		  MessageReferenceNumber, "0062", NULL,
		  LastParameter);
      break;

    case UNZ:
      set_params (parameters, SVCDIR, segment,
		  InterchangeControlCount, "0036", NULL,
		  InterchangeControlReference, "0020", NULL,
		  LastParameter);
      break;
      
    case UNS:
      set_params (parameters, SVCDIR, segment,
		  SectionId, "0081", NULL,
		  LastParameter);
      break;
      
    default:
      set_params (parameters, SVCDIR, segment,
		  LastParameter);
    }
}


static edi_error_t
edifact_segment (edi_parser_t *SELF)
{
  edi_parameters_t parameters;
  edi_segment_t *segment, *context;
  edifact_code_t segment_code, context_code;
  char *str1, *str2;
  
  segment = SELF->segment;
  context = edi_parser_peek_segment (SELF);
  segment_code = edifact_get_segment_code (segment);
  context_code = edifact_get_segment_code (context);
  
  edifact_set_parameters (SELF, segment, &parameters);
  
  /*
    if(segment_code)
    edi_parameters_set_one (&parameters, ServiceSegment, "Yes");
  */
  /*
    if (!EDI_isSegmentValid (segment_code ? SVCDIR : MSGDIR, segment))
    edi_parser_raise_error (SELF, EDI_ESEGMENT);
  */
  
  switch (segment_code)
    {
    case UNB:
      if (context_code)
	/*return*/ edi_parser_raise_error (SELF, EDI_EENVELOPE);
      edi_parser_push_segment (SELF, segment);
      self->message_count = 0;
      self->segment_count = 0;
      self->group_count = 0;
      /* level = "(//UNB/S001/0001)[4]", version = "//UNB/S001/0002" */
      /* FIXME - need to set character set in the lexer fsa */
      if((str1 = edi_segment_get_element (segment, 0, 0)) && strlen (str1) > 3)
	self->syntax_level = str1[3];
      else
	edi_parser_raise_error (SELF, EDI_EENVELOPE);
      if((str1 = edi_segment_get_element (segment, 0, 1)) && strlen (str1) > 0)
	self->syntax_version = str1[0];
      else
	edi_parser_raise_error (SELF, EDI_EENVELOPE);
      /* FIXME - switch() on syntax version  */
      edi_parser_handle_start (SELF, EDI_INTERCHANGE, &parameters);
      edi_parser_handle_segment (SELF, &parameters, SVCDIR);
      break;


    case UNG:
      if (context_code != UNB)
	/*return*/ edi_parser_raise_error (SELF, EDI_EENVELOPE);
      edi_parser_push_segment (SELF, segment);
      self->group_count++;
      self->message_count = 0;
      edi_parser_handle_start (SELF, EDI_GROUP, &parameters);
      edi_parser_handle_segment (SELF, &parameters, SVCDIR);
      break;


    case UNH:
      if (context_code != UNB && context_code != UNG)
	/*return*/ edi_parser_raise_error (SELF, EDI_EENVELOPE);
      /* if in a functional group, only one message type is allowed */
      if (context_code == UNG &&
	  (str1 = edi_get_element_by_name (SVCDIR, context, "0038", NULL)) &&
	  (str2 = edi_get_element_by_name (SVCDIR, segment, "S009", "0065")) &&
	  strcmp (str1, str2))
	/*return*/ edi_parser_raise_error (SELF, EDI_EGMT);
      /* if in a functional group, only one message version is allowed */
      if (context_code == UNG &&
	  (str1 = edi_get_element_by_name (SVCDIR, context, "S008", "0052")) &&
	  (str2 = edi_get_element_by_name (SVCDIR, segment, "S009", "0052")) &&
	  strcmp (str1, str2))
	/*return*/ edi_parser_raise_error (SELF, EDI_EGMV);
      edi_parser_push_segment (SELF, segment);
      self->message_count++;
      self->segment_count = 1;	/* count is inclusive of UNH (and UNT) */
      edi_parameters_set_one (&parameters, Standard, "EDIFACT");
      edi_parser_handle_start (SELF, EDI_TRANSACTION, &parameters);
      if(!(str1 = edi_segment_get_element (segment, 1, 0)))
	edi_parser_raise_error (SELF, EDI_ENOTTTH);
      MSGDIR = edi_parser_handle_directory (SELF, &parameters);
      /*edi_directory_head (MSGDIR, segment, &parameters, SELF, str1);*/
      edi_parser_transaction_head(SELF, segment, MSGDIR, str1);
      break;


    case UNT:
      self->segment_count++;
      if (context_code != UNH)
	/*return*/ edi_parser_raise_error (SELF, EDI_EENVELOPE);
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) ||
	  atoi(str1) != self->segment_count)
	edi_parser_raise_error (SELF, EDI_ETTC);
      if ((str1 = edi_segment_get_element (segment, 1, 0)) &&
	  (str2 = edi_segment_get_element (context, 0, 0)) &&
	  strcmp (str1, str2))
	edi_parser_raise_error (SELF, EDI_ETTR);
      edi_parser_pop_segment (SELF);
      /*edi_directory_tail (MSGDIR, segment, &parameters, SELF);*/
      edi_parser_transaction_tail(SELF, segment, MSGDIR);
      MSGDIR = NULL;
      edi_parser_handle_end (SELF, EDI_TRANSACTION, NULL);
      self->segment_count++;
      break;
      
      
    case UNE:
      if (context_code != UNG)
	/*return*/ edi_parser_raise_error (SELF, EDI_EENVELOPE);
      if (!self->message_count)
	edi_parser_raise_error (SELF, EDI_EGEMPTY);
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) ||
	  atoi(str1) != self->message_count)
	edi_parser_raise_error (SELF, EDI_EGTC);
      if ((str1 = edi_segment_get_element (segment, 1, 0)) &&
	  (str2 = edi_segment_get_element (context, 4, 0)) &&
	  strcmp (str1, str2))
	edi_parser_raise_error (SELF, EDI_EGTR);
      edi_parser_pop_segment (SELF);
      edi_parser_handle_segment (SELF, &parameters, SVCDIR);
      edi_parser_handle_end (SELF, EDI_GROUP, NULL);
      break;
      
      
    case UNZ:
      if (context_code != UNB)
	/*return*/ edi_parser_raise_error (SELF, EDI_EENVELOPE);
      if (!(self->group_count || self->message_count))
	edi_parser_raise_error (SELF, EDI_EIEMPTY);
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) || atoi (str1) !=
	  (self->group_count ? self->group_count : self->message_count))
	edi_parser_raise_error (SELF, EDI_EITC);
      if ((str1 = edi_segment_get_element (segment, 1, 0)) &&
	  (str2 = edi_segment_get_element (context, 4, 0)) &&
	  strcmp (str1, str2))
	edi_parser_raise_error (SELF, EDI_EITR);
      edi_parser_pop_segment (SELF);
      edi_parser_handle_segment (SELF, &parameters, SVCDIR);
      edi_parser_handle_end (SELF, EDI_INTERCHANGE, NULL);
      SELF->done = 1;
      break;


    case UNS:
      self->segment_count++;
      if (context_code != UNH)
	/*return*/ edi_parser_raise_error (SELF, EDI_EENVELOPE);      
      /*edi_directory_body (MSGDIR, segment, &parameters, SELF);*/
      edi_parser_transaction_body(SELF, segment, MSGDIR);
      break;
      
      
    default:
      self->segment_count++;
      if (context_code != UNH)
	/*return*/ edi_parser_raise_error (SELF, EDI_EENVELOPE);
      /*edi_directory_body (MSGDIR, segment, &parameters, SELF);*/
      edi_parser_transaction_body(SELF, segment, MSGDIR);
      break;
    }

  return EDI_ENONE;
}


static void
edifact_fini (edi_parser_t *SELF)
{
  if (MSGDIR)
    edi_directory_free (MSGDIR);
  MSGDIR = NULL;

  if (SVCDIR)
    edi_directory_free (SVCDIR);
  SVCDIR = NULL;
}


edi_error_t edi_edifact_init (edi_parser_t *SELF)
{
  memset(self, 0, sizeof(edi_edifact_t)); /* mitigate bugs */

  SVCDIR = EDIFACT_UNO();
  MSGDIR = NULL;

  SELF->syntax_fini = edifact_fini;
  SELF->sgmnt_handler = edifact_segment;
  
  /* initalise service segment parser */
  self->syntax_version = ASCII_2; /* FIXME */
  self->syntax_level = ASCII_A;	/* FIXME */
  
  return EDI_ENONE;
}












#define EDI_UNO_VERSION 2


static edi_francesco_element_info_t elementinfo[] = {
  { "0001", EDI_ISO2382A, 4, 4, "Syntax identifier", "a3 upper case Controlling Agency (e.g. UNO=UN/ECE) and a1 stating level (e.g. A)  (which together give UNOA)" },
  
#if EDI_UNO_VERSION == 1
  { "0002", EDI_ISO2382N, 1, 1, "Syntax version number", "Increments 1 for each new version. Shall be 1 to indicate this version" },
#else
  { "0002", EDI_ISO2382N, 1, 1, "Syntax version number", "Increments 1 for each new version. Shall be 2 to indicate this version" },
#endif

  { "0004", EDI_ISO2382X, 1, 35, "Sender identification", "Code or name as specified in IA" },
  { "0007", EDI_ISO2382X, 1,  4, "Partner identification code qualifier", "Used with sender/recipient identification code" },
  { "0008", EDI_ISO2382X, 1, 14, "Address for reverse routing", "Address for reverse routing" },
  { "0010", EDI_ISO2382X, 1, 35, "Recipient Identification", "Code or name as specified in IA" },
  { "0014", EDI_ISO2382X, 1, 14, "Routing address", "If used, normally coded sub-address for onward routing" },
  { "0017", EDI_ISO2382N, 6,  6, "Date", "YYMMDD" },
  { "0019", EDI_ISO2382N, 4,  4, "Time", "HHMM" },
  { "0020", EDI_ISO2382X, 1, 14, "INTERCHANGE CONTROL REFERENCE", "Unique reference assigned by sender" },
  { "0022", EDI_ISO2382X, 1, 14, "Recipient's reference/password", "As specified in IA. May be password to recipient's system or to third party network" },
  { "0025", EDI_ISO2382X, 2,  2, "Recipient's reference/password qualifier", "If specified in IA" },
  { "0026", EDI_ISO2382X, 1, 14, "APPLICATION REFERENCE", "Optionally message identification if the interchange contains only one type of message" },
  { "0029", EDI_ISO2382A, 1,  1, "PROCESSING PRIORITY CODE", "Used if specified in IA" },
  { "0031", EDI_ISO2382N, 1,  1, "ACKNOWLEDGEMENT REQUEST", "Set = 1 if sender requests acknowledgement, i.e. UNB and UNZ segments received and identified" },
  { "0032", EDI_ISO2382X, 1, 35, "COMMUNICATIONS AGREEMENT ID", "If used, to identify type of communication agreement controlling the interchange, e.g. Customs or ECE agreement. Code or name as specified in IA"},
  { "0035", EDI_ISO2382N, 1,  1, "TEST INDICATOR", "Set = 1 if the interchange is a test. Otherwise not used" },
  { "0036", EDI_ISO2382N, 1,  6, "INTERCHANGE CONTROL COUNT", "The count of the number of messages or, if used, the number of functional groups in the interchange. One of these counts shall appear." },
  { "0038", EDI_ISO2382X, 1,  6, "FUNCTIONAL GROUP IDENTIFICATION", "Identifies the one message type in the functional group" },
  { "0040", EDI_ISO2382X, 1, 35, "Application sender's identification", "Code or name identifying the division, department etc. within the originating sender's organization" },
  { "0044", EDI_ISO2382X, 1, 35, "Recipient's identification", "Code or name identifying the division,department etc. within the recipients organization for which the group of messages is intended" },
  { "0048", EDI_ISO2382X, 1, 14, "FUNCTIONAL GROUP REFERENCE NUMBER", "Unique reference number assigned by sender's division, department etc." },
  { "0051", EDI_ISO2382X, 1,  2, "CONTROLLING AGENCY", "Code to identify the agency controlling the specification, maintenance and publication of the message type" },
  { "0052", EDI_ISO2382X, 1,  3, "Message version number", "Version number of the message type in the functional group" },
  { "0054", EDI_ISO2382X, 1,  3, "Message release number", "Release number within current version number" },
  { "0057", EDI_ISO2382X, 1,  6, "Association assigned Code", "A code assigned by the association responsible for the design and maintenance of the type of message concerned" },
  { "0058", EDI_ISO2382X, 1, 14, "APPLICATION PASSWORD", "Password to recepient's division, department or sectional system (if required)" },
  { "0060", EDI_ISO2382N, 1,  6, "NUMBER OF MESSAGES", "The count of the number of messages in the functional group" },
  { "0062", EDI_ISO2382X, 1, 14, "MESSAGE REFERENCE NUMBER", "A sender's unique message reference" },
  { "0065", EDI_ISO2382X, 1,  6, "Message type", "Type of message being transmitted" },
  { "0068", EDI_ISO2382X, 1, 35, "COMMON ACCESS REFERENCE", "Key to relate all subsequent transfers of data to the same business case of file. Within the 35 characters the IA may specify component elements" },
  { "0070", EDI_ISO2382N, 1,  2, "Sequence of transfers", "Starts at 1 and is incremented by 1 for each transfer" },
  { "0073", EDI_ISO2382A, 1,  1, "First and last transfer", "C = Creation, must be present for first transfer if more than one foreseen, F = Final, must be present for last transfer" },
  { "0074", EDI_ISO2382N, 1,  6, "NUMBER OF SEGMENTS IN THE MESSAGE", "Control count including UNH and UNT" },
  { "0077", EDI_ISO2382X, 3,  3, "TEXT REFERENCE CODE", "Qualifies and identifies the purpose and function of the segment if indicated in the message specification" },
  { "0078", EDI_ISO2382X, 1, 70, "FREE FORM TEXT", "Not machine-processable information" },
  { "0081", EDI_ISO2382A, 1,  1, "SECTION IDENTIFICATION", "Separates sections in a message by one of the following codes: D separates the header and detail sections S separates the detail and summary sections" },
  { NULL, 0, 0, 0, NULL, NULL }
};

static edi_francesco_cmpsite_info_t compositeinfo[] = {
  { "S001", "SYNTAX IDENTIFIER", NULL},
  { "S002", "INTERCHANGE SENDER", NULL},
  { "S003", "INTERCHANGE RECIPIENT", NULL},
  { "S004", "DATE/TIME OF PREPARATION", NULL},
  { "S005", "RECIPIENTS REFERENCE", NULL},
  { "S006", "APPLICATION SENDER'S IDENTIFICATION", NULL},
  { "S007", "APPLICATION RECIPIENTS IDENTIFICATION", NULL},
  { "S008", "MESSAGE VERSION", NULL},
  { "S009", "MESSAGE IDENTIFIER", NULL},
  { "S010", "STATUS OF THE TRANSFER", NULL},
  { NULL, NULL, NULL }
};

static edi_francesco_cmpsite_list_t compositecontents[] = {
  { "S001", "0001", 'M' },
  { "S001", "0002", 'M' },
  { "S002", "0004", 'M' },
  { "S002", "0007", 'C' },
  { "S002", "0008", 'C' },
  { "S003", "0010", 'M' },
  { "S003", "0007", 'C' },
  { "S003", "0014", 'C' },
  { "S004", "0017", 'M' },
  { "S004", "0019", 'M' },
  { "S005", "0022", 'M' },
  { "S005", "0025", 'C' },
  { "S006", "0040", 'M' },
  { "S006", "0007", 'C' },
  { "S007", "0044", 'M' },
  { "S007", "0007", 'C' },
  { "S008", "0052", 'M' },

#if EDI_UNO_VERSION == 1
  { "S008", "0054", 'C' },
#else
  { "S008", "0054", 'M' },
#endif

  { "S008", "0057", 'C' },
  { "S009", "0065", 'M' },
  { "S009", "0052", 'M' },

#if EDI_UNO_VERSION == 1
  { "S009", "0054", 'C' },
  { "S009", "0051", 'C' },
#else
  { "S009", "0054", 'M' },
  { "S009", "0051", 'M' },
#endif
  { "S009", "0057", 'C' },

  { "S010", "0070", 'M' },
  { "S010", "0073", 'C' },
  { NULL, NULL, 0 }
};


static edi_francesco_segment_info_t segmentinfo[] = {
  { "UNB", "Interchange Header", "To start, identify and specify an interchange"},
  { "UNZ", "Interchange Trailer", "To end and check the completeness of an interchange"},
  { "UNG", "Functional Group Header", "To head, identify and specify a Functional Group"},
  { "UNE", "Functional Group Trailer", "To end and check the completeness of a Functional Group"},
  { "UNH", "Message Header", "To head, identify and specify a Message"},
  { "UNT", "Message Trailer", "To end and check the completeness of a Message"},
  { "TXT", "Text", "To give information in addition to that in other segments in the service message, as required"},
  { "UNS", "Section Control", "To separate Header, Detail and Summary sections of a message"},
  { NULL, NULL, NULL }
};

static edi_francesco_segment_list_t segmentcontents[] = {
  { "UNB", "S001", 'M', EDI_COMPOSITE },
  { "UNB", "S002", 'M', EDI_COMPOSITE },
  { "UNB", "S003", 'M', EDI_COMPOSITE },
  { "UNB", "S004", 'M', EDI_COMPOSITE },
  { "UNB", "0020", 'M', EDI_ELEMENT   },
  { "UNB", "S005", 'C', EDI_COMPOSITE },
  { "UNB", "0026", 'C', EDI_ELEMENT   },
  { "UNB", "0029", 'C', EDI_ELEMENT   },
  { "UNB", "0031", 'C', EDI_ELEMENT   },
  { "UNB", "0032", 'C', EDI_ELEMENT   },
  { "UNB", "0035", 'C', EDI_ELEMENT   },

  { "UNZ", "0036", 'M', EDI_ELEMENT   },
  { "UNZ", "0020", 'M', EDI_ELEMENT   },

  { "UNG", "0038", 'M', EDI_ELEMENT   },
  { "UNG", "S006", 'M', EDI_COMPOSITE },
  { "UNG", "S007", 'M', EDI_COMPOSITE },
  { "UNG", "S004", 'M', EDI_COMPOSITE },
  { "UNG", "0048", 'M', EDI_ELEMENT   },
  { "UNG", "0051", 'M', EDI_ELEMENT   },
  { "UNG", "S008", 'M', EDI_COMPOSITE },
  { "UNG", "0058", 'C', EDI_ELEMENT   },

  { "UNE", "0060", 'M', EDI_ELEMENT   },
  { "UNE", "0048", 'M', EDI_ELEMENT   },

  { "UNH", "0062", 'M', EDI_ELEMENT   },
  { "UNH", "S009", 'M', EDI_COMPOSITE },
  { "UNH", "0068", 'C', EDI_ELEMENT   },
  { "UNH", "S010", 'C', EDI_COMPOSITE },

  { "UNT", "0074", 'M', EDI_ELEMENT   },
  { "UNT", "0062", 'M', EDI_ELEMENT   },

  { "TXT", "0077", 'C', EDI_ELEMENT   },
  { "TXT", "0078", 'M', EDI_ELEMENT   },

  { "UNS", "0081", 'M', EDI_ELEMENT   },

  { NULL, NULL, 0, 0 }
};

static edi_francesco_codelst_info_t codelistinfo[] = {
  { "0001", "UNOA", "UN/ECE level A", "As defined in the basic code table of ISO 646 with the exceptions of lower case letters, alternative graphic character allocations and national or application- oriented graphic character allocations."},
  { "0001", "UNOB", "UN/ECE level B", "As defined in the basic code table of ISO 646 with the exceptions of alternative graphic character allocations and national or application-oriented graphic character allocations."},
  { "0001", "UNOC", "UN/ECE level C", "As defined in ISO 8859-1 : Information processing - Part 1: Latin alphabet No. 1."},
  { "0001", "UNOD", "UN/ECE level D", "As defined in ISO 8859-2 : Information processing - Part 2: Latin alphabet No. 2."},
  { "0001", "UNOE", "UN/ECE level E", "As defined in ISO 8859-5 : Information processing - Part 5: Latin/Cyrillic alphabet."},
  { "0001", "UNOF", "UN/ECE level F", "As defined in ISO 8859-7 : Information processing - Part 7: Latin/Greek alphabet."},
  { "0007",    "1", "DUNS (Dun + Bradstreet)", "Self explanatory."},
  { "0007",    "4", "IATA (International Air Transport Association)", "Self explanatory."},
  { "0007",    "5", "INSEE (Institut National de la Statistique et des Etudes", "Economiques) - SIRET French national statistics institute. SIRET means Systeme Informatique du Repertoire des entreprises et de leurs ETablissements."},
  { "0007",    "8", "UCC Communications ID (Uniform Code Council Communications", "Identifier) The Uniform Code Council Communications Identifier is a ten digit code used to uniquely identify physical and logical locations."},
  { "0007",    "9", "DUNS (Dun + Bradstreet) with 4 digit suffix", "Self explanatory."},
  { "0007",   "12", "Telephone number", "Self explanatory."},
  { "0007",   "14", "EAN (European Article Numbering Association)", "Self explanatory."},
  { "0007",   "18", "AIAG (Automotive Industry Action Group)", "Self explanatory."},
  { "0007",   "22", "INSEE (Institut National de la Statistique et des Etudes", "Economiques) - SIREN French national statistics institute. SIREN means Systeme Informatique du Repertoire des ENtreprises (et de leurs etablissements)."},
  { "0007",   "30", "ISO 6523: Organization identification", "Self explanatory."},
  { "0007",   "31", "DIN (Deutsches Institut fuer Normung)", "German standardization institute."},
  { "0007",   "33", "BfA (Bundesversicherungsanstalt fuer Angestellte)", "German social security association."},
  { "0007",   "34", "National Statistical Agency", "Self explanatory."},
  { "0007",   "51", "GEIS (General Electric Information Services)", "Self explanatory."},
  { "0007",   "52", "INS (IBM Network Services)", "Self explanatory."},
  { "0007",   "53", "Datenzentrale des Einzelhandels", "German data centre for retail trade."},
  { "0007",   "54", "Bundesverband der Deutschen Baustoffhaendler", "German building material trade association."},
  { "0007",   "55", "Bank identifier code", "Self explanatory."},
  { "0007",   "56", "Statens Teleforvaltning", "Norwegian telecommunications regulatory authority (NTRA)."},
  { "0007",   "57", "KTNet (Korea Trade Network Services)", "Self explanatory."},
  { "0007",   "58", "UPU (Universal Postal Union)", "Self explanatory."},
  { "0007",   "59", "ODETTE (Organization for Data Exchange through Tele-Transmission in Europe)", "European automotive industry project."},
  { "0007",   "61", "SCAC (Standard Carrier Alpha Code)", "Directory of standard multimodal carriers and tariff agent codes. The SCAC lists and codes transportation companies."},
  { "0007",   "63", "ECA (Electronic Commerce Australia)", "Australian association for electronic commerce."},
  { "0007",   "65", "TELEBOX 400 (Deutsche Telekom)", "German telecommunications service."},
  { "0007",   "80", "NHS (National Health Service)", "United Kingdom National Health Service."},
  { "0007",   "82", "Statens Teleforvaltning", "Norwegian telecommunications regulatory authority (NTRA)."},
  { "0007",   "84", "Athens Chamber of Commerce", "Greek Chamber of Commerce."},
  { "0007",   "85", "Swiss Chamber of Commerce", "Swiss Chamber of Commerce."},
  { "0007",   "86", "US Council for International Business", "United States Council for International Business."},
  { "0007",   "87", "National Federation of Chambers of Commerce and Industry", "Belgium National Federation of Chambers of Commerce and Industry."},
  { "0007",   "89", "Association of British Chambers of Commerce", "Association of British Chambers of Commerce."},
  { "0007",   "90", "SITA (Societe Internationale de Telecommunications", "Aeronautiques) SITA (Societe Internationale de Telecommunications Aeronautiques)."},
  { "0007",   "91", "Assigned by seller or seller's agent", "Self explanatory."},
  { "0007",   "92", "Assigned by buyer or buyer's agent", "Self explanatory."},
  { "0007",  "103", "TW, Trade-van", "Trade-van is an EDI VAN service center for customs, transport, and insurance in national and international trade."},
  { "0007",  "128", "BNCR (Telekurs Banken Clearing Number)", "Swiss national bank number assigned by Telekurs AG for the purpose of identifying a non-clearing banking institution."},
  { "0007",  "129", "BPI (Telekurs Business Partner Identification)", "Swiss national business number assigned by Telekurs AG for the purpose of identifying a non-clearing banking institution."},
  { "0007",  "ZZZ", "Mutually defined", "Self explanatory."},
  { "0013",  "UCD", "Data element error indication", "To identify an erroneous simple, composite or component data element, and to identify the nature of the error."},
  { "0013",  "UCF", "Functional group response", "To identify a functional group in the subject interchange and to indicate acknowledgement or rejection (action taken) of the UNG and UNE segments, and to identify any error related to these segments. Depending on the action code, it may also indicate the action taken on the messages within that functional group."},
  { "0013",  "UCI", "Interchange response", "To identify the subject interchange, to indicate interchange receipt, to indicate acknowledgement or rejection (action taken) of the UNA, UNB and UNZ segments, and to identify any error related to these segments. Depending on the action code, it may also indicate the action taken on the functional groups and messages within that interchange."},
  { "0013",  "UCM", "Message response", "To identify a message in the subject interchange, and to indicate that message's acknowledgement or rejection (action taken), and to identify any error related to the UNH and UNT segments."},
  { "0013",  "UCS", "Segment error indication", "To identify either a segment containing an error or a missing segment, and to identify any error related to the complete segment."},
  { "0013",  "UNA", "Service string advice", "To define the characters selected for use as delimiters and indicators in the rest of the interchange that follows."},
  { "0013",  "UNB", "Interchange header", "To start, identify and specify an interchange."},
  { "0013",  "UNE", "Functional group trailer", "To end and check the completeness of a functional group."},
  { "0013",  "UNG", "Functional group header", "To head, identify and specify a functional group."},
  { "0013",  "UNH", "Message header", "To head, identify and specify a message."},
  { "0013",  "UNS", "Section control", "To separate header, detail and summary sections of a message."},
  { "0013",  "UNT", "Message trailer", "To end and check the completeness of a message."},
  { "0013",  "UNZ", "Interchange trailer", "To end and check the completeness of an interchange."},
  { "0025",   "AA", "Reference", "Self explanatory."},
  { "0025",   "BB", "Password", "Self explanatory."},
  { "0029",    "A", "Highest priority", "Self explanatory."},
  { "0031",    "1", "Requested", "Acknowledgement is requested."},
  { "0035",    "1", "Interchange is a test", "Self explanatory."},
  { "0051",   "AA", "EDICONSTRUCT", "French construction project."},
  { "0051",   "AB", "DIN (Deutsches Institut fuer Normung)", "German standardization institute."},
  { "0051",   "AC", "ICS (International Chamber of Shipping)", "Self explanatory."},
  { "0051",   "AD", "UPU (Union Postale Universelle)", "Universal Postal Union."},
  { "0051",   "AE", "United Kingdom ANA (Article Numbering Association)", "Identifies the Article Numbering Association of the United Kingdom."},
  { "0051",   "AF", "ANSI ASC X12 (American National Standard Institute Accredited Standards Committee X12)", "Identifies the United States electronic data interchange standards body."},
  { "0051",   "CC", "CCC (Customs Co-operation Council)", "Self explanatory."},
  { "0051",   "CE", "CEFIC (Conseil Europeen des Federations de l'Industrie", "Chimique) EDI project for chemical industry."},
  { "0051",   "EC", "EDICON", "UK Construction project."},
  { "0051",   "ED", "EDIFICE (Electronic industries project)", "EDI Forum for companies with Interest in Computing and Electronics (EDI project for EDP/ADP sector)."},
  { "0051",   "EE", "EC + EFTA (European Communities and European Free Trade", "Association) Self explanatory."},
  { "0051",   "EN", "EAN (European Article Numbering Association)", "Self explanatory."},
  { "0051",   "ER", "UIC (International Union of railways)", "European railways."},
  { "0051",   "EU", "European Union", "Self explanatory."},
  { "0051",   "EX", "IECC (International Express Carriers Conference)", "Self explanatory."},
  { "0051",   "IA", "IATA (International Air Transport Association)", "Self explanatory."},
  { "0051",   "KE", "KEC (Korea EDIFACT Committee)", "Self explanatory."},
  { "0051",   "LI", "LIMNET", "UK Insurance project."},
  { "0051",   "OD", "ODETTE (Organization for Data Exchange through Tele-Transmission in Europe)", "European automotive industry project."},
  { "0051",   "RI", "RINET (Reinsurance and Insurance Network)", "Self explanatory."},
  { "0051",   "RT", "UN/ECE/TRADE/WP.4/GE.1/EDIFACT Rapporteurs' Teams", "Self explanatory."},
  { "0051",   "UN", "UN/ECE/TRADE/WP.4", "United Nations Economic UN Economic Commission for Europe (UN/ECE), Committee on the development of trade (TRADE), Working Party on facilitation of international trade procedures (WP.4)."},
  { "0052",    "1", "Status 1 version", "Message approved and issued as a status 1 (trial) message. (Valid for directories published after March 1990 and prior to March 1993)."},
  { "0052",    "2", "Status 2 version", "Message approved and issued as a status 2 (formal recommendation) message. (Valid for directories published after March 1990 and prior to March 1993)."},
  { "0052",   "88", "1988 version", "Message approved and issued in the 1988 release of the UNTDID (United Nations Trade Data Interchange Directory) as a status 2 (formal recommendation) message."},
  { "0052",   "89", "1989 version", "Message approved and issued in the 1989 release of the UNTDID (United Nations Trade Data Interchange Directory) as a status 2 (formal recommendation) message."},
  { "0052",   "90", "1990 version", "Message approved and issued in the 1990 release of the UNTDID (United Nations Trade Data Interchange Directory) as a status 2 (formal recommendation) message."},
  { "0052",    "D", "Draft version/UN/EDIFACT Directory", "Message approved and issued as a draft message (Valid for directories published after March 1993 and prior to March 1997). Message approved as a standard message (Valid for directories published after March 1997)."},
  { "0052",    "S", "Standard version", "Message approved and issued as a standard message. (Valid for directories published after March 1993 and prior to March 1997)."},
  { "0054",    "1", "First release", "Message approved and issued in the first release of the year of the UNTDID (United Nations Trade Data Interchange Directory). (Valid for directories published prior to March 1990)"},
  { "0054",    "2", "Second release", "Message approved and issued in the second release of the year of the UNTDID (United Nations Trade Data Interchange Directory) (Valid for directories published prior to March 1990)."},
  { "0054",  "902", "Trial release 1990", "Message approved and issued in the 1990 status 1 (trial) release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "911", "Trial release 1991", "Message approved and issued in the 1991 status 1 (trial) release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "912", "Standard release 1991", "Message approved and issued in the 1991 status 2 (standard) release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "921", "Trial release 1992", "Message approved and issued in the 1992 status 1 (trial) release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "932", "Standard release 1993", "Message approved and issued in the 1993 status 2 (standard) release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "93A", "Release 1993 - A", "Message approved and issued in the 1993 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "94A", "Release 1994 - A", "Message approved and issued in the first 1994 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "94B", "Release 1994 - B", "Message approved and issued in the second 1994 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "95A", "Release 1995 - A", "Message approved and issued in the first 1995 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "95B", "Release 1995 - B", "Message approved and issued in the second 1995 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "96A", "Release 1996 - A", "Message approved and issued in the first 1996 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "96B", "Release 1996 - B", "Message approved and issued in the second 1996 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "97A", "Release 1997 - A", "Message approved and issued in the first 1997 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "97B", "Release 1997 - B", "Message approved and issued in the second 1997 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "98A", "Release 1998 - A", "Message approved and issued in the first 1998 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "98B", "Release 1998 - B", "Message approved and issued in the second 1998 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0054",  "99A", "Release 1999 - A", "Message approved and issued in the first 1999 release of the UNTDID (United Nations Trade Data Interchange Directory)."},
  { "0065", "APERAK", "Application error and acknowledgement message", "A code to identify the application error and acknowledgement message."},
  { "0065", "AUTACK", "Secure authentication and acknowledgement message", "A code to identify the secure authentication and acknowledgement message."},
  { "0065", "AUTHOR", "Authorization message", "A code to identify the authorization message."},
  { "0065", "AVLREQ", "Availability request - interactive message", "A code to identify the availability request - interactive message."},
  { "0065", "AVLRSP", "Availability response - interactive message", "A code to identify the availability response - interactive message."},
  { "0065", "BALANC", "Balance message", "A code to identify the balance message."},
  { "0065", "BANSTA", "Banking status message", "A code to identify the banking status message."},
  { "0065", "BAPLIE", "Bayplan/stowage plan occupied and empty locations message", "A code to identify the bayplan/stowage plan occupied and empty locations message."},
  { "0065", "BAPLTE", "Bayplan/stowage plan total numbers message", "A code to identify the bayplan/stowage plan total numbers message."},
  { "0065", "BMISRM", "Bulk marine inspection summary report message", "A code to identify the bulk marine inspection summary report message."},
  { "0065", "BOPBNK", "Bank transactions and portfolio transactions report message", "A code to identify the bank transactions and portfolio transactions report message."},
  { "0065", "BOPCUS", "Balance of payment customer transaction report message", "A code to identify the balance of payment customer transaction report message."},
  { "0065", "BOPDIR", "Direct balance of payment declaration message", "A code to identify the direct balance of payment declaration message."},
  { "0065", "BOPINF", "Balance of payment information from customer message", "A code to identify the balance of payment information from customer message."},
  { "0065", "CALINF", "Vessel call information message", "A code to identify the vessel call information message."},
  { "0065", "CASINT", "Request for legal administration action in civil proceedings", "message A code to identify the request for legal administration action in civil proceedings message."},
  { "0065", "CASRES", "Legal administration response in civil proceedings message", "A code to identify the legal administration response in civil proceedings message."},
  { "0065", "CHACCO", "Chart of accounts message", "A code to identify the chart of accounts message."},
  { "0065", "CLASET", "Classification information set message", "A code to identify the classification information set message."},
  { "0065", "CNTCND", "Contractual conditions message", "A code to identify the contractual conditions message."},
  { "0065", "COARRI", "Container discharge/loading report message", "A code to identify the container discharge/loading report message."},
  { "0065", "CODECO", "Container gate-in/gate-out report message", "A code to identify the container gate-in/gate-out report message."},
  { "0065", "CODENO", "Permit expiration/clearance ready notice message", "A code to identify the permit expiration/clearance ready notice message."},
  { "0065", "COEDOR", "Container stock report message", "A code to identify the container stock report message."},
  { "0065", "COHAOR", "Container special handling order message", "A code to identify the container special handling order message."},
  { "0065", "COLREQ", "Request for a documentary collection message", "A code to identify the request for a documentary collection message."},
  { "0065", "COMDIS", "Commercial dispute message", "A code to identify the commercial dispute message."},
  { "0065", "CONAPW", "Advice on pending works message", "A code to identify the advice on pending works message."},
  { "0065", "CONDPV", "Direct payment valuation message", "A code to identify the direct payment valuation message."},
  { "0065", "CONDRA", "Drawing administration message", "A code to identify the drawing administration message."},
  { "0065", "CONDRO", "Drawing organisation message", "A code to identify the drawing organisation message."},
  { "0065", "CONEST", "Establishment of contract message", "A code to identify the establishment of contract message."},
  { "0065", "CONITT", "Invitation to tender message", "A code to identify the invitation to tender message."},
  { "0065", "CONPVA", "Payment valuation message", "A code to identify the payment valuation message."},
  { "0065", "CONQVA", "Quantity valuation message", "A code to identify the quantity valuation message."},
  { "0065", "CONRPW", "Response of pending works message", "A code to identify the response of pending works message."},
  { "0065", "CONTEN", "Tender message", "A code to identify the tender message."},
  { "0065", "CONTRL", "Syntax and service report message", "A code to identify the syntax and service report message."},
  { "0065", "CONWQD", "Work item quantity determination message", "A code to identify the work item quantity determination message."},
  { "0065", "COPARN", "Container announcement message", "A code to identify the container announcement message."},
  { "0065", "COPINO", "Container pre-notification message", "A code to identify the container pre-notification message."},
  { "0065", "COPRAR", "Container discharge/loading order message", "A code to identify the container discharge/loading order message."},
  { "0065", "COREOR", "Container release order message", "A code to identify the container release order message."},
  { "0065", "COSTCO", "Container stuffing/stripping confirmation message", "A code to identify the container stuffing/stripping confirmation message."},
  { "0065", "COSTOR", "Container stuffing/stripping order message", "A code to identify the container stuffing/stripping order message."},
  { "0065", "CREADV", "Credit advice message", "A code to identify the credit advice message."},
  { "0065", "CREEXT", "Extended credit advice message", "A code to identify the extended credit advice message."},
  { "0065", "CREMUL", "Multiple credit advice message", "A code to identify the multiple credit advice message."},
  { "0065", "CUSCAR", "Customs cargo report message", "A code to identify the customs cargo report message."},
  { "0065", "CUSDEC", "Customs declaration message", "A code to identify the customs declaration message."},
  { "0065", "CUSEXP", "Customs express consignment declaration message", "A code to identify the customs express consignment declaration message."},
  { "0065", "CUSPED", "Periodic customs declaration message", "A code to identify the periodic customs declaration message."},
  { "0065", "CUSREP", "Customs conveyance report message", "A code to identify the customs conveyance report message."},
  { "0065", "CUSRES", "Customs response message", "A code to identify the customs response message."},
  { "0065", "DEBADV", "Debit advice message", "A code to identify the debit advice message."},
  { "0065", "DEBMUL", "Multiple debit advice message", "A code to identify the multiple debit advice message."},
  { "0065", "DELFOR", "Delivery schedule message", "A code to identify the delivery schedule message."},
  { "0065", "DELJIT", "Delivery just in time message", "A code to identify the delivery just in time message."},
  { "0065", "DESADV", "Despatch advice message", "A code to identify the despatch advice message."},
  { "0065", "DESTIM", "Equipment damage and repair estimate message", "A code to identify the equipment damage and repair estimate message."},
  { "0065", "DGRECA", "Dangerous goods recapitulation message", "A code to identify the dangerous goods recapitulation message."},
  { "0065", "DIRDEB", "Direct debit message", "A code to identify the direct debit message."},
  { "0065", "DIRDEF", "Directory definition message", "A code to identify the directory definition message."},
  { "0065", "DMRDEF", "Data maintenance request definition message", "A code to identify the data maintenance request definition message."},
  { "0065", "DMSTAT", "Data maintenance status report/query message", "A code to identify the data maintenance status report/query message."},
  { "0065", "DOCADV", "Documentary credit advice message", "A code to identify the documentary credit advice message."},
  { "0065", "DOCAMA", "Advice of an amendment of a documentary credit message", "A code to identify the advice of an amendment of a documentary credit message."},
  { "0065", "DOCAMI", "Documentary credit amendment information message", "A code to identify the documentary credit amendment information message."},
  { "0065", "DOCAMR", "Request for an amendment of a documentary credit message", "A code to identify the request for an amendment of a documentary credit message."},
  { "0065", "DOCAPP", "Documentary credit application message", "A code to identify the documentary credit application message."},
  { "0065", "DOCARE", "Response to an amendment of a documentary credit message", "A code to identify the response to an amendment of a documentary credit message."},
  { "0065", "DOCINF", "Documentary credit issuance information message", "A code to identify the documentary credit issuance information message."},
  { "0065", "ENTREC", "Accounting entries message", "A code to identify the accounting entries message."},
  { "0065", "FINCAN", "Financial cancellation message", "A code to identify the financial cancellation message."},
  { "0065", "FINPAY", "Multiple interbank funds transfer message", "A code to identify the multiple interbank funds transfer message."},
  { "0065", "FINSTA", "Financial statement of an account message", "A code to identify the financial statement of an account message."},
  { "0065", "GENRAL", "General purpose message", "A code to identify the general purpose message."},
  { "0065", "GESMES", "Generic statistical message", "A code to identify the generic statistical message."},
  { "0065", "HANMOV", "Cargo/goods handling and movement message", "A code to identify the cargo/goods handling and movement message."},
  { "0065", "IFCSUM", "Forwarding and consolidation summary message", "A code to identify the forwarding and consolidation summary message."},
  { "0065", "IFTCCA", "Forwarding and transport shipment charge calculation message", "A code to identify the forwarding and transport shipment charge calculation message."},
  { "0065", "IFTDGN", "Dangerous goods notification message", "A code to identify the dangerous goods notification message."},
  { "0065", "IFTFCC", "International transport freight costs and other charges", "message A code to identify the international transport freight costs and other charges message."},
  { "0065", "IFTIAG", "Dangerous cargo list message", "A code to identify the dangerous cargo list message."},
  { "0065", "IFTMAN", "Arrival notice message", "A code to identify the arrival notice message."},
  { "0065", "IFTMBC", "Booking confirmation message", "A code to identify the booking confirmation message."},
  { "0065", "IFTMBF", "Firm booking message", "A code to identify the firm booking message."},
  { "0065", "IFTMBP", "Provisional booking message", "A code to identify the provisional booking message."},
  { "0065", "IFTMCS", "Instruction contract status message", "A code to identify the instruction contract status message."},
  { "0065", "IFTMIN", "Instruction message", "A code to identify the instruction message."},
  { "0065", "IFTRIN", "Forwarding and transport rate information message", "A code to identify the forwarding and transport rate information message."},
  { "0065", "IFTSAI", "Forwarding and transport schedule and availability", "information message A code to identify the forwarding and transport schedule and availability information message."},
  { "0065", "IFTSTA", "International multimodal status report message", "A code to identify the international multimodal status report message."},
  { "0065", "IFTSTQ", "International multimodal status request message", "A code to identify the international multimodal status request message."},
  { "0065", "IMPDEF", "EDI implementation guide definition message", "A code to identify the edi implementation guide definition message."},
  { "0065", "INFENT", "Enterprise accounting information message", "A code to identify the enterprise accounting information message."},
  { "0065", "INSDES", "Instruction to despatch message", "A code to identify the instruction to despatch message."},
  { "0065", "INSPRE", "Insurance premium message", "A code to identify the insurance premium message."},
  { "0065", "INSREQ", "Inspection request message", "A code to identify the inspection request message."},
  { "0065", "INSRPT", "Inspection report message", "A code to identify the inspection report message."},
  { "0065", "INVOIC", "Invoice message", "A code to identify the invoice message."},
  { "0065", "INVRPT", "Inventory report message", "A code to identify the inventory report message."},
  { "0065", "IPPOMO", "Motor insurance policy message", "A code to identify the motor insurance policy message."},
  { "0065", "ITRRPT", "In transit report detail message", "A code to identify the in transit report detail message."},
  { "0065", "JAPRES", "Job application result message", "A code to identify the job application result message."},
  { "0065", "JINFDE", "Job information demand message", "A code to identify the job information demand message."},
  { "0065", "JOBAPP", "Job application proposal message", "A code to identify the job application proposal message."},
  { "0065", "JOBCON", "Job order confirmation message", "A code to identify the job order confirmation message."},
  { "0065", "JOBMOD", "Job order modification message", "A code to identify the job order modification message."},
  { "0065", "JOBOFF", "Job order message", "A code to identify the job order message."},
  { "0065", "KEYMAN", "Security key and certificate management message", "A code to identify the security key and certificate management message."},
  { "0065", "LREACT", "Life reinsurance activity message", "A code to identify the life reinsurance activity message."},
  { "0065", "LRECLM", "Life reinsurance claims message", "A code to identify the life reinsurance claims message."},
  { "0065", "MEDPID", "Person identification message", "A code to identify the person identification message."},
  { "0065", "MEDREQ", "Medical service request message", "A code to identify the medical service request message."},
  { "0065", "MEDRPT", "Medical service report message", "A code to identify the medical service report message."},
  { "0065", "MEDRUC", "Medical resource usage and cost message", "A code to identify the medical resource usage and cost message."},
  { "0065", "MEQPOS", "Means of transport and equipment position message", "A code to identify the means of transport and equipment position message."},
  { "0065", "MOVINS", "Stowage instruction message", "A code to identify the stowage instruction message."},
  { "0065", "MSCONS", "Metered services consumption report message", "A code to identify the metered services consumption report message."},
  { "0065", "ORDCHG", "Purchase order change request message", "A code to identify the purchase order change request message."},
  { "0065", "ORDERS", "Purchase order message", "A code to identify the purchase order message."},
  { "0065", "ORDRSP", "Purchase order response message", "A code to identify the purchase order response message."},
  { "0065", "OSTENQ", "Order status enquiry message", "A code to identify the order status enquiry message."},
  { "0065", "OSTRPT", "Order status report message", "A code to identify the order status report message."},
  { "0065", "PARTIN", "Party information message", "A code to identify the party information message."},
  { "0065", "PAXLST", "Passenger list message", "A code to identify the passenger list message."},
  { "0065", "PAYDUC", "Payroll deductions advice message", "A code to identify the payroll deductions advice message."},
  { "0065", "PAYEXT", "Extended payment order message", "A code to identify the extended payment order message."},
  { "0065", "PAYMUL", "Multiple payment order message", "A code to identify the multiple payment order message."},
  { "0065", "PAYORD", "Payment order message", "A code to identify the payment order message."},
  { "0065", "PRICAT", "Price/sales catalogue message", "A code to identify the price/sales catalogue message."},
  { "0065", "PRIHIS", "Pricing history message", "A code to identify the pricing history message."},
  { "0065", "PRODAT", "Product data message", "A code to identify the product data message."},
  { "0065", "PRODEX", "Product exchange reconciliation message", "A code to identify the product exchange reconciliation message."},
  { "0065", "PROINQ", "Product inquiry message", "A code to identify the product inquiry message."},
  { "0065", "PROTAP", "Project tasks planning message", "A code to identify the project tasks planning message."},
  { "0065", "PRPAID", "Insurance premium payment message", "A code to identify the insurance premium payment message."},
  { "0065", "QALITY", "Quality data message", "A code to identify the quality data message."},
  { "0065", "QUOTES", "Quote message", "A code to identify the quote message."},
  { "0065", "RDRMES", "Raw data reporting message", "A code to identify the raw data reporting message."},
  { "0065", "REBORD", "Reinsurance bordereau message", "A code to identify the reinsurance bordereau message."},
  { "0065", "RECADV", "Receiving advice message", "A code to identify the receiving advice message."},
  { "0065", "RECALC", "Reinsurance calculation message", "A code to identify the reinsurance calculation message."},
  { "0065", "RECECO", "Credit risk cover message", "A code to identify the credit risk cover message."},
  { "0065", "RECLAM", "Reinsurance claims message", "A code to identify the reinsurance claims message."},
  { "0065", "REMADV", "Remittance advice message", "A code to identify the remittance advice message."},
  { "0065", "REPREM", "Reinsurance premium message", "A code to identify the reinsurance premium message."},
  { "0065", "REQDOC", "Request for document message", "A code to identify the request for document message."},
  { "0065", "REQOTE", "Request for quote message", "A code to identify the request for quote message."},
  { "0065", "RESETT", "Reinsurance settlement message", "A code to identify the reinsurance settlement message."},
  { "0065", "RESMSG", "Reservation message", "A code to identify the reservation message."},
  { "0065", "RESREQ", "Reservation request - interactive message", "A code to identify the reservation request - interactive message."},
  { "0065", "RESRSP", "Reservation response - interactive message", "A code to identify the reservation response - interactive message."},
  { "0065", "RETACC", "Reinsurance technical account message", "A code to identify the reinsurance technical account message."},
  { "0065", "RETANN", "Announcement for returns message", "A code to identify the announcement for returns message."},
  { "0065", "RETINS", "Instruction for returns message", "A code to identify the instruction for returns message."},
  { "0065", "SAFHAZ", "Safety and hazard data message", "A code to identify the safety and hazard data message."},
  { "0065", "SANCRT", "International movement of goods governmental regulatory", "message A code to identify the international movement of goods governmental regulatory message."},
  { "0065", "SLSFCT", "Sales forecast message", "A code to identify the sales forecast message."},
  { "0065", "SLSRPT", "Sales data report message", "A code to identify the sales data report message."},
  { "0065", "SOCADE", "Social administration message", "A code to identify the social administration message."},
  { "0065", "SSIMOD", "Modification of identity details message", "A code to identify the modification of identity details message."},
  { "0065", "SSRECH", "Worker's insurance history message", "A code to identify the worker's insurance history message."},
  { "0065", "SSREGW", "Notification of registration of a worker message", "A code to identify the notification of registration of a worker message."},
  { "0065", "STATAC", "Statement of account message", "A code to identify the statement of account message."},
  { "0065", "STLRPT", "Settlement transaction reporting message", "A code to identify the settlement transaction reporting message."},
  { "0065", "SUPCOT", "Superannuation contributions advice message", "A code to identify the superannuation contributions advice message."},
  { "0065", "SUPMAN", "Superannuation maintenance message", "A code to identify the superannuation maintenance message."},
  { "0065", "SUPRES", "Supplier response message", "A code to identify the supplier response message."},
  { "0065", "TANSTA", "Tank status report message", "A code to identify the tank status report message."},
  { "0065", "TIQREQ", "Travel, tourism and leisure information inquiry request -", "interactive message A code to identify the travel, tourism and leisure information inquiry request - interactive message."},
  { "0065", "TIQRSP", "Travel, tourism and leisure information inquiry response -", "interactive message A code to identify the travel, tourism and leisure information inquiry response - interactive message."},
  { "0065", "VATDEC", "Value added tax message", "A code to identify the value added tax message."},
  { "0065", "VESDEP", "Vessel departure message", "A code to identify the vessel departure message."},
  { "0065", "WASDIS", "Waste disposal information message", "A code to identify the waste disposal information message."},
  { "0065", "WKGRDC", "Work grant decision message", "A code to identify the work grant decision message."},
  { "0065", "WKGRRE", "Work grant request message", "A code to identify the work grant request message."},
  { "0073",    "C", "Creation", "First transmission of a number of transfers of the same message."},
  { "0073",    "F", "Final", "Last transmission of a number of transfers of the same message."},
  { "0081",    "D", "Header/detail section separation", "To qualify the segment UNS, when separating the header from the detail section of a message."},
  { "0081",    "S", "Detail/summary section separation", "To qualify the segment UNS, when separating the detail from the summary section of a message."},
  { "0083",    "1", "Acknowledged (this level and all lower levels)", "Self explanatory."},
  { "0083",    "2", "Acknowledged - errors detected and reported", "Self explanatory."},
  { "0083",    "3", "One or more rejected - next lower level", "Self explanatory."},
  { "0083",    "4", "This level and all lower levels rejected", "The corresponding referenced-level and all its lower referenced-levels are rejected. One or more errors are reported at this reporting-level or a lower reporting- level."},
  { "0083",    "5", "UNB/UNZ accepted", "Self explanatory."},
  { "0083",    "6", "UNB/UNZ rejected", "Self explanatory."},
  { "0083",    "7", "This level acknowledged, next lower level acknowledged if", "not explicitly rejected The corresponding referenced-level is acknowledged. All messages or functional groups at the next lower referenced-level are acknowledged except those explicitly reported as rejected at the next lower reporting-level in this CONTRL message."},
  { "0083",    "8", "Interchange received", "Indication of interchange receipt implies that the recipient of the subject interchange: has received the interchange; and acknowledges the parts of the interchange that have been checked in order to assure that the data elements copied into the reporting UCI segment are syntactically correct; and has accepted liability for notifying the sender of acknowledgement or rejection of the other parts of the interchange; and has taken reasonable precautions in order to ensure that the sender is so notified."},
  { "0085",    "1", "UNA not supported", "Notification that the UNA character string cannot be understood or complied with."},
  { "0085",    "2", "Syntax version or level not supported", "Notification that the syntax version and/or level is not supported by the recipient."},
  { "0085",    "3", "Message version/release not supported", "Notification that the message type, version number and/or release number in the UNG and/or UNH segments are not supported by the recipient."},
  { "0085",    "4", "Service segment missing/invalid", "Notification that a service segment is missing, contains invalid data or cannot be processed for any reason."},
  { "0085",    "5", "Trailer check in error", "Notification that data contained in the trailer does not agree with data in the header and/or that the functional group or message or segment count is incorrect."},
  { "0085",    "6", "Data segment missing/invalid", "Notification that a data segment is missing, contains invalid data or cannot be processed for any reason."},
  { "0085",    "7", "Interchange recipient not actual recipient", "Notification that the Interchange recipient (S003) is different from the actual recipient."},
  { "0085",    "8", "Too many data elements in segment", "Indication that a data segment contains too many data elements."},
  { "0085",    "9", "Mandatory data element missing", "Indication that a mandatory data element is missing in a service or data segment."},
  { "0085",   "10", "Data element attribute error", "Indication that a data element does not conform to the relevant message or segment specification."},
  { "0085",   "11", "Password invalid", "Indication that the password in segment UNB is invalid."},
  { "0085",   "12", "Invalid value", "Notification that the value of a simple data element, composite data element or component data element does not conform to the relevant specifications for the value."},
  { "0085",   "13", "Missing", "Notification that a mandatory (or otherwise required) service or user segment, data element, composite data element or component data element is missing"},
  { "0085",   "14", "Value not supported in this position", "Notification that the recipient does not support use of the specific value of an identified simple data element, composite data element or component data element in the position where it is used. The value may be valid according to the relevant specifications and may be supported if it is used in another position."},
  { "0085",   "15", "Not supported in this position", "Notification that the recipient does not support use of the segment type, simple data element type, composite data element type or component data element type in the specific in the identified position."},
  { "0085",   "16", "Too many constituents", "Notification that the identified segment contained to many data elements or that the identified composite data element contained too many component data elements."},
  { "0085",   "17", "No agreement", "No agreement exist that allows receipt of an interchange, functional group or message with the value of the identified simple data element, composite data element or component data element."},
  { "0085",   "18", "Unspecified error", "Notification that an error has been identified, but the nature of the error is not reported."},
  { "0085",   "19", "Invalid decimal notation", "Notification that the character indicated as decimal notation in UNA is invalid, or the decimal notation used in a data element is not consistent with the one indicated in UNA."},
  { "0085",   "20", "Character invalid as service character", "Notification that a character advised in UNA is invalid as service character."},
  { "0085",   "21", "Invalid character(s)", "Notification that one or more character(s) used in the interchange is not a valid character as defined by the syntax level indicated in UNB. The invalid character is part of the referenced-level, or followed immediately after the identified part of the interchange."},
  { "0085",   "22", "Invalid service character(s)", "Notification that the service character(s) used in the interchange is not a valid service character as advised in UNA or not one of the service characters in the syntax level indicated in UNB or defined in an interchange agreement. If the code is used in UCS or UCD, the invalid character followed immediately after the identified part of the interchange."},
  { "0085",   "23", "Unknown Interchange sender", "Notification that the Interchange sender (S002) is unknown."},
  { "0085",   "24", "Too old", "Notification that the received interchange or functional group is older than a limit specified in an IA or determined by the recipient."},
  { "0085",   "25", "Test indicator not supported", "Notification that a test processing could not be performed for the identified interchange, functional group or message."},
  { "0085",   "26", "Duplicate detected", "Notification that a possible duplication of a previously received interchange, functional group or message has been detected. The earlier transmission may have been rejected."},
  { "0085",   "27", "Security function not supported", "Notification that a security function related to the referenced-level or data element is not supported."},
  { "0085",   "28", "References do not match", "Notification that the control reference in UNB/UNG/UNH does not match the one in UNZ/UNE/UNT."},
  { "0085",   "29", "Control count does not match number of instances received", "Notification that the number of functional groups/ messages/segments does not match the number given in UNZ/UNE/UNT."},
  { "0085",   "30", "Functional groups and messages mixed", "Notification that individual messages and functional groups have been mixed at the same level in the interchange."},
  { "0085",   "31", "More than one message type in group", "Notification that different message types are contained in a functional group."},
  { "0085",   "32", "Lower level empty", "Notification that the interchange did not contain any messages or functional groups, or a functional group did not contain any messages."},
  { "0085",   "33", "Invalid occurrence outside message or functional group", "Notification that an invalid segment or data element occurred in the interchange, between messages or between functional groups. Rejection is reported at the level above."},
  { "0085",   "34", "Nesting indicator not allowed", "Notification that explicit nesting has been used in a message where it shall not be used."},
  { "0085",   "35", "Too many segment repetitions", "Notification that a segment was repeated too many times."},
  { "0085",   "36", "Too many segment group repetitions", "Notification that a segment group is repeated to many times."},
  { "0085",   "37", "Invalid type of character(s)", "Notification that one or more numeric characters were used in an alphabetic (component) data element or that one or more alphabetic characters were used in a numeric (component) data element."},
  { "0085",   "38", "Missing digit in front of decimal sign", "Notification that a decimal sign is not preceded by one or more digits."},
  { "0085",   "39", "Data element too long", "Notification that the length of the data element received exceeded the maximum length specified in the data element description."},
  { "0085",   "40", "Data element too short", "Notification that the length of the data element received is shorter than the minimum length specified in the data element description."},
  { "0085",   "41", "Permanent communication network error", "Notification that a permanent error was reported by the communication network used for transfer of the interchange. Re-transmission of an identical interchange with the same parameters at network level will not succeed."},
  { "0085",   "42", "Temporary communication network error", "Notification that a temporary error was reported by the communication network used for transfer of the interchange. Re-transmissions of an identical interchange may succeed."},
  { "0085",   "43", "Unknown interchange recipient", "Notification that the interchange recipient is not known by a network provider."},
  { "0085",   "44", "Application temporarily unavailable", "Notification that traffic for an application is inhibited."},



  { "0001", "UNOA", "UN/ECE level A", NULL},
  { "0001", "UNOB", "UN/ECE level B", NULL},
  { "0001", "UNOC", "UN/ECE level C", NULL},
  { "0001", "UNOD", "UN/ECE level D", NULL},
  { "0001", "UNOE", "UN/ECE level E", NULL},
  { "0001", "UNOF", "UN/ECE level F", NULL},
  { "0007", "1"  , "DUNS (Dun + Bradstreet)", NULL},
  { "0007", "4"  , "IATA (International Air Transport Association)", NULL},
  { "0007", "5"  , "INSEE/SIRET", NULL},
  { "0007", "8"  , "UCC Communications ID (Uniform Code Council Communications Identifier)", NULL},
  { "0007", "9"  , "DUNS with 4 digit suffix", NULL},
  { "0007", "12" , "Telephone number", NULL},
  { "0007", "14" , "EAN (European Article Numbering Association)", NULL},
  { "0007", "18" , "AIAG (Automotive Industry Action Group)", NULL},
  { "0007", "22" , "INSEE/SIREN", NULL},
  { "0007", "30" , "ISO 6523: Organization identification", NULL},
  { "0007", "31" , "DIN (Deutsches Institut fuer Normung)", NULL},
  { "0007", "33" , "BfA (Bundesversicherungsanstalt fuer Angestellte)", NULL},
  { "0007", "34" , "National Statistical Agency", NULL},
  { "0007", "51" , "General Electric Information Services", NULL},
  { "0007", "52" , "IBM Network Services", NULL},
  { "0007", "53" , "Datenzentrale des Einzelhandels, Germany", NULL},
  { "0007", "54" , "Bundesverband der Deutschen Baustoffhaendler, Germany", NULL},
  { "0007", "55" , "Bank identifier code", NULL},
  { "0007", "56" , "Statens Teleforvaltning", NULL},
  { "0007", "57" , "KTNet", NULL},
  { "0007", "58" , "UPU (Universal Postal Union)", NULL},
  { "0007", "59" , "ODETTE", NULL},
  { "0007", "61" , "SCAC (Standard Carrier Alpha Code)", NULL},
  { "0007", "63" , "ECA (Electronic Commerce Australia)", NULL},
  { "0007", "65" , "TELEBOX 400 (Deutsche Bundespost)", NULL},
  { "0007", "80" , "NHS (National Health Service)", NULL},
  { "0007", "82" , "Statens Teleforvaltning", NULL},
  { "0007", "84" , "Athens Chamber of Commerce", NULL},
  { "0007", "85" , "Swiss Chamber of Commerce", NULL},
  { "0007", "86" , "US Council for International Business", NULL},
  { "0007", "87" , "National Federation of Chambers of Commerce and Industry", NULL},
  { "0007", "89" , "Association of British Chambers of Commerce", NULL},
  { "0007", "90" , "SITA (Societe Internationale de Telecommunications Aeronautiques)", NULL},
  { "0007", "91" , "Assigned by seller or seller's agent", NULL},
  { "0007", "92" , "Assigned by buyer or buyer's agent", NULL},
  { "0007", "ZZZ", "Mutually defined", NULL},
  { "0025", "AA" , "Reference", NULL},
  { "0025", "BB" , "Password", NULL},
  { "0029", "A"  , "Highest priority", NULL},
  { "0031", "1"  , "Requested", NULL},
  { "0035", "1"  , "Interchange is a test", NULL},
  { "0051", "AA" , "EDICONSTRUCT", NULL},
  { "0051", "AB" , "DIN (Deutsches Institut fuer Normung)", NULL},
  { "0051", "AC" , "ICS (International Chamber of Shipping)", NULL},
  { "0051", "AD" , "UPU (Union Postale Universelle)", NULL},
  { "0051", "CC" , "CCC (Customs Co-operation Council)", NULL},
  { "0051", "CE" , "CEFIC (Conseil Europeen des Federations de l'Industrie Chimique)", NULL},
  { "0051", "EC" , "EDICON", NULL},
  { "0051", "ED" , "EDIFICE (Electronic industries project)", NULL},
  { "0051", "EE" , "EC + EFTA (European Communities and European Free Trade Association)", NULL},
  { "0051", "EN" , "EAN (European Article Numbering Association)", NULL},
  { "0051", "ER" , "UIC (International Union of railways)", NULL},
  { "0051", "EU" , "European Union", NULL},
  { "0051", "EX" , "IECC (International Express Carriers Conference)", NULL},
  { "0051", "IA" , "IATA (International Air Transport Association)", NULL},
  { "0051", "KE" , "KEC (Korea EDIFACT Committee)", NULL},
  { "0051", "LI" , "LIMNET", NULL},
  { "0051", "OD" , "ODETTE", NULL},
  { "0051", "RI" , "RINET (Reinsurance and Insurance Network)", NULL},
  { "0051", "RT" , "UN/ECE/TRADE/WP.4/GE.1/EDIFACT Rapporteurs' Teams", NULL},
  { "0051", "UN" , "UN/ECE/TRADE/WP.4, United Nations Standard Messages (UNSM)", NULL},
  { "0065", "APERAK", "Application error and acknowledgement message", NULL},
  { "0065", "AUTHOR", "Authorization message", NULL},
  { "0065", "BANSTA", "Banking status message", NULL},
  { "0065", "BAPLIE", "Bayplan/stowage plan occupied and empty locations message", NULL},
  { "0065", "BAPLTE", "Bayplan/stowage plan total numbers message", NULL},
  { "0065", "BOPBNK", "Bank transactions and portfolio transactions report message", NULL},
  { "0065", "BOPCUS", "Balance of payment customer transaction report message", NULL},
  { "0065", "BOPDIR", "Direct balance of payment declaration message", NULL},
  { "0065", "BOPINF", "Balance of payment information from customer message", NULL},
  { "0065", "CALINF", "Call info message", NULL},
  { "0065", "COARRI", "Container discharge/loading report message", NULL},
  { "0065", "CODECO", "Container gate-in/gate-out report message", NULL},
  { "0065", "CODENO", "Permit expiration/clearance ready notice message", NULL},
  { "0065", "COEDOR", "Container stock report message", NULL},
  { "0065", "COHAOR", "Container special handling order message", NULL},
  { "0065", "COMDIS", "Commercial dispute message", NULL},
  { "0065", "CONAPW", "Advice on pending works message", NULL},
  { "0065", "CONDPV", "Direct payment valuation message", NULL},
  { "0065", "CONDRA", "Drawing administration message", NULL},
  { "0065", "CONDRO", "Drawing organisation message", NULL},
  { "0065", "CONEST", "Establishment of contract message", NULL},
  { "0065", "CONITT", "Invitation to tender message", NULL},
  { "0065", "CONPVA", "Payment valuation message", NULL},
  { "0065", "CONQVA", "Quantity valuation message", NULL},
  { "0065", "CONRPW", "Response of pending works message", NULL},
  { "0065", "CONTEN", "Tender message", NULL},
  { "0065", "CONTRL", "Syntax and service report message", NULL},
  { "0065", "CONWQD", "Work item quantity determination message", NULL},
  { "0065", "COPARN", "Container announcement message", NULL},
  { "0065", "COPINO", "Container pre-notification message", NULL},
  { "0065", "COPRAR", "Container discharge/loading order message", NULL},
  { "0065", "COREOR", "Container release order message", NULL},
  { "0065", "COSTCO", "Container stuffing/stripping confirmation message", NULL},
  { "0065", "COSTOR", "Container stuffing/stripping order message", NULL},
  { "0065", "CREADV", "Credit advice message", NULL},
  { "0065", "CREEXT", "Extended credit advice message", NULL},
  { "0065", "CREMUL", "Multiple credit advice message", NULL},
  { "0065", "CUSCAR", "Customs cargo report message", NULL},
  { "0065", "CUSDEC", "Customs declaration message", NULL},
  { "0065", "CUSEXP", "Customs express consignment declaration message", NULL},
  { "0065", "CUSREP", "Customs conveyance report message", NULL},
  { "0065", "CUSRES", "Customs response message", NULL},
  { "0065", "DEBADV", "Debit advice message", NULL},
  { "0065", "DEBMUL", "Multiple debit advice message", NULL},
  { "0065", "DELFOR", "Delivery schedule message", NULL},
  { "0065", "DELJIT", "Delivery just-in-time message", NULL},
  { "0065", "DESADV", "Despatch advice message", NULL},
  { "0065", "DIRDEB", "Direct debit message", NULL},
  { "0065", "DIRDEF", "Directory definition message", NULL},
  { "0065", "DOCADV", "Documentary credit advice message", NULL},
  { "0065", "DOCAMA", "Advice of an amendment of a documentary credit message", NULL},
  { "0065", "DOCAMI", "Documentary credit amendment information message", NULL},
  { "0065", "DOCAMR", "Request for an amendment of a documentary credit message", NULL},
  { "0065", "DOCAPP", "Documentary credit application message", NULL},
  { "0065", "DOCARE", "Response to an amendment of a documentary credit message", NULL},
  { "0065", "DOCINF", "Documentary credit issuance information message", NULL},
  { "0065", "FINCAN", "Financial cancellation message", NULL},
  { "0065", "FINSTA", "Financial statement of an account message", NULL},
  { "0065", "GENRAL", "General purpose message", NULL},
  { "0065", "GESMES", "Generic statistical message", NULL},
  { "0065", "HANMOV", "Cargo/goods handling and movement message", NULL},
  { "0065", "IFCSUM", "International forwarding and consolidation summary message", NULL},
  { "0065", "IFTCCA", "Forwarding and transport shipment charge calculation message", NULL},
  { "0065", "IFTDGN", "Dangerous goods notification message", NULL},
  { "0065", "IFTFCC", "International transport freight costs and other charges message", NULL},
  { "0065", "IFTIAG", "Dangerous cargo list message", NULL},
  { "0065", "IFTMAN", "Arrival notice message", NULL},
  { "0065", "IFTMBC", "Booking confirmation message", NULL},
  { "0065", "IFTMBF", "Firm booking message", NULL},
  { "0065", "IFTMBP", "Provisional booking message", NULL},
  { "0065", "IFTMCS", "Instruction contract status message", NULL},
  { "0065", "IFTMIN", "Instruction message", NULL},
  { "0065", "IFTRIN", "Forwarding and transport rate information message", NULL},
  { "0065", "IFTSAI", "Forwarding and transport schedule and availability information message", NULL},
  { "0065", "IFTSTA", "International multimodal status report message", NULL},
  { "0065", "IFTSTQ", "International multimodal status request message", NULL},
  { "0065", "INSPRE", "Insurance premium message", NULL},
  { "0065", "INVOIC", "Invoice message", NULL},
  { "0065", "INVRPT", "Inventory report message", NULL},
  { "0065", "JAPRES", "Job application result message", NULL},
  { "0065", "JINFDE", "Job information demand message", NULL},
  { "0065", "JOBAPP", "Job application proposal message", NULL},
  { "0065", "JOBCON", "Job order confirmation message", NULL},
  { "0065", "JOBMOD", "Job order modification message", NULL},
  { "0065", "JOBOFF", "Job order message", NULL},
  { "0065", "MEDPID", "Person identification message", NULL},
  { "0065", "MOVINS", "Stowage instruction message", NULL},
  { "0065", "ORDCHG", "Purchase order change message", NULL},
  { "0065", "ORDERS", "Purchase order message", NULL},
  { "0065", "ORDRSP", "Purchase order response message", NULL},
  { "0065", "PARTIN", "Party information message", NULL},
  { "0065", "PAXLST", "Passenger list message", NULL},
  { "0065", "PAYDUC", "Payroll deductions advice message", NULL},
  { "0065", "PAYEXT", "Extended payment order message", NULL},
  { "0065", "PAYMUL", "Multiple payment order message", NULL},
  { "0065", "PAYORD", "Payment order message", NULL},
  { "0065", "PRICAT", "Price/sales catalogue message", NULL},
  { "0065", "PRODEX", "Product exchange reconciliation message", NULL},
  { "0065", "PRPAID", "Insurance premium payment message", NULL},
  { "0065", "QALITY", "Quality data message", NULL},
  { "0065", "QUOTES", "Quote message", NULL},
  { "0065", "RDRMES", "Raw data reporting message", NULL},
  { "0065", "RECADV", "Receiving advice message", NULL},
  { "0065", "RECECO", "Credit risk cover message", NULL},
  { "0065", "RECLAM", "Reinsurance claims message", NULL},
  { "0065", "REMADV", "Remittance advice message", NULL},
  { "0065", "REQDOC", "Request for document message", NULL},
  { "0065", "REQOTE", "Request for quote message", NULL},
  { "0065", "RESETT", "Reinsurance settlement message", NULL},
  { "0065", "RESMSG", "Reservation message", NULL},
  { "0065", "RETACC", "Reinsurance technical account message", NULL},
  { "0065", "SAFHAZ", "Safety and hazard data message", NULL},
  { "0065", "SANCRT", "Sanitary/phytosanitary certificate message", NULL},
  { "0065", "SLSFCT", "Sales forecast message", NULL},
  { "0065", "SLSRPT", "Sales data report message", NULL},
  { "0065", "SSIMOD", "Modification of identity details message", NULL},
  { "0065", "SSRECH", "Worker's insurance history message", NULL},
  { "0065", "SSREGW", "Notification of registration of a worker message", NULL},
  { "0065", "STATAC", "Statement of account message", NULL},
  { "0065", "SUPCOT", "Superannuation contributions advice message", NULL},
  { "0065", "SUPMAN", "Superannuation maintenance message", NULL},
  { "0065", "SUPRES", "Supplier response message", NULL},
  { "0065", "TANSTA", "Tank status report message", NULL},
  { "0065", "VESDEP", "Vessel departure message", NULL},
  { "0065", "WKGRDC", "Work grant decision message", NULL},
  { "0065", "WKGRRE", "Work grant request message", NULL},
  { "0073", "C"  , "Creation", NULL},
  { "0073", "F"  , "Final", NULL},
  { "0081", "D"  , "Header/detail section separation", NULL},
  { "0081", "S"  , "Detail/summary section separation", NULL},
  {NULL, NULL, NULL, NULL }
};

static edi_francesco_trnsctn_rule_t transactionsetrule[] = {
  {NULL, NULL, NULL, NULL, 0, 0, 0 }
};

static edi_directory_t *EDIFACT_UNO (void)
{
  return edi_francesco_create(elementinfo,
			      compositeinfo,
			      segmentinfo,
			      compositecontents,
			      segmentcontents,
			      codelistinfo,
			      transactionsetrule);
}
