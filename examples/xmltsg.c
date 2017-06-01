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

/**********************************************************************
 *
 * This file reads a simple XML/PYX based format containing
 * transaction set guidelines and creates an edi_giovanni_t structure.
 *
 **********************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <medici.h>

#ifdef HAVE_LIBEXPAT
#include <expat.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "expyx.h"
#include "xmltsg.h"
#define BUFFSIZE 8192

EDI_Directory edi_giovanni_create(void);
void edi_giovanni_clear(void *);
void edi_giovanni_start(void *, const char *, const char **);
void edi_giovanni_end(void *, const char *);






/**********************************************************************  
 * You can embed an XML/PYX TSG into this program by #include-ing a
 * file filled with the character values of each character in the TSG
 * file - this can be done with a perl line such as the one below,
 * eg.:
 * 
 * perl -pe 's/(.)/"0x".unpack("H2",$1).","/egs; $_.="\n"' <d96a.xml >xmlbuff.h
 * 
 * This will take *quite*a*while* to compile, but the executables don't
 * have to find the TSG at run-time - you can, however, override the
 * embedded TSG at run-time by using a command line switch.
 * 
 * Uncomment the #define directive below to include your file
 *
 **********************************************************************/

/* #define XMLBUFF "xmlbuff.h" */
/* #define PYXBUFF "pyxbuff.h" */

#ifdef XMLBUFF
char xmlbuff_[] = {
#include XMLBUFF
};
char *xmlbuff = xmlbuff_;
#else
char *xmlbuff = NULL;
#endif /* XMLBUFF */

#ifdef PYXBUFF
char pyxbuff_[] = {
#include PYXBUFF
};
char *pyxbuff = pyxbuff_;
#else /* PYXLBUFF */
char *pyxbuff = NULL;
#endif /* PYXLBUFF */






/**********************************************************************
 * Code for reading in XML file from various sources
 **********************************************************************/

EDI_Directory read_xmltsg_stream (FILE *);

EDI_Directory read_xmltsg_file (char *file)
{
  EDI_Directory tsg;
  FILE *stream;
  
  if(!(stream = fopen(file, "r")))
    {
      /*perror("Couldn't open XML file");*/
      perror(file);
      return NULL;
    }
  
  tsg = read_xmltsg_stream(stream);
  
  fclose(stream);
  
  return tsg;
}




EDI_Directory read_xmltsg_buffer (char *buff, unsigned int size)
{
#ifdef HAVE_LIBEXPAT
  XML_Parser p = NULL;
  EDI_Directory tsg = NULL;
  
  if(!(tsg = edi_giovanni_create ()))
    {
      perror ("Couldn't create TSG");
      goto cleanup;
    }
  
  if (!(p = XML_ParserCreate (NULL)))
    {
      perror ("Couldn't create XML parser");
      goto cleanup;
    }
  
  XML_SetUserData (p, tsg);
  XML_SetElementHandler (p, edi_giovanni_start, edi_giovanni_end);
  
  XML_Parse (p, buff, size ? size : strlen(buff), 1);

  if(XML_GetErrorCode (p))
    {
      fprintf (stderr, "Parse error in XML file at line %d: %s\n",
	       XML_GetCurrentLineNumber (p),
	       XML_ErrorString (XML_GetErrorCode (p)));
      goto cleanup;
    }
  
  /* clear down stack in case xml reading routine didn't do it properly */
  edi_giovanni_clear(tsg);
  
  XML_ParserFree (p);
  
  return tsg;
  
 cleanup:
  
  if (p)
    XML_ParserFree (p);
  
  if (tsg)
    EDI_DirectoryFree (tsg);
#else
  fprintf (stderr, "expat.h not available at compile time!\n");
#endif
  
  return NULL;
}




