#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "xmlout.h"

#define VATTRIBUTES(start, end, code, last) \
char *attribute, *value; \
va_list ap; \
printf ("%s", start); \
xmlprint(code); \
va_start (ap, last); \
while((attribute = va_arg (ap, char *))) { \
  value = va_arg (ap, char *); \
  if(value) { \
    printf(" "); \
    xmlprint(attribute); \
    printf("=\""); \
    xmlprint(value); \
    printf("\""); \
  } \
} \
va_end (ap); \
printf("%s", end);

#define VLIST(start, end, first) \
char *next; \
va_list ap; \
printf ("%s", start); \
xmlprint(first); \
va_start (ap, first); \
while((next = va_arg (ap, char *))) { \
    xmlprint(next); \
} \
va_end (ap); \
printf("%s", end);

/*
  http://www.w3.org/TR/2004/REC-xml-20040204/#dt-escape
  <!ENTITY lt     "&#38;#60;">
  <!ENTITY gt     "&#62;">
  <!ENTITY amp    "&#38;#38;">
  <!ENTITY apos   "&#39;">
  <!ENTITY quot   "&#34;">
*/


#define XMLPRINTF(cdata) \
switch (cdata) \
{ \
 case '&' : printf("&amp;" ); break;\
 case '<' : printf("&lt;"  ); break;\
 case '"' : printf("&quot;"); break;\
 case '\'': printf("&apos;"); break;\
 case '>' : printf("&gt;"  ); break;\
 default: \
   printf ("%c", cdata); \
}




void
xmlnprint (char *cdata, int length)
{
  int n;

  if(!cdata)
    return;

  for (n = 0; n < length; n++)
    XMLPRINTF(cdata[n]);
}

void
xmlprint (char *cdata)
{
  int n;

  if(!cdata)
    return;
  
  for (n = 0; cdata[n]; n++)
    XMLPRINTF(cdata[n]);
}

void
xmlprintn (const char *cdata, int csize)
{
  int n;

  if(!cdata)
    return;
  
  for (n = 0; n < csize; n++)
    XMLPRINTF(cdata[n]);
}

void
vxmlstartelement (char *code, ...)
{
  VATTRIBUTES ("<", ">", code, code);
}

void
vxmlvoidelement (char *code, ...)
{
  VATTRIBUTES ("<", "/>", code, code);
}

void
vxmlprocessinginstruction (char *code, ...)
{
  VATTRIBUTES ("<?", "?>", code, code);
}

void
vxmlcomment (char *first, ...)
{
  char *next; 
  va_list ap; 
  
  printf ("<!-- "); 
  printf("%s", first); 
  va_start (ap, first); 
  while((next = va_arg (ap, char *))) { 
    printf("%s", next);
  } 
  va_end (ap); 
  printf(" -->");
}

void
vxmlelement (char *code, char *cdata, ...)
{
  VATTRIBUTES ("<", ">", code, cdata);
  xmlprint (cdata);
  printf ("</");
  xmlprint (code);
  printf (">");
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
xmlelement (char *code, char **attributes, char *cdata)
{
  printf ("<");
  xmlprint (code);
  if (attributes)
    xmlattributes (attributes);
  printf (">");
  xmlprint (cdata);
  printf ("</");
  xmlprint (code);
  printf (">");
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

void
xmlvoidelement (char *code, char **attributes)
{
  printf ("<");
  xmlprint (code);
  if (attributes)
    xmlattributes (attributes);
  printf ("\n");
  printf ("/>");
}

void
xmlprocessinginstruction (char *code, char **attributes)
{
  printf ("<?");
  xmlprint (code);
  if (attributes)
    xmlattributes (attributes);
  printf ("?>");
}
