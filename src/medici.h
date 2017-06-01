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

#ifndef MEDICI_H
#define MEDICI_H

#ifdef __cplusplus
extern "C" {
#endif
  
#include "common.h"
#include "prmtrs.h"
  
#define EDI_SetTextHandler EDI_SetCharacterHandler

  typedef void *EDI_Directory;
  typedef void *EDI_TSG;
  typedef void *EDI_Parameters;
  typedef void *EDI_Parser;
  typedef void *EDI_Segment;
  typedef void *EDI_Token;
  
  typedef edi_event_t EDI_Event;
  typedef edi_pragma_t EDI_Pragma;
  typedef edi_parameter_t EDI_Parameter;
  typedef edi_data_type_t EDI_DataType;
  
  typedef void (*EDI_SeparatorHandler) (void *, edi_event_t, char);
  typedef void (*EDI_ErrorHandler) (void *, int);
  typedef void (*EDI_TokenHandler) (void *, EDI_Token);
  typedef void (*EDI_CharacterHandler) (void *, const char *, int);
  typedef void (*EDI_StartHandler) (void *, edi_event_t, EDI_Parameters);
  typedef void (*EDI_EndHandler) (void *, edi_event_t);
  typedef void (*EDI_SegmentHandler) (void *, EDI_Parameters,
				      EDI_Segment, EDI_Directory);
  
  typedef EDI_Directory (*EDI_DirectoryHandler) (void *, EDI_Parameters);
  
  /* obsolete */
  
  /* medici.c */
  EDI_Parser EDI_ParserCreate(void);
  void EDI_ParserReset(EDI_Parser);
  EDI_Pragma EDI_SetPragma(EDI_Parser, EDI_Pragma);
  EDI_StartHandler EDI_SetStartHandler(EDI_Parser, EDI_StartHandler);
  EDI_EndHandler EDI_SetEndHandler(EDI_Parser, EDI_EndHandler);
  EDI_ErrorHandler EDI_SetErrorHandler(EDI_Parser, EDI_ErrorHandler);
  EDI_ErrorHandler EDI_SetWarningHandler(EDI_Parser, EDI_ErrorHandler);
  /*EDI_TokenHandler EDI_SetTokenHandler(EDI_Parser, EDI_TokenHandler);*/
  EDI_DirectoryHandler EDI_SetDirectoryHandler(EDI_Parser, EDI_DirectoryHandler);
  EDI_SegmentHandler EDI_SetSegmentHandler(EDI_Parser, EDI_SegmentHandler);
  EDI_CharacterHandler EDI_SetCharacterHandler(EDI_Parser, EDI_CharacterHandler);
  EDI_CharacterHandler EDI_SetDefaultHandler(EDI_Parser, EDI_CharacterHandler);
  EDI_SeparatorHandler EDI_SetSeparatorHandler(EDI_Parser, EDI_SeparatorHandler);
  void *EDI_SetUserData(EDI_Parser, void *);
  long EDI_Parse(EDI_Parser, char *, long, int);
  int EDI_GetErrorCode(EDI_Parser);
  char *EDI_GetErrorString(int);
  char *EDI_GetEventString(EDI_Event);
  void EDI_ParserFree(EDI_Parser);
  unsigned long EDI_GetCurrentSegmentIndex(EDI_Parser);
  char *EDI_GetParameter(EDI_Parameters, EDI_Parameter);
  char *EDI_GetCode(EDI_Segment);
  int EDI_GetElementCount(EDI_Segment);
  int EDI_GetSubelementCount(EDI_Segment, int);
  char *EDI_GetElement(EDI_Segment, int, int);
  unsigned long EDI_GetCurrentByteIndex(EDI_Parser);
  char *EDI_GetParameterString(EDI_Parameter);
  char *EDI_GetElementByName(EDI_Directory, EDI_Segment, char *);
  int EDI_EvaluateString(edi_data_type_t, char *, edi_data_type_t, void *);
  EDI_Parameters EDI_GetParserInfo(EDI_Parser);
  char *EDI_MEDICIVersion(void);
  int EDI_InterchangeComplete(EDI_Parser);
  char *EDI_ElementName(EDI_Directory, char *);
  char *EDI_ElementDesc(EDI_Directory, char *);
  char *EDI_ElementNote(EDI_Directory, char *);
  edi_item_t EDI_ElementRepresentation(EDI_Directory, char *);
  char *EDI_SegmentName(EDI_Directory, char *);
  char *EDI_SegmentDesc(EDI_Directory, char *);
  char *EDI_SegmentNote(EDI_Directory, char *);
  char *EDI_CompositeName(EDI_Directory, char *);
  char *EDI_CompositeDesc(EDI_Directory, char *);
  char *EDI_CompositeNote(EDI_Directory, char *);
  int EDI_isComposite(EDI_Directory, char *, char *);
  char EDI_SegmentReqr(EDI_Directory, char *, char *);
  char EDI_CompositeReqr(EDI_Directory, char *, char *);
  char *EDI_CodelistName(EDI_Directory, char *, char *);
  char *EDI_CodelistDesc(EDI_Directory, char *, char *);
  char *EDI_CodelistNote(EDI_Directory, char *, char *);
  int EDI_ElementIndex(EDI_Directory, char *, int *, int *);
  char *EDI_GetCodelistValue(EDI_Directory, char *, char *);
  void EDI_DirectoryFree(EDI_Directory);
  EDI_Directory EDI_GetServiceDirectory(EDI_Parser);
  edi_item_t EDI_SegmentItem(EDI_Directory, char *, unsigned int);
  edi_item_t EDI_CompositeItem(EDI_Directory, char *, unsigned int);

#ifdef __cplusplus
}
#endif

#endif /*MEDICI_H*/

