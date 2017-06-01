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
 *
 * The function prototypes and comments describing the behaviour of
 * the API are taken from the documentation included with Expat
 * entitled "The Expat XML Parser - Release 1.95.8". I tried to
 * contact Clark Cooper (and James Clark) at the addresses listed in
 * the Expat source, but received rejections from both MTAs. Hopefully
 * there will be not problem with the prototypes and the documentation
 * is mainly there so that I can see at a glance how the functions
 * should behave. If anyone does have any problems with this then
 * please get in touch ASAP.
 *
 **********************************************************************/

/**********************************************************************
 *
 * From <http://xmlstar.sourceforge.net/>:
 *
 * "The PYX format is a line-oriented representation of XML documents
 * that is derived from the SGML ESIS format.  (see ESIS - ISO 8879
 * Element Structure Information Set spec, ISO/IEC JTC1/SC18/WG8 N931
 * (ESIS))"
 *
 * The goal of Expyx is to provide a drop-in replacement for the Expat
 * parser which will parse PYX files rather than XML. The expected
 * user would be on a platform where Expat is not available/ported.
 *
 * The user would simply change the #include <expat.h> line to
 * #include "expyx.h" and #define EXPYX_COMPAT (before the #include)
 * to make all XML_ prefixed Expat calls refer to the Expyx PYX_
 * functions instead. Link the object file into your application and
 * you should be able to use PYX files in place of XML files.
 *
 * If Expat and Expyx are to be mixed in an application then don't
 * define the EXPYX_COMPAT symbol to leave the two APIs separate.
 *
 * You can produce PYX files from XML files with the XMLStarlet
 * toolkit (http://xmlstar.sourceforge.net/).
 * 
 **********************************************************************/

/**********************************************************************
 *
 * WARNING: Quick and dirty PYX parser implementation. This code is
 * all pretty rough and ready. Buffer overflows could occur on
 * pathological PYX files. It is expected that the PYX file to be
 * parsed is trusted and unlikely to cause overflows. You can check
 * the length of lines in the PYX file with `wc -L file.pxy`.
 * 
 * No well-formed-ness checking is done on the PYX file (yet). You
 * should check that the source XML file is fit for service before
 * converting to PYX.
 *
 * PYX_BUFFSIZE should be large enough to contain the longest line and
 * PYX_ATTRSIZE should be larger than the maximum number of attributes
 * in the file. With 8192/64 an instance of the PYX parser will occupy
 * around one megabyte of memory. Static buffers will eventually be
 * replaced.
 *
 **********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "expyx.h"

#define PYX_BUFFSIZE 8192
#define PYX_ATTRSIZE 64

#define APPEND_BUFFER(c) \
if(p->offset < (PYX_BUFFSIZE - 1)) { \
  p->buffer[p->offset++] = c; \
  p->buffer[p->offset] = '\0'; \
}


PYX_Memory_Handling_Suite mhs = {malloc, realloc, free};


typedef enum {
  PYX_SOL = 0,
  PYX_KEY,
  PYX_VAL
} state_t;

typedef enum {
  PYX_NONE = 0,
  PYX_COMMENT,
  PYX_PI,
  PYX_DECL,
  PYX_DATA,
  PYX_ATTR,
  PYX_START,
  PYX_END,

  PYX_UNKNOWN
} line_t;


typedef struct
{
  int offset;
  char buffer[PYX_BUFFSIZE];
  char target[PYX_BUFFSIZE];
  char element[PYX_BUFFSIZE];


  line_t line;
  state_t state;
  int escape;

  int nlines;
  int nattrs;

  char key[PYX_ATTRSIZE][PYX_BUFFSIZE];
  char val[PYX_ATTRSIZE][PYX_BUFFSIZE];

  void *user_data;

  PYX_StartElementHandler start_element_handler;
  PYX_EndElementHandler end_element_handler;
  PYX_CharacterDataHandler character_data_handler;
  PYX_ProcessingInstructionHandler processing_instruction_handler;
  PYX_CommentHandler comment_handler;

  const PYX_Memory_Handling_Suite *ms;
  char *get_buffer;


  /* old */
  char *attr[PYX_ATTRSIZE * 2];
  int todo;

} pyxparser_t;