EDI_Directory read_xmltsg_stream (FILE *stream)
{
#ifdef HAVE_LIBEXPAT
  char buff[BUFFSIZE];
  size_t size;
  XML_Parser p = NULL;
  EDI_Directory tsg = NULL;
  int done;
        
  if(!(tsg = edi_giovanni_create ()))
    {
      perror ("Couldn't create TSG");
      goto cleanup;
    }
  
  if (!(p = XML_ParserCreate (NULL)))
    {
      perror ("Couldn't create XML parser");
      goto cleanup;
    }
  
  XML_SetUserData (p, tsg);
  XML_SetElementHandler (p, edi_giovanni_start, edi_giovanni_end);
  
  for (;;)
    {
      size = fread(buff, 1, sizeof(buff), stream);
      
      if (ferror(stream))
        {
          perror("Error reading XML file");
	  goto cleanup;
        }

      done = feof(stream);
      
      XML_Parse(p, buff, size, done);

      if(XML_GetErrorCode (p))
        {
	  fprintf (stderr, "Parse error in XML file at line %d: %s\n",
		   XML_GetCurrentLineNumber (p),
		   XML_ErrorString (XML_GetErrorCode (p)));
	  goto cleanup;
        }
      
      if (done)
        break;
    }
  
  /* clear down stack in case xml reading routine didn't do it properly */
  edi_giovanni_clear(tsg);
  
  XML_ParserFree (p);
  
  return tsg;
  
 cleanup:
  
  if (p)
    XML_ParserFree (p);
  
  if (tsg)
    EDI_DirectoryFree (tsg);
#else
  fprintf(stderr, "expat.h not available at compile time!\n");
#endif
  
  return NULL;
}




EDI_Directory read_xmltsg_fd(int fd)
{
#ifdef HAVE_UNISTD_H
#ifdef HAVE_LIBEXPAT
  char buff[BUFFSIZE];
  ssize_t size;
  XML_Parser p = NULL;
  EDI_Directory tsg = NULL;
  int done;       
        
  if(!(tsg = edi_giovanni_create ()))
    {
      perror ("Couldn't create TSG");
      goto cleanup;
    }
  
  if (!(p = XML_ParserCreate (NULL)))
    {
      perror ("Couldn't create XML parser");
      goto cleanup;
    }
  
  XML_SetUserData (p, tsg);
  XML_SetElementHandler (p, edi_giovanni_start, edi_giovanni_end);
  
  for (;;)
    {
      size = read(fd, buff, sizeof(buff));
      
      if (size == -1)
	{
          perror("Error reading XML file");
	  goto cleanup;
        }
      
      done = !size;
      
      XML_Parse(p, buff, size, done);

      if(XML_GetErrorCode (p))
        {
	  fprintf (stderr, "Parse error in XML file at line %d: %s\n",
		   XML_GetCurrentLineNumber (p),
		   XML_ErrorString (XML_GetErrorCode (p)));
	  goto cleanup;
        }
      
      if (done)
        break;
    }
  
  /* clear down stack in case xml reading routine didn't do it properly */
  edi_giovanni_clear(tsg);
  
  XML_ParserFree (p);
  
  return tsg;
  
 cleanup:
  
  if (p)
    XML_ParserFree (p);
  
  if (tsg)
    EDI_DirectoryFree (tsg);

#else /* #ifdef HAVE_LIBEXPAT */
  fprintf(stderr, "expat.h not available at compile time!\n");
#endif

#else /* #ifdef HAVE_UNISTD_H */
  fprintf(stderr, "unistd.h not available at compile time!\n");
#endif
  
  return NULL;
}


























/**********************************************************************
 * Don't edit the following section - it is the XML section from above
 * with s/XML/PYX/gi and #ifdef HAVE_LIBEXPAT clauses taken out. After
 * making a change to the XML code you should copy'n'paste and amend
 * the code in this manner.
 **********************************************************************/

/**********************************************************************
 * Code for reading in PYX file from various sources
 **********************************************************************/

EDI_Directory read_pyxtsg_stream (FILE *);

EDI_Directory read_pyxtsg_file (char *file)
{
  EDI_Directory tsg;
  FILE *stream;
  
  if(!(stream = fopen(file, "r")))
    {
      /*perror("Couldn't open PYX file");*/
      perror(file);
      return NULL;
    }
  
  tsg = read_pyxtsg_stream(stream);
  
  fclose(stream);
  
  return tsg;
}




