#include <stdlib.h>
#include <stdio.h>
#include <string.h>



/* set to 0 to use the expat library, then switch to 1 to test expyx */

#if 1
#define EXPYX_COMPAT
#include "expyx.h"
#else
#include <expat.h>
#endif

/**********************************************************************
 * NB: this is not intended to produce valid XML - at least not yet.
 * it is just to give a visual clue that the parser might be working
 * properly
 **********************************************************************/

void start_handler(void *v, const char *e, const char **a)
{
  int n;
  
  printf("<%s", e);

  for(n = 0; a[n*2] != NULL; n++)
    printf(" %s='%s'", a[n*2], a[n*2 + 1]);
  
  printf(">");

  (*((int *) v))++;
}

void end_handler(void *v, const char *e)
{
  (*((int *) v))--;
  printf("</%s>", e);
}

void cdata_handler(void *v, const char *c, int l)
{
  printf("%.*s", l, c);
}


void
pi_handler (void *userData, const XML_Char *target, const XML_Char *data)
{
  printf("<?%s %s?>", target, data);
}

void comment_handler (void *userData, const XML_Char *data)
{
  printf("<!-- %s -->", data);

}



int main (void)
{

  XML_Parser p = NULL;
  FILE *stream = stdin;
  char buffer[8192];
  size_t size;
  int done = 0;
  int depth = 0;

  if (!(p = XML_ParserCreate (NULL)))
    {
      perror ("Couldn't create parser");
      goto cleanup;
    }
  
  XML_SetUserData (p, &depth);
  XML_SetElementHandler (p, start_handler, end_handler);
  XML_SetStartElementHandler (p, start_handler);
  XML_SetEndElementHandler (p, end_handler);
  XML_SetCharacterDataHandler (p, cdata_handler);
  XML_SetProcessingInstructionHandler (p, pi_handler);
  XML_SetCommentHandler (p, comment_handler);

  
  while(!done)
    {
      size = fread(buffer, 1, sizeof(buffer), stream);
      
      if (ferror(stream))
        {
          perror("Error reading file");
	  goto cleanup;
        }

      done = feof(stream);
      
      if(XML_Parse(p, buffer, size, done) != XML_STATUS_OK)
        {
	  fprintf (stderr, "Parse error in file at line %d, column %d: %s\n",
		   XML_GetCurrentLineNumber (p),
		   XML_GetCurrentColumnNumber (p),
		   XML_ErrorString (XML_GetErrorCode (p)));
	  goto cleanup;
        }
    }
  
  
 cleanup:

  if (p)
    XML_ParserFree (p);
  
  return 0;
}