static void pyx_handle_start_element (pyxparser_t *p)
{
  char *attr[PYX_ATTRSIZE * 2];
  int n;

  for(n = 0; n < p->nattrs; n++)
    {
      attr[(n * 2) + 0] = p->key[n];
      attr[(n * 2) + 1] = p->val[n];
    }

  attr[(n * 2) + 0] = NULL;
  
  if(p->start_element_handler)
    p->start_element_handler(p->user_data, p->element, (const char **) &attr);
}

static void pyx_handle_end_element (pyxparser_t *p)
{
  if(p->end_element_handler)
    p->end_element_handler(p->user_data, p->element);
}

static void pyx_handle_character_data (pyxparser_t *p)
{
  if(p->character_data_handler)
    p->character_data_handler(p->user_data, p->buffer, p->offset);
}


static void pyx_handle_processing_instruction(pyxparser_t *p)
{
  if(p->processing_instruction_handler)
    p->processing_instruction_handler(p->user_data, p->target, p->buffer);
}

static void pyx_handle_comment(pyxparser_t *p)
{
  if(p->comment_handler)
    p->comment_handler(p->user_data, p->buffer);
}


static void pyx_char (pyxparser_t *p, char c)
{
  
  /* EOL */
  if(c == '\n' && p->state != PYX_SOL)
    {
      switch(p->line)
	{
	case PYX_START:
	  strncpy(p->element, p->buffer, PYX_BUFFSIZE);
	  break;
	  
	case PYX_END:
	  strncpy(p->element, p->buffer, PYX_BUFFSIZE);
	  pyx_handle_end_element(p);
	  break;
	  
	case PYX_ATTR:
	  strncpy(p->val[p->nattrs], p->buffer, PYX_BUFFSIZE);
	  if(p->nattrs < (PYX_ATTRSIZE - 1))
	    p->nattrs++;
	  break;
	  
	case PYX_DATA:
	  pyx_handle_character_data(p);
	  break;
	  
	case PYX_PI:
	  pyx_handle_processing_instruction(p);
	  break;
	  
	case PYX_COMMENT:
	  pyx_handle_comment(p);
	  break;
	  
	default:
	  break;
	}
      
      p->buffer[0] = '\0';
      p->offset = 0;
      p->state = PYX_SOL;
      goto done_with_char;
    }
  
  /* SOL */
  if(p->state == PYX_SOL)
    {
      p->buffer[0] = '\0';
      p->offset = 0;
      
      switch(p->line)
	{
	case PYX_START:
	case PYX_ATTR:
	  if(c != 'A')
	    pyx_handle_start_element(p);
	  break;
	  
	default:
	  break;
	}
      
      switch(c)
	{
	case '(':
	  p->line = PYX_START;
	  p->element[0] = '\0';
	  p->nattrs = 0;
	  break;
	case ')':
	  p->line = PYX_END;
	  break;
	case 'A':
	  p->line = PYX_ATTR;
	  break;
	case '-':
	  p->line = PYX_DATA;
	  break;
	case 'C':
	  p->line = PYX_COMMENT;
	  break;
	case 'D':
	  p->line = PYX_DECL;
	  break;
	case '?':
	  p->line = PYX_PI;
	  break;
	default:
	  p->line = PYX_UNKNOWN;
	  break;
	}
      
      p->state = PYX_KEY;
      goto done_with_char;
    }
  
  
  /* deal with escaped characters - newlines, etc. */
  if(p->escape == 1)
    {
      p->escape = 0;
      switch(c)
	{
	case 'n':
	  c = '\n';
	  break;
	}
    }
  else if(c == '\\')
    {
      p->escape = 1;
      goto done_with_char;
    }
  
  
  switch (p->line)
    {	  
    case PYX_UNKNOWN:
      break;
      
    case PYX_ATTR:
      if(p->state == PYX_KEY && c == ' ')
	{
	  strncpy(p->key[p->nattrs], p->buffer, PYX_BUFFSIZE);
	  p->buffer[0] = '\0';
	  p->offset = 0;
	  p->state = PYX_VAL;
	}
      else 
	{
	  APPEND_BUFFER(c);
	}
      break;
      
    case PYX_PI:
      if(p->state == PYX_KEY && c == ' ')
	{
	  strncpy(p->target, p->buffer, PYX_BUFFSIZE);
	  p->buffer[0] = '\0';
	  p->offset = 0;
	  p->state = PYX_VAL;
	}
      else 
	{
	  APPEND_BUFFER(c);
	}
      break;
      
    case PYX_COMMENT:
    case PYX_END:
    case PYX_DATA:
    case PYX_START:
      APPEND_BUFFER(c);
      break;
      
    default:
      break;
    }

 done_with_char:
  return;
}

