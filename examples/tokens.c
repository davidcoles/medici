#include <stdio.h>
#include "../src/internal.h"

/**********************************************************************
 * Simple example of using the internals of the library. Uses the
 * tokeniser to show each individual token in an EDI stream (read from
 * a file or string).  This is normally done by the parser which then
 * adds message structure.  usage: tokens [<edifile>]
 **********************************************************************/

void
itype_handler (void *v, edi_interchange_type_t type)
{
  printf ("# %s\n", edi_itype_string (type));
}

void
error_handler (void *v, edi_error_t error)
{
  fprintf (stderr, "%s!\n", edi_error_string (error));
}

int
token_handler (void *v, edi_token_t * token)
{
  edi_tokentype_t type = token->type;

  switch(type)
    {
    case EDI_TST:
      printf ("\n");
      return 0;
    case EDI_TTS:
      printf ("=");
      return 0;
    case EDI_TES:
      printf ("+");
      return 0;
    case EDI_TSS:
      printf (":");
      return 0;
    case EDI_TSA:
    case EDI_TTG:
    case EDI_TEL:
      break;
    }
  
  printf ("%s%.*s%s", token->first ? "[" : "",
	  (int) token->csize, token->cdata, token->last ? "]" : "");

  return 0;
}

int
main (int argc, char **argv)
{
  FILE *stream = stdin;
  char buffer[8192];
  size_t nread;
  int done;
  edi_tokeniser_t tokeniser;

  edi_tokeniser_init (&tokeniser);
  tokeniser.itype_handler = itype_handler;
  tokeniser.error_handler = error_handler;
  tokeniser.token_handler = token_handler;
  
  if (argc > 1 && !(stream = fopen (argv[1], "r")))
    {
      perror (argv[1]);
      return -1;
    }
  
  do
    {
      nread = fread (buffer, 1, sizeof (buffer), stream);
      
      if(ferror(stream))
	{
	  perror("Error reading stream");
	  return -1;
	}

      done = feof(stream);

      edi_tokeniser_parse (&tokeniser, buffer, nread, done);
    }
  while (!done);

  fclose(stream);

  return 0;
}
