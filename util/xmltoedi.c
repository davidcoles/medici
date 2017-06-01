#include <stdio.h>
#include <expat.h>

/**********************************************************************
 * This program will read in an XML file and strip all of the XML
 * markup from it, leaving just the character data.
 *
 * If you have converted an EDI file to XML using the editoxml program
 * and used the -i (no indentation) and -v (verbose output, all
 * separator characters) switches, eg. like this:
 * 
 * editoxml -i -v -x tsgfile.xml edifile.edi
 *
 * then the resultant XML is simply the raw EDI file with added XML
 * markup. You can use this program to remove the markup and leave the
 * original XML file.
 *
 * You can compile the program like this:
 *
 * cc -o xmltoedi xmltoedi.c -lexpat
 *
 **********************************************************************/

void starthndl(void *v, const char *e, const char **a)
{
  printf("<%s>\n", e);
}

void endhndl(void *v, const char *e)
{
  printf("</%s>\n", e);
}

void charhndl(void *u, const XML_Char *s, int len)
{
  printf("%.*s", len, s);
}

void dflthndl(void *u, const XML_Char *s, int len)
{
  printf("[%.*s]\n", len, s);
}

int
main (int argc, char **argv)
{
  FILE *stream = stdin;
  char buffer[8192];
  size_t nread;
  int done;
  XML_Parser p = NULL;
  
  if (!(p = XML_ParserCreate (NULL)))
    {
      perror ("Couldn't create XML parser");
      return -1;
    }

  /*XML_SetElementHandler (p, starthndl, endhndl);*/
  XML_SetCharacterDataHandler(p, charhndl);
  /*XML_SetDefaultHandler(p, dflthndl);*/
  
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
      
      XML_Parse (p, buffer, nread, done);
    }
  while (!done);
  
  XML_ParserFree (p);
  
  return 0;
}