static int
pyx_parse_new (pyxparser_t *p, const char *buffer, int length, int done)
{
  int n;
  
  for(n = 0; n < length; n++)
    pyx_char(p, buffer[n]);
  
  if(done && buffer[n] != '\n')
      pyx_char(p, '\n');

  return length;
}



void  pyx_free_mallocs (pyxparser_t *p)
{

  if(p->get_buffer)
    PYX_MemFree(p, p->get_buffer);
}




























static void pyx_line(pyxparser_t *p, char *buffer)
{
  char *ptr = buffer;
  int n = 0;

  for(ptr = buffer; *ptr; ptr++)
    if(*ptr == '\n')
      *ptr = '\0';
  
  switch(buffer[0])
    {
    case '(':
      if(p->todo)
        {
          if(p->start_element_handler)
            p->start_element_handler(p->user_data,
				     p->element, (const char **) p->attr);
          p->todo = 0;
        }
      strcpy(p->element, buffer + 1);
      p->todo = 1;
      break;
      
    case ')':
      if(p->todo)
        {
          if(p->start_element_handler)
            p->start_element_handler(p->user_data,
				     p->element, (const char **) p->attr);
          p->todo = 0;
        }
      if(p->end_element_handler)
        p->end_element_handler(p->user_data, buffer + 1);
      for(n = 0; n < p->nattrs; n++)
        {
          free(p->attr[n]);
          p->attr[n] = NULL;
        }
      p->nattrs = 0;
      break;
      
    case 'A':
      for(n = 0; buffer[n]; n++)
        if(buffer[n] == ' ')
          break;
      buffer[n] = '\0';
      ptr = malloc(n + 1);
      strcpy(ptr, buffer + 1);
      p->attr[p->nattrs++] = ptr;
      ptr = malloc(strlen(buffer + n + 1) + 1);
      strcpy(ptr, buffer + n + 1);
      p->attr[p->nattrs++] = ptr;
      p->attr[p->nattrs] = NULL;
      break;

    default:
      /* Ignore any other lines - I'm not using any character data */
      /* in the simple tsg format for now */
      break;
    }
}

static int
pyx_parse_old (pyxparser_t *p, const char *buffer, int length, int done)
{
  int n;

  for(n = 0; n < length; n++)
    {
      p->buffer[p->offset++] = buffer[n];
      if(buffer[n] == '\n')
        {
          pyx_line(p, p->buffer);
          p->offset = 0;
        }
    }
  return length;
}






























































































void PYX_SetUserData(void *p, void *u)
{
  ((pyxparser_t *) p)->user_data = u;
}

void PYXCALL
PYX_SetStartElementHandler(PYX_Parser p,
                           PYX_StartElementHandler start)
{
  ((pyxparser_t *) p)->start_element_handler = start;
}

void PYXCALL
PYX_SetEndElementHandler(PYX_Parser p,
                         PYX_EndElementHandler end)
{
  ((pyxparser_t *) p)->end_element_handler = end;
}


