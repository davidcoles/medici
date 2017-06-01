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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "internal.h"
#include "medici.h"

/** \file medici.c

    \brief External C API

*/

/**
   \defgroup medici medici
   \brief External C API

   These are the "official" C API functions that client applications
   should use. Pointers to structures are opaque (cast to void *).

   \{
*/



/**
   \brief Construct a new parser.
   \return Opaque pointer to the new parser object.
*/
EDI_Parser EDI_ParserCreate (void)
{
  return edi_parser_create (EDI_UNKNOWN);
}

void EDI_ParserReset (EDI_Parser p)
{
  edi_parser_reset ((edi_parser_t *) p);
}

EDI_Pragma
EDI_SetPragma (EDI_Parser p, EDI_Pragma pragma)
{
  EDI_Pragma old = ((edi_parser_t *) p)->pragma;
  ((edi_parser_t *) p)->pragma = pragma;
  return (old);
}

/**
   \brief Set handler for start of structural elements.
   \return Pointer to previously set handler.

   The type of "element" is indicated by the edi_event_t enumeration,
   and "attributes" are contained in the EDI_Parameters object.

*/
EDI_StartHandler
EDI_SetStartHandler (EDI_Parser p, EDI_StartHandler h)
{
  return (EDI_StartHandler)
    edi_parser_set_start_handler((edi_parser_t *) p, (edi_start_handler_t) h);
}

/**
   \brief Set handler for end of structural elements.
 */
EDI_EndHandler
EDI_SetEndHandler (EDI_Parser p, EDI_EndHandler h)
{
  return (EDI_EndHandler)
    edi_parser_set_end_handler((edi_parser_t *) p, (edi_end_handler_t) h);
}

EDI_ErrorHandler
EDI_SetErrorHandler (EDI_Parser p, EDI_ErrorHandler h)
{
  return (EDI_ErrorHandler)
    edi_parser_set_error_handler((edi_parser_t *) p,
				 (edi_error_handler_t) h);
}

EDI_ErrorHandler
EDI_SetWarningHandler (EDI_Parser p, EDI_ErrorHandler h)
{
  return (EDI_ErrorHandler)
    edi_parser_set_warning_handler((edi_parser_t *) p,
				   (edi_error_handler_t) h);
}

/*
EDI_TokenHandler
EDI_SetTokenHandler (EDI_Parser p, EDI_TokenHandler h)
{
  return (EDI_TokenHandler)
    edi_parser_set_token_handler((edi_parser_t *) p,
				 (edi_token_handler_t) h);
}
*/

EDI_DirectoryHandler
EDI_SetDirectoryHandler (EDI_Parser p, EDI_DirectoryHandler h)
{
  return (EDI_DirectoryHandler)
    edi_parser_set_directory_handler((edi_parser_t *) p,
				     (edi_directory_handler_t) h);
}

EDI_SegmentHandler
EDI_SetSegmentHandler (EDI_Parser p, EDI_SegmentHandler h)
{
  return (EDI_SegmentHandler)
    edi_parser_set_segment_handler((edi_parser_t *) p,
				   (edi_segment_handler_t) h);
}

EDI_CharacterHandler
EDI_SetCharacterHandler (EDI_Parser p, EDI_CharacterHandler h)
{
  return (EDI_CharacterHandler)
    edi_parser_set_text_handler((edi_parser_t *) p,
				(edi_character_handler_t) h);
}

EDI_CharacterHandler
EDI_SetDefaultHandler (EDI_Parser p, EDI_CharacterHandler h)
{
  return (EDI_CharacterHandler)
    edi_parser_set_default_handler((edi_parser_t *) p,
				   (edi_character_handler_t) h);
}

EDI_SeparatorHandler
EDI_SetSeparatorHandler (EDI_Parser p, EDI_SeparatorHandler h)
{
  return (EDI_SeparatorHandler)
    edi_parser_set_separator_handler((edi_parser_t *) p,
				     (edi_separator_handler_t) h);
}

void *EDI_SetUserData (EDI_Parser p, void *v)
{
  return edi_parser_set_user_data((edi_parser_t *) p, v);
}

long
EDI_Parse (EDI_Parser p, char *buffer, long length, int done)
{
  return edi_parser_parse((edi_parser_t *) p, buffer, length, done);
}

int
EDI_GetErrorCode (EDI_Parser p)
{
  return edi_parser_get_error_code ((edi_parser_t *) p);
}

char *
EDI_GetErrorString (int error)
{
  return edi_error_string ((edi_error_t) error);
}


char *
EDI_GetEventString (EDI_Event event)
{
  return edi_event_string (event);
}

void
EDI_ParserFree (EDI_Parser p)
{
  edi_parser_free ((edi_parser_t *) p);
}


unsigned long
EDI_GetCurrentSegmentIndex (EDI_Parser p)
{
  return edi_parser_get_segment_index((edi_parser_t *) p);
}

char *
EDI_GetParameter (EDI_Parameters ps, EDI_Parameter p)
{
  return (char *)
    (ps ? edi_parameters_get ((edi_parameters_t *) ps, p) : NULL);
}

char *
EDI_GetCode (EDI_Segment s)
{
  return edi_segment_get_code ((edi_segment_t *) s);
}

int
EDI_GetElementCount (EDI_Segment s)
{
  return edi_segment_get_element_count ((edi_segment_t *) s);
}