EDI_Directory read_pyxtsg_buffer (char *buff, unsigned int size)
{

  PYX_Parser p = NULL;
  EDI_Directory tsg = NULL;
  
  if(!(tsg = edi_giovanni_create ()))
    {
      perror ("Couldn't create TSG");
      goto cleanup;
    }
  
  if (!(p = PYX_ParserCreate (NULL)))
    {
      perror ("Couldn't create PYX parser");
      goto cleanup;
    }
  
  PYX_SetUserData (p, tsg);
  PYX_SetElementHandler (p, edi_giovanni_start, edi_giovanni_end);
  
  PYX_Parse (p, buff, size ? size : strlen(buff), 1);

  if(PYX_GetErrorCode (p))
    {
      fprintf (stderr, "Parse error in PYX file at line %d: %s\n",
	       PYX_GetCurrentLineNumber (p),
	       PYX_ErrorString (PYX_GetErrorCode (p)));
      goto cleanup;
    }
  
  /* clear down stack in case pyx reading routine didn't do it properly */
  edi_giovanni_clear(tsg);
  
  PYX_ParserFree (p);
  
  return tsg;
  
 cleanup:
  
  if (p)
    PYX_ParserFree (p);
  
  if (tsg)
    EDI_DirectoryFree (tsg);
  
  return NULL;
}




EDI_Directory read_pyxtsg_stream (FILE *stream)
{
  char buff[BUFFSIZE];
  size_t size;
  PYX_Parser p = NULL;
  EDI_Directory tsg = NULL;
  int done;
        
  if(!(tsg = edi_giovanni_create ()))
    {
      perror ("Couldn't create TSG");
      goto cleanup;
    }
  
  if (!(p = PYX_ParserCreate (NULL)))
    {
      perror ("Couldn't create PYX parser");
      goto cleanup;
    }
  
  PYX_SetUserData (p, tsg);
  PYX_SetElementHandler (p, edi_giovanni_start, edi_giovanni_end);
  
  for (;;)
    {
      size = fread(buff, 1, sizeof(buff), stream);
      
      if (ferror(stream))
        {
          perror("Error reading PYX file");
	  goto cleanup;
        }

      done = feof(stream);
      
      PYX_Parse(p, buff, size, done);

      if(PYX_GetErrorCode (p))
        {
	  fprintf (stderr, "Parse error in PYX file at line %d: %s\n",
		   PYX_GetCurrentLineNumber (p),
		   PYX_ErrorString (PYX_GetErrorCode (p)));
	  goto cleanup;
        }
      
      if (done)
        break;
    }
  
  /* clear down stack in case pyx reading routine didn't do it properly */
  edi_giovanni_clear(tsg);
  
  PYX_ParserFree (p);
  
  return tsg;
  
 cleanup:
  
  if (p)
    PYX_ParserFree (p);
  
  if (tsg)
    EDI_DirectoryFree (tsg);
  
  return NULL;
}




EDI_Directory read_pyxtsg_fd(int fd)
{
#ifdef HAVE_UNISTD_H
  char buff[BUFFSIZE];
  ssize_t size;
  PYX_Parser p = NULL;
  EDI_Directory tsg = NULL;
  int done;       
        
  if(!(tsg = edi_giovanni_create ()))
    {
      perror ("Couldn't create TSG");
      goto cleanup;
    }
  
  if (!(p = PYX_ParserCreate (NULL)))
    {
      perror ("Couldn't create PYX parser");
      goto cleanup;
    }
  
  PYX_SetUserData (p, tsg);
  PYX_SetElementHandler (p, edi_giovanni_start, edi_giovanni_end);
  
  for (;;)
    {
      size = read(fd, buff, sizeof(buff));
      
      if (size == -1)
	{
          perror("Error reading PYX file");
	  goto cleanup;
        }
      
      done = !size;
      
      PYX_Parse(p, buff, size, done);

      if(PYX_GetErrorCode (p))
        {
	  fprintf (stderr, "Parse error in PYX file at line %d: %s\n",
		   PYX_GetCurrentLineNumber (p),
		   PYX_ErrorString (PYX_GetErrorCode (p)));
	  goto cleanup;
        }
      
      if (done)
        break;
    }
  
  /* clear down stack in case pyx reading routine didn't do it properly */
  edi_giovanni_clear(tsg);
  
  PYX_ParserFree (p);
  
  return tsg;
  
 cleanup:
  
  if (p)
    PYX_ParserFree (p);
  
  if (tsg)
    EDI_DirectoryFree (tsg);
  
#else /* #ifdef HAVE_UNISTD_H */
  fprintf(stderr, "unistd.h not available at compile time!\n");
#endif

  return NULL;
}