void PYXCALL PYX_SetElementHandler(PYX_Parser p,
			PYX_StartElementHandler start,
			PYX_EndElementHandler end)
{
  ((pyxparser_t *) p)->start_element_handler = start;
  ((pyxparser_t *) p)->end_element_handler = end;
}

void PYXCALL
PYX_SetCharacterDataHandler(PYX_Parser p,
                            PYX_CharacterDataHandler charhndl)
{
  
  ((pyxparser_t *) p)->character_data_handler = charhndl;
}

void PYXCALL
PYX_SetProcessingInstructionHandler(PYX_Parser p,
                                    PYX_ProcessingInstructionHandler proc)
{
  ((pyxparser_t *) p)->processing_instruction_handler = proc;
}


void PYXCALL
PYX_SetCommentHandler(PYX_Parser p, PYX_CommentHandler cmnt)
{
  ((pyxparser_t *) p)->comment_handler = cmnt;
}


















/**********************************************************************
 * Allocate size bytes of memory using the allocator the parser object
 * has been configured to use. Returns a pointer to the memory or NULL
 * on failure. Memory allocated in this way must be freed using
 * PYX_MemFree.
 **********************************************************************/

void * PYXCALL
PYX_MemMalloc(PYX_Parser parser, size_t size)
{
  pyxparser_t *p = parser;
  return p->ms->malloc_fcn(size);
}


/**********************************************************************
 * Allocate size bytes of memory using the allocator the parser object
 * has been configured to use. ptr must point to a block of memory
 * allocated by PYX_MemMalloc or PYX_MemRealloc, or be NULL. This
 * function tries to expand the block pointed to by ptr if possible.
 * Returns a pointer to the memory or NULL on failure. On success, the
 * original block has either been expanded or freed. On failure, the
 * original block has not been freed; the caller is responsible for
 * freeing the original block. Memory allocated in this way must be
 * freed using PYX_MemFree.
 **********************************************************************/

void * PYXCALL
PYX_MemRealloc(PYX_Parser parser, void *ptr, size_t size)
{
  pyxparser_t *p = parser;
  return p->ms->realloc_fcn(ptr, size);
}

/**********************************************************************
 * Free a block of memory pointed to by ptr. The block must have been
 * allocated by PYX_MemMalloc or PYX_MemRealloc, or be NULL.
 **********************************************************************/

void PYXCALL
PYX_MemFree(PYX_Parser parser, void *ptr)
{
  pyxparser_t *p = parser;
  p->ms->free_fcn(ptr);
}






























/**********************************************************************
 * Parser Creation Functions
 **********************************************************************/


/**********************************************************************
   Construct a new parser. If encoding is non-null, it specifies a
   character encoding to use for the document. This overrides the
     document encoding declaration. There are four built-in encodings:
     * US-ASCII
     * UTF-8
     * UTF-16
     * ISO-8859-1
   Any other value will invoke a call to the UnknownEncodingHandler.
**********************************************************************/

PYX_Parser PYXCALL
PYX_ParserCreate(const PYX_Char *encoding)
{
  return PYX_ParserCreate_MM(encoding, &mhs, NULL);
}

/**********************************************************************
 * Constructs a new parser that has namespace processing in effect.
 * Namespace expanded element names and attribute names are returned
 * as a concatenation of the namespace URI, sep, and the local part of
 * the name. This means that you should pick a character for sep that
 * can't be part of a legal URI.
**********************************************************************/

PYX_Parser PYXCALL
PYX_ParserCreateNS(const PYX_Char *encoding, PYX_Char sep)
{
  PYX_Char s[2];
  s[0] = sep;
  s[1] = '\0';
  return PYX_ParserCreate_MM(encoding, NULL, s);
}


/**********************************************************************
 * Construct a new parser using the suite of memory handling functions
 * specified in ms. If ms is NULL, then use the standard set of memory
 * management functions. If sep is non NULL, then namespace processing
 * is enabled in the created parser and the character pointed at by
 * sep is used as the separator between the namespace URI and the
 * local part of the name.
 **********************************************************************/
