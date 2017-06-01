/**********************************************************************
 *
 * The Expyx PYX Parser Library
 * Copyright (C) 2007, David Coles.  All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software. If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * David Coles
 * david@disintegration.org
 *
 **********************************************************************/

/**********************************************************************
 * This software (neccessarily) includes an implementation of the
 * Expat API. I would like to make it abundantly, explicitly clear
 * that I am not claiming to have written any of that - the Expat
 * function names, architecture, enumerations all belong to the Expat
 * guys.
 **********************************************************************/



#ifndef EXPYX_H
#define EXPYX_H

#ifdef __cplusplus
extern "C" {
#endif

#define PYXCALL
#define PYX_Char char
#define PYX_LChar char

  enum PYX_Error {
    PYX_ERROR_NONE = 0
  };

  enum PYX_Status {
    PYX_STATUS_ERROR = 0,
    PYX_STATUS_OK = 1
  };

  enum PYX_Parsing {
    PYX_INITIALIZED,
    PYX_PARSING,
    PYX_FINISHED,
    PYX_SUSPENDED
  };

  typedef unsigned char PYX_Bool;
#define PYX_TRUE ((PYX_Bool) 1)
#define PYX_FALSE ((PYX_Bool) 0)



  typedef void * PYX_Parser;
    
  typedef struct {
    enum PYX_Parsing parsing;
    PYX_Bool finalBuffer;
  } PYX_ParsingStatus;
  
  typedef struct {
    void *(PYXCALL *malloc_fcn)(size_t size);
    void *(PYXCALL *realloc_fcn)(void *ptr, size_t size);
    void (PYXCALL *free_fcn)(void *ptr);
  } PYX_Memory_Handling_Suite;
  

  typedef void
  (PYXCALL *PYX_StartElementHandler)(void *userData,
				     const PYX_Char *name,
				     const PYX_Char **atts);
  
  typedef void
  (PYXCALL *PYX_EndElementHandler)(void *userData,
				   const PYX_Char *name);
  
  typedef void
  (PYXCALL *PYX_CharacterDataHandler)(void *userData,
				      const PYX_Char *s,
				      int len);

  typedef void
  (PYXCALL *PYX_ProcessingInstructionHandler)(void *userData,
					      const PYX_Char *target,
					      const PYX_Char *data);

  typedef void
  (PYXCALL *PYX_CommentHandler)(void *userData,
				const PYX_Char *data);


  
  void PYX_SetUserData(void *, void *);





  void PYX_SetStartElementHandler(PYX_Parser, PYX_StartElementHandler);
  void PYX_SetEndElementHandler(PYX_Parser, PYX_EndElementHandler);
  void PYX_SetElementHandler(PYX_Parser, PYX_StartElementHandler, PYX_EndElementHandler);
  void PYX_SetCharacterDataHandler(PYX_Parser, PYX_CharacterDataHandler);
  void PYX_SetProcessingInstructionHandler(PYX_Parser, PYX_ProcessingInstructionHandler);
  void PYX_SetCommentHandler(PYX_Parser, PYX_CommentHandler);





  PYX_Parser PYXCALL PYX_ParserCreate(const char *);
  PYX_Parser PYXCALL PYX_ParserCreateNS(const char *, char);
  PYX_Parser PYXCALL PYX_ParserCreate_MM(const char *, const PYX_Memory_Handling_Suite *, const char *);
  PYX_Parser PYXCALL PYX_ExternalEntityParserCreate(PYX_Parser, const char *, const char *);
  void PYXCALL PYX_ParserFree(PYX_Parser);
  PYX_Bool PYXCALL PYX_ParserReset(PYX_Parser, const char *);




  enum PYX_Status PYXCALL PYX_Parse(PYX_Parser, const char *, int, int);
  enum PYX_Status PYXCALL PYX_ParseBuffer(PYX_Parser, int, int);
  void * PYXCALL PYX_GetBuffer(PYX_Parser, int);
  



  void * PYXCALL PYX_MemMalloc(PYX_Parser, size_t);
  void * PYXCALL PYX_MemRealloc(PYX_Parser, void *, size_t);
  void PYXCALL PYX_MemFree(PYX_Parser, void *);





  void PYXCALL PYX_GetParsingStatus(PYX_Parser, PYX_ParsingStatus *);
  enum PYX_Error PYXCALL PYX_GetErrorCode(PYX_Parser);
  const char * PYXCALL PYX_ErrorString(enum PYX_Error);
  long PYXCALL PYX_GetCurrentByteIndex(PYX_Parser);
  int PYXCALL PYX_GetCurrentLineNumber(PYX_Parser);
  int PYXCALL PYX_GetCurrentColumnNumber(PYX_Parser);
  int PYXCALL PYX_GetCurrentByteCount(PYX_Parser);
  const char * PYXCALL PYX_GetInputContext(PYX_Parser, int *, int *);





























  


#ifdef EXPYX_COMPAT

#define XMLCALL PYXCALL
#define XML_Char PYX_Char
#define XML_LChar PYX_LChar
#define XML_TRUE PYX_TRUE
#define XML_FALSE PYX_FALSE
#define XML_Error PYX_Error

#define XML_Status PYX_Status

#define XML_Parsing PYX_Parsing
#define XML_INITIALIZED PYX_INITIALIZED
#define XML_PARSING PYX_PARSING
#define XML_FINISHED PYX_FINISHED
#define XML_SUSPENDED PYX_SUSPENDED
#define XML_ParsingStatus PYX_ParsingStatus







#define XML_STATUS_ERROR PYX_STATUS_ERROR
#define XML_STATUS_OK PYX_STATUS_OK

#define XML_Bool PYX_Bool
#define XML_Parser PYX_Parser

#define XML_StartElementHandler PYX_StartElementHandler
#define XML_EndElementHandler PYX_EndElementHandler
#define XML_CharacterDataHandler PYX_CharacterDataHandler
#define XML_ProcessingInstructionHandler PYX_ProcessingInstructionHandler
#define XML_CommentHandler PYX_CommentHandler




#define XML_SetUserData PYX_SetUserData
#define XML_SetStartElementHandler PYX_SetStartElementHandler
#define XML_SetEndElementHandler PYX_SetEndElementHandler
#define XML_SetElementHandler PYX_SetElementHandler
#define XML_SetCharacterDataHandler PYX_SetCharacterDataHandler
#define XML_SetProcessingInstructionHandler PYX_SetProcessingInstructionHandler
#define XML_SetCommentHandler PYX_SetCommentHandler


#define XML_ParserCreate PYX_ParserCreate
#define XML_ParserCreateNS PYX_ParserCreateNS
#define XML_ParserCreate_MM PYX_ParserCreate_MM
#define XML_ExternalEntityParserCreate PYX_ExternalEntityParserCreate
#define XML_ParserFree PYX_ParserFree
#define XML_ParserReset PYX_ParserReset

#define XML_Parse PYX_Parse








#define XML_GetParsingStatus PYX_GetParsingStatus
#define XML_GetErrorCode PYX_GetErrorCode
#define XML_ErrorString PYX_ErrorString
#define XML_GetCurrentByteIndex PYX_GetCurrentByteIndex
#define XML_GetCurrentLineNumber PYX_GetCurrentLineNumber
#define XML_GetCurrentColumnNumber PYX_GetCurrentColumnNumber
#define XML_GetCurrentByteCount PYX_GetCurrentByteCount
#define XML_GetInputContext PYX_GetInputContext






#endif /* EXPYX_COMPAT */


#ifdef __cplusplus
}
#endif

#endif /* EXPYX_H */
