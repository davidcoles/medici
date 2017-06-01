/**********************************************************************
 * Just a test C++ wrapper around the library
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <ctype.h>

#include <internal.h>
#include <medici.h>

extern "C" { edi_directory_t *read_pyxtsg_file (char *file); }
extern "C" { edi_directory_t *read_xmltsg_file (char *file); }

using namespace std;

class EDIParser
{
private:
  void *parser;
  
public:
  EDIParser();
  virtual ~EDIParser();

  unsigned long parse(char *, unsigned long, int);
  int error(void);
  char *status(void);
  char *errorString(int);

  virtual void errorHandler(int) {}
  virtual void warningHandler(int) {}

  virtual void charHandler(char *, long) {}
  virtual void defaultHandler(char *, long) {}

  virtual void startInterchange(void) {}
  virtual void endInterchage(void) {}

  virtual void startHandler(char *, char **) {}
  virtual void endHandler(char *) {}

  virtual char *directoryHandler(char **) {return NULL;}
};

// Convert edi_parameters_t to char** + free it

char **p2c(edi_parameters_t *p)
{
  char **ptrs;
  char *key, *val;
  int n, m = 0;
  
  if(!(ptrs = (char **) malloc(sizeof(char *) * 2 * (MaxParameter + 1))))
    return NULL;
  
  for(n = 0; n < MaxParameter; n++)
    {   
      if((val = EDI_GetParameter(p, (edi_parameter_t) n)) &&
	 (key = EDI_GetParameterString((edi_parameter_t) n)))
        {
          ptrs[(m*2) + 0] = strdup(key);
          ptrs[(m*2) + 1] = strdup(val);
          m++;
        }
    }
  
  ptrs[(m*2) + 0] = NULL;
  ptrs[(m*2) + 1] = NULL;
  
  return ptrs;
}

void freec(char **ptrs)
{
  char **p = ptrs;

  if(!p)
    return;

  while(*p)
    {
      free(*p);
      p++;
    }

  free(ptrs);
}

// Glue functions for callbacks

static void char_handler(void *v, char *c, long l)
{ ((EDIParser *) v)->charHandler(c, l); }

static void default_handler(void *v, char *c, long l)
{ ((EDIParser *) v)->defaultHandler(c, l); }

static void start_handler(void *v, edi_event_t e, edi_parameters_t *p)
{
  char **ptrs = p2c(p);
  if(EDI_INTERCHANGE == e)
    ((EDIParser *) v)->startInterchange();
  ((EDIParser *) v)->startHandler(edi_event_string(e), ptrs);
  freec(ptrs);
}

static void end_handler(void *v, edi_event_t e)
{
  ((EDIParser *) v)->endHandler(edi_event_string(e));
  if(EDI_INTERCHANGE == e)
    ((EDIParser *) v)->endInterchage();
}

static void error_handler(void *v, edi_error_t e)
{ ((EDIParser *) v)->errorHandler((int) e); }

static void warning_handler(void *v, edi_error_t e)
{ ((EDIParser *) v)->warningHandler((int) e); }

static edi_directory_t *directory_handler(void *v, edi_parameters_t *p)
{
  char **ptrs = p2c(p), *f;
  int n;

  f = ((EDIParser *) v)->directoryHandler((char **) ptrs);
  freec(ptrs);

  if(f && (n = strlen(f))> 4)
    {
      if(!strcmp(f + (n - 4), ".xml"))
	return read_xmltsg_file(f);
      return read_pyxtsg_file(f);
    }
  
  return NULL;
}






// "Real" methods

EDIParser::EDIParser()
{
  if(!(parser = edi_parser_create(EDI_UNKNOWN)))
    {
      throw "Couldn't create parser!";
      return;
    }
  
  edi_parser_init((edi_parser_t *) parser);
  edi_parser_set_user_data((edi_parser_t *) parser, (void *) this);
  edi_parser_set_text_handler((edi_parser_t *) parser, char_handler);
  edi_parser_set_default_handler((edi_parser_t *) parser, default_handler);
  edi_parser_set_start_handler((edi_parser_t *) parser, start_handler);
  edi_parser_set_end_handler((edi_parser_t *) parser, end_handler);
  edi_parser_set_error_handler((edi_parser_t *) parser, error_handler);
  edi_parser_set_warning_handler((edi_parser_t *) parser, warning_handler);
  edi_parser_set_directory_handler((edi_parser_t *) parser, directory_handler);
}

EDIParser::~EDIParser()
{
  edi_parser_free((edi_parser_t *) parser);
}

unsigned long EDIParser::parse(char *data, unsigned long size, int done)
{
  return edi_parser_parse((edi_parser_t *) parser, data, size, done);
}

int EDIParser::error(void)
{
  return edi_parser_get_error_code ((edi_parser_t *) parser);
}

char *EDIParser::status(void)
{
  return edi_error_string((edi_error_t) this->error());
}

char *EDIParser::errorString(int e)
{
  return edi_error_string((edi_error_t) e);
}







































// XML output helpers


#define XMLPRINT(data) \
switch (data) \
{ \
 case '&' : printf("&amp;" ); break;\
 case '<' : printf("&lt;"  ); break;\
 case '"' : printf("&quot;"); break;\
 case '\'': printf("&apos;"); break;\
 case '>' : printf("&gt;"  ); break;\
 default: printf("%c", data); break;\
}

void
xmlnprint (char *cdata, int length)
{
  int n;

  if(!cdata)
    return;

  for (n = 0; n < length; n++)
    XMLPRINT(cdata[n]);
}

void
xmlprint (char *cdata)
{
  int n;

  if(!cdata)
    return;
  
  for (n = 0; cdata[n]; n++)
    XMLPRINT(cdata[n]);
}

void
xmlattributes (char **attributes)
{
  char *attribute, *value;

  while (*attributes)
    {
      attribute = *(attributes++);
      value = *(attributes++);

      if (value)
        {
          printf (" ");
          xmlprint (attribute);
          printf ("=\"");
          xmlprint (value);
          printf ("\"");
        }
    }
}

void
xmlstartelement (char *code, char **attributes)
{
  printf ("<");
  xmlprint (code);
  if (attributes)
    xmlattributes (attributes);
  printf (">");
}

void
xmlendelement (char *code)
{
  printf ("</");
  xmlprint (code);
  printf (">");
}








// Sub-class of the parser for application use



class MyEDIParser : public EDIParser
{
private:
  char file[1024];
  
public:
  int parse(FILE *stream)
  {
    unsigned int length;
    int done;
    char buffer[4096];
    
    do
      {
	length = fread (buffer, 1, sizeof (buffer), stream);
	done = length < sizeof (buffer);
	
	this->EDIParser::parse(buffer, length, done);
      }
    while (!done);
    
    return this->error();
  }
  
  void errorHandler(int e)
  { fprintf(stderr, "ERROR: %s\n", this->errorString(e)); }
  
  void warningHandler(edi_error_t e)
  { fprintf(stderr, "WARNING: %s\n", this->errorString(e)); }
  
  void startInterchange(void)
  { printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"); }
  
  void endInterchage(void)
  { printf("\n"); }
  
  void startHandler(char *e, char **p)
  { xmlstartelement(e, p); }
  
  void endHandler(char *e)
  { xmlendelement(e); }

  void charHandler(char *c, long l)
  { xmlnprint(c, l); }
  
  char *directoryHandler(char **p)
  {
    char *mvn = NULL, *mrn = NULL;
    file[0] = '\0';
    
    for(; *p; p++, p++)
      if(!strcmp(*p, "MessageVersionNumber"))
	mvn = *(p+1);
      else if(!strcmp(*p, "MessageReleaseNumber"))
	mrn = *(p+1);
    
    if(mvn && mrn)
      {
	strcat(file, mvn);
	strcat(file, mrn);
	strcat(file, ".xml");
	for(int n = strlen(file); n; n--)
	  file[n-1] = tolower(file[n-1]);
      }
    
    return file;
  }  
};











int main(int argc, char **argv)
{
  FILE *stream = stdin;
  MyEDIParser p;
  
  if (argc > 1 && !(stream = fopen (argv[1], "r")))
    {
      perror (argv[1]);
      return -1;
    }
  
  return p.parse(stream);
}
  