PYX_Parser PYXCALL
PYX_ParserCreate_MM(const PYX_Char *encoding,
                    const PYX_Memory_Handling_Suite *ms,
                    const PYX_Char *sep)
{
  pyxparser_t *p;

  if((p = ms->malloc_fcn(sizeof(pyxparser_t))))
    {
      memset(p, 0, sizeof(pyxparser_t));
      p->state = 0;
      p->ms = ms;
    }
  
  return (PYX_Parser) p;
}



/**********************************************************************
 * Construct a new PYX_Parser object for parsing an external general
 * entity. Context is the context argument passed in a call to a
 * ExternalEntityRefHandler. Other state information such as handlers,
 * user data, namespace processing is inherited from the parser passed
 * as the 1st argument. So you shouldn't need to call any of the
 * behavior changing functions on this parser (unless you want it to
 * act differently than the parent parser).
 **********************************************************************/
PYX_Parser PYXCALL
PYX_ExternalEntityParserCreate(PYX_Parser p,
                               const PYX_Char *context,
                               const PYX_Char *encoding)
{
  assert(0 && "Not implemented yet!");
  return NULL;
}



/**********************************************************************
 * Free memory used by the parser. Your application is responsible for
 * freeing any memory associated with user data.
 **********************************************************************/
void PYXCALL
PYX_ParserFree(PYX_Parser p)
{
  pyx_free_mallocs(p);
  PYX_MemFree(p, p);
}


/**********************************************************************
 * Clean up the memory structures maintained by the parser so that it
 * may be used again. After this has been called, parser is ready to
 * start parsing a new document. This function may not be used on a
 * parser created using PYX_ExternalEntityParserCreate; it will return
 * PYX_FALSE in that case. Returns PYX_TRUE on success. Your
 * application is responsible for dealing with any memory associated
 * with user data.
 **********************************************************************/
PYX_Bool PYXCALL
PYX_ParserReset(PYX_Parser p, const PYX_Char *encoding)
{
  assert(0 && "Not implemented yet!");
  return PYX_TRUE;
}










/**********************************************************************
 * Parsing
 **********************************************************************/

/**********************************************************************
 * To state the obvious: the three parsing functions PYX_Parse,
 * PYX_ParseBuffer and PYX_GetBuffer must not be called from within a
 * handler unless they operate on a separate parser instance, that is,
 * one that did not call the handler. For example, it is OK to call
 * the parsing functions from within an PYX_ExternalEntityRefHandler,
 * if they apply to the parser created by PYX_ExternalEntityParserCreate.
 **********************************************************************/


/**********************************************************************
 * Parse some more of the document. The string s is a buffer
 * containing part (or perhaps all) of the document. The number of
 * bytes of s that are part of the document is indicated by len. This
 * means that s doesn't have to be null terminated. It also means that
 * if len is larger than the number of bytes in the block of memory
 * that s points at, then a memory fault is likely. The isFinal
 * parameter informs the parser that this is the last piece of the
 * document. Frequently, the last piece is empty (i.e. len is zero.)
 * If a parse error occurred, it returns PYX_STATUS_ERROR. Otherwise
 * it returns PYX_STATUS_OK value.
 **********************************************************************/

enum PYX_Status PYXCALL
PYX_Parse(PYX_Parser p, const char *s, int len, int isFinal)
{
  pyx_parse_new (p, s, len, isFinal);
  return PYX_STATUS_OK;
}



/**********************************************************************
 * This is just like PYX_Parse, except in this case Expat provides the
 * buffer. By obtaining the buffer from Expat with the PYX_GetBuffer
 * function, the application can avoid double copying of the input.
 **********************************************************************/

enum PYX_Status PYXCALL
PYX_ParseBuffer(PYX_Parser p, int len, int isFinal)
{
  pyxparser_t *parser = p;

  if(parser->get_buffer) {
    /*pyx_parse (p, parser->get_buffer, len, isFinal);*/
    PYX_Parse(p, parser->get_buffer, len, isFinal);
    PYX_MemFree(p, parser->get_buffer);
    parser->get_buffer = NULL;
    return PYX_STATUS_OK;
  }
  return PYX_STATUS_ERROR;
}


