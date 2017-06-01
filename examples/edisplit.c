#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <medici.h>


/**********************************************************************
 * This example program reads a stream of concatenated EDI
 * interchanges (on stdin or from a file specified as an argument) and
 * writes each interchange to an individual file (interchange.NNN)
 * whilst reporting extension (the NNN bit), sender/recipient and
 * application/interchange references to stdout.  This sort of thing
 * can be useful when a VAN transfers multiple interchanges (EDIFACT /
 * TRADACOMS / X12 intermixed) in a single unstructured stream.
 **********************************************************************/

typedef struct
{
  FILE *out;
  unsigned int ext;
} user_data_t;


void
start_handler (void *v, EDI_Event event, EDI_Parameters parameters)
{
  user_data_t *user_data = (user_data_t *) v;
  char filename[32];
  char *std, *snd, *rcp, *app, *icr;

  if (event != EDI_INTERCHANGE)
    return;

  std = EDI_GetParameter (parameters, Standard);
  snd = EDI_GetParameter (parameters, SendersId);
  rcp = EDI_GetParameter (parameters, RecipientsId);
  app = EDI_GetParameter (parameters, ApplicationReference);
  icr = EDI_GetParameter (parameters, InterchangeControlReference);
  
  printf ("%03d %-7s %-15s %-15s %-6s %s\n", user_data->ext, std ? std : "-",
	  snd ? snd : "-", rcp ? rcp : "-", app ? app : "-", icr ? icr : "-");
  
  sprintf (filename, "interchange.%03d", user_data->ext++);

  if (!(user_data->out = fopen (filename, "w")))
    {
      perror ("fopen()");
      exit (-1);
    }
}


void
end_handler (void *v, EDI_Event event)
{
  user_data_t *user_data = (user_data_t *) v;

  if (event == EDI_INTERCHANGE && user_data->out)
    {
      fclose (user_data->out);
      user_data->out = NULL;
    }
}


void default_handler(void *v, const char *text, int size)
{
  user_data_t *user_data = (user_data_t *) v;

  if (!fwrite (text, size, 1, user_data->out))
    {
      perror ("fwrite()");
      exit (-1);
    }  
}


int
main (int argc, char **argv)
{
  FILE *stream = stdin;
  char buffer[8192];
  unsigned int length, parsed, offset;
  EDI_Parser parser;
  user_data_t user_data;

  user_data.out = NULL;
  user_data.ext = 0;

  if (!(parser = EDI_ParserCreate ()))
    {
      perror ("EDI_ParserCreate()");
      return -1;
    }

  EDI_SetUserData (parser, &user_data);
  EDI_SetStartHandler (parser, start_handler);
  EDI_SetEndHandler (parser, end_handler);
  EDI_SetDefaultHandler (parser, default_handler);
  
  if (argc > 1)
    if (!(stream = fopen (argv[1], "r")))
      {
	perror ("Couldn't open file");
	return -1;
      }
  
  printf ("%-3s %-7s %-15s %-15s %-6s %s\n",
	  "EXT", "SYNTAX", "FROM", "TO", "APPREF", "REFERENCE");
  
  while (!feof (stream))
    {
      parsed = 0;
      offset = 0;
      length = fread (buffer, 1, sizeof (buffer), stream);
      
      if (ferror (stream))
	{
	  perror ("fread()");
	  return -1;
	}
      
      while (length && !EDI_GetErrorCode (parser))
	{
	  parsed = EDI_Parse (parser, buffer + offset, length, feof (stream));
	  length -= parsed;
	  offset += parsed;
	  
	  if (EDI_GetErrorCode (parser))
	    {
	      fprintf (stderr, "%s at segment %ld\n",
		       EDI_GetErrorString (EDI_GetErrorCode (parser)),
		       EDI_GetCurrentSegmentIndex (parser));
	      return -1;
	    }
	  
	  if (EDI_InterchangeComplete (parser))
	    EDI_ParserReset (parser);
	}
    }
  
  EDI_ParserFree (parser);

  fclose(stream);

  return 0;
}