int
EDI_GetSubelementCount (EDI_Segment s, int e)
{
  return edi_segment_get_subelement_count((edi_segment_t *) s, e);
}


char *
EDI_GetElement (EDI_Segment s, int x, int y)
{
  return edi_segment_get_element ((edi_segment_t *) s, x, y);
}

unsigned long EDI_GetCurrentByteIndex (EDI_Parser p)
{
  return edi_parser_get_byte_index ((edi_parser_t *) p);
}

char *EDI_GetParameterString (EDI_Parameter p)
{
  return (char *) edi_parameters_get_string(p);
}

char *
EDI_GetElementByName (EDI_Directory d, EDI_Segment s, char *element)
{
  int x = 0, y = 0;
  char key[256], *value = NULL;

  if(!d || !s || !element)
    return value;
  
  sprintf (key, "%s/%s", edi_segment_get_code ((edi_segment_t *) s), element);
  
  value = edi_directory_element_index ((edi_directory_t *) d, key, &x, &y) ?
    edi_segment_get_element ((edi_segment_t *) s, x, y) : NULL;
  
  return value;
}

int EDI_EvaluateString
(edi_data_type_t criteria, char *string, edi_data_type_t as, void *value)
{
  edi_evaluate_element (criteria, string, as, value);
  return 1;
}

EDI_Parameters
EDI_GetParserInfo (EDI_Parser p)
{
  return edi_parser_info ((edi_parser_t *) p);
}

char *EDI_MEDICIVersion (void)
{
  return EDI_MEDICI_VERSION;
}

int EDI_InterchangeComplete (EDI_Parser p)
{
  return edi_parser_is_complete((edi_parser_t *) p);
}
















char *
EDI_ElementName (EDI_Directory d, char *ref)
{
  return edi_directory_element_name ((edi_directory_t *) d, ref);
}

char *
EDI_ElementDesc (EDI_Directory d, char *ref)
{
  return edi_directory_element_desc ((edi_directory_t *) d, ref);
}

char *
EDI_ElementNote (EDI_Directory d, char *ref)
{
  return edi_directory_element_note ((edi_directory_t *) d, ref);
}

edi_item_t
EDI_ElementRepresentation (EDI_Directory d, char *ref)
{
  return edi_directory_element_repr ((edi_directory_t *) d, ref); 
}

char *
EDI_SegmentName (EDI_Directory d, char *ref)
{
  return edi_directory_segment_name ((edi_directory_t *) d, ref);
}

char *
EDI_SegmentDesc (EDI_Directory d, char *ref)
{
  return edi_directory_segment_desc ((edi_directory_t *) d, ref);
}

char *
EDI_SegmentNote (EDI_Directory d, char *ref)
{
  return edi_directory_segment_note ((edi_directory_t *) d, ref);
}

char *
EDI_CompositeName (EDI_Directory d, char *ref)
{
  return edi_directory_composite_name ((edi_directory_t *) d, ref);
}

char *
EDI_CompositeDesc (EDI_Directory d, char *ref)
{
  return edi_directory_composite_desc ((edi_directory_t *) d, ref);
}

char *
EDI_CompositeNote (EDI_Directory d, char *ref)
{
  return edi_directory_composite_note ((edi_directory_t *) d, ref);
}

int
EDI_isComposite (EDI_Directory d, char *t, char *ref)
{
  return edi_directory_is_composite ((edi_directory_t *) d, t, ref);
}

char
EDI_SegmentReqr (EDI_Directory d, char *t, char *ref)
{
  return edi_directory_segment_reqr ((edi_directory_t *) d, t, ref);
}

char
EDI_CompositeReqr (EDI_Directory d, char *t, char *ref)
{
  return edi_directory_composite_reqr ((edi_directory_t *) d, t, ref);
}

char *
EDI_CodelistName (EDI_Directory d, char *c, char *v)
{
  return edi_directory_codelist_name ((edi_directory_t *) d, c, v);
}

char *
EDI_CodelistDesc (EDI_Directory d, char *c, char *v)
{
  return edi_directory_codelist_desc ((edi_directory_t *) d, c, v);
}

char *
EDI_CodelistNote (EDI_Directory d, char *c, char *v)
{
  return edi_directory_codelist_note ((edi_directory_t *) d, c, v);
}


int
EDI_ElementIndex (EDI_Directory d, char *key, int *x, int *y)
{
  return edi_directory_element_index ((edi_directory_t *) d, key, x, y);
}

char *
EDI_GetCodelistValue (EDI_Directory d, char *element, char *value)
{
  return edi_directory_codelist_value ((edi_directory_t *) d, element, value);
}




void EDI_DirectoryFree(EDI_Directory d)
{
  edi_directory_free((edi_directory_t *) d);
}


EDI_Directory
EDI_GetServiceDirectory(EDI_Parser p)
{
  return edi_parser_service((edi_parser_t *)  p);
}

edi_item_t
EDI_SegmentItem (EDI_Directory d, char *code, unsigned int n)
{
  return edi_directory_segment_item((edi_directory_t *) d, code, n);
}

edi_item_t
EDI_CompositeItem (EDI_Directory d, char *code, unsigned int n)
{
  return edi_directory_composite_item((edi_directory_t *) d, code, n);
}


/** \} */