/**********************************************************************
 * Obtain a buffer of size len to read a piece of the document into. A
 * NULL value is returned if Expat can't allocate enough memory for
 * this buffer. This has to be called prior to every call to
 * PYX_ParseBuffer.
 **********************************************************************/

/**********************************************************************
   A typical use would look like this:
   for (;;) {
     int bytes_read;
     void *buff = PYX_GetBuffer(p, BUFF_SIZE);
     if (buff == NULL) {
       // handle error
     }

     bytes_read = read(docfd, buff, BUFF_SIZE);
     if (bytes_read < 0) {
       // handle error
     }

     if (! PYX_ParseBuffer(p, bytes_read, bytes_read == 0)) {
       // handle parse error
     }

     if (bytes_read == 0)
       break;            
   }
 **********************************************************************/

void * PYXCALL
PYX_GetBuffer(PYX_Parser p, int len)
{
  pyxparser_t *parser = p;

  if(parser->get_buffer)
    PYX_MemFree(p, parser->get_buffer);

  parser->get_buffer = PYX_MemMalloc(p, len);
  
  return parser->get_buffer;
}

 












/**********************************************************************
                      
  Stops parsing, causing PYX_Parse or PYX_ParseBuffer to return. Must be
  called from within a call-back handler, except when aborting (when
  resumable is PYX_FALSE) an already suspended parser. Some call-backs
  may still follow because they would otherwise get lost, including

     * the end element handler for empty elements when stopped in the
       start element handler,                                        
     * end namespace declaration handler when stopped in the end element
       handler,                                                         

   and possibly others.

   This can be called from most handlers, including DTD related
   call-backs, except when parsing an external parameter entity and
   resumable is PYX_TRUE. Returns PYX_STATUS_OK when successful,
   PYX_STATUS_ERROR otherwise. The possible error codes are:

   PYX_ERROR_SUSPENDED
          when suspending an already suspended parser.

   PYX_ERROR_FINISHED
          when the parser has already finished.

   PYX_ERROR_SUSPEND_PE
          when suspending while parsing an external PE.

   Since the stop/resume feature requires application support in the
   outer parsing loop, it is an error to call this function for a parser
   not being handled appropriately; see Temporarily Stopping Parsing for
   more information.

   When resumable is PYX_TRUE then parsing is suspended, that is,
   PYX_Parse and PYX_ParseBuffer return PYX_STATUS_SUSPENDED. Otherwise,
   parsing is aborted, that is, PYX_Parse and PYX_ParseBuffer return
   PYX_STATUS_ERROR with error code PYX_ERROR_ABORTED.

   Note: This will be applied to the current parser instance only, that
   is, if there is a parent parser then it will continue parsing when the
   external entity reference handler returns. It is up to the
   implementation of that handler to call PYX_StopParser on the parent
   parser (recursively), if one wants to stop parsing altogether.

   When suspended, parsing can be resumed by calling PYX_ResumeParser.

 **********************************************************************/

enum PYX_Status PYXCALL
PYX_StopParser(PYX_Parser p, PYX_Bool resumable)
{
  assert(0 && "Not implemented yet!");
  return PYX_STATUS_OK;
}
 

/**********************************************************************
 * Resumes parsing after it has been suspended with
 * PYX_StopParser. Must not be called from within a handler
 * call-back. Returns same status codes as PYX_Parse or
 * PYX_ParseBuffer. An additional error code, PYX_ERROR_NOT_SUSPENDED,
 * will be returned if the parser was not currently suspended.
 *
 * Note: This must be called on the most deeply nested child parser
 * instance first, and on its parent parser only after the child
 * parser has finished, to be applied recursively until the document
 * entity's parser is restarted. That is, the parent parser will not
 * resume by itself and it is up to the application to call
 * PYX_ResumeParser on it at the appropriate moment.
 **********************************************************************/

enum PYX_Status PYXCALL
PYX_ResumeParser(PYX_Parser p)
{
  assert(0 && "Not implemented yet!");
  return PYX_STATUS_OK;
}


/**********************************************************************
 * Returns status of parser with respect to being initialized,
 * parsing, finished, or suspended, and whether the final buffer is
 * being processed. The status parameter must not be NULL.
 **********************************************************************/

void PYXCALL
PYX_GetParsingStatus(PYX_Parser p, PYX_ParsingStatus *status)
{
  assert(0 && "Not implemented yet!");
}



















/**********************************************************************
 * Parse position and error reporting functions
 **********************************************************************/

/**********************************************************************
 * These are the functions you'll want to call when the parse
 * functions return PYX_STATUS_ERROR (a parse error has occurred),
 * although the position reporting functions are useful outside of
 * errors. The position reported is the byte position (in the original
 * document or entity encoding) of the first of the sequence of
 * characters that generated the current event (or the error that
 * caused the parse functions to return PYX_STATUS_ERROR.) The
 * exceptions are callbacks trigged by declarations in the document
 * prologue, in which case they exact position reported is somewhere
 * in the relevant markup, but not necessarily as meaningful as for
 * other events.
 *
 *  The position reporting functions are accurate only outside of the
 *  DTD.  In other words, they usually return bogus information when
 *  called from within a DTD declaration handler.
 **********************************************************************/

/**********************************************************************
 * Return what type of error has occurred.
 **********************************************************************/
enum PYX_Error PYXCALL
PYX_GetErrorCode(PYX_Parser p)
{
  return PYX_ERROR_NONE;
}

/**********************************************************************
 * Return a string describing the error corresponding to code. The
 * code should be one of the enums that can be returned from
 * PYX_GetErrorCode.
 **********************************************************************/
const PYX_LChar * PYXCALL
PYX_ErrorString(enum PYX_Error code)
{
  switch(code)
    {
    default:
      return "Not implemented yet!\n";
    }
}


/**********************************************************************
 * Return the byte offset of the position. This always corresponds to
 * the values returned by PYX_GetCurrentLineNumber and
 * PYX_GetCurrentColumnNumber.
 **********************************************************************/
long PYXCALL
PYX_GetCurrentByteIndex(PYX_Parser p)
{
  return 1;
}


/**********************************************************************
 * Return the line number of the position. The first line is reported
 * as 1.
 **********************************************************************/
int PYXCALL
PYX_GetCurrentLineNumber(PYX_Parser p)
{
  return 1;
}

/**********************************************************************
 * Return the offset, from the beginning of the current line, of the
 * position.
 **********************************************************************/
int PYXCALL
PYX_GetCurrentColumnNumber(PYX_Parser p)
{
  return 1;
}

/**********************************************************************
 * Return the number of bytes in the current event. Returns 0 if the
 * event is inside a reference to an internal entity and for the
 * end-tag event for empty element tags (the later can be used to
 * distinguish empty-element tags from empty elements using separate
 * start and end tags).
 **********************************************************************/
int PYXCALL
PYX_GetCurrentByteCount(PYX_Parser p)
{
  return 1;
}



/**********************************************************************
 * Returns the parser's input buffer, sets the integer pointed at by
 * offset to the offset within this buffer of the current parse
 * position, and set the integer pointed at by size to the size of the
 * returned buffer.
 *
 * This should only be called from within a handler during an active
 * parse and the returned buffer should only be referred to from
 * within the handler that made the call. This input buffer contains
 * the untranslated bytes of the input.
 *
 * Only a limited amount of context is kept, so if the event
 * triggering a call spans over a very large amount of input, the
 * actual parse position may be before the beginning of the buffer.
 *
 * If PYX_CONTEXT_BYTES is not defined, this will always return NULL.
 **********************************************************************/
const char * PYXCALL
PYX_GetInputContext(PYX_Parser p, int *offset, int *size)
{
  return NULL;
}

