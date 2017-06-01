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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "internal.h"

void ISO9735Decimal_init(FSAutomaton *fsa);
void EDIFACTNumeric_init(FSAutomaton *fsa);

/*char *
edi_get_element_by_namex (edi_directory_t *d, edi_segment_t *s, char *element)
{
  int x = 0, y = 0;
  char key[256], *value = NULL;

  if(!d || !s || !element)
    return value;
 
  sprintf (key, "%s/%s", edi_segment_get_code (s), element);

  value = edi_directory_element_index (d, key, &x, &y) ?
    edi_segment_get_element (s, x, y) : NULL;

  return value;
}*/



char *
edi_itype_string (edi_interchange_type_t type)
{
  switch (type)
    {
    case EDI_UNKNOWN:
      return "Unknown";

    case EDI_EDIFACT:
      return "UN/EDIFACT";

    case EDI_UNGTDI:
      return "UN(G)TDI";
      
    case EDI_X12:
      return "ASC X12";

    case EDI_IMP:
      return "Cargo IMP";
    }

  return "This shouldn't happen - please file a bug report";
}



/*
  Perl to fill in switches:
  perl -ne 'printf "case %s:\n\treturn \"%s\";\n",
  $1, $2 if m:(\w+),?.*?/\052\s(.*?)\s+\052/:' common.h
*/

char *
edi_error_string (edi_error_t error)
{
  switch (error)
    {
    case EDI_ENONE:
      return "Success";

    case EDI_EAPP:
      return "Application error";

    case EDI_ECHARSET:
      return "Character not allowed in character set";
    case EDI_ESEGMENT:
      return "Segment contains invalid elements";
    case EDI_ESYNTAX:
      return "Unknown syntax";
    case EDI_ERULES:
      return "Rules corrupt";
    case EDI_EEOF:
      return "Premature end of file";
    case EDI_EGARBAGE:
      return "Trailing garbage";
    case EDI_EREQUIRED:
      return "Required segment missing";
    case EDI_EENVELOPE:
      return "Envelope corrupt";
    case EDI_ETUNKNOWN:
      return "Unknown transaction";
    case EDI_EBADTSG:
      return "TSG badness";
    case EDI_ECORRUPT:
      return "Transaction corrupt";
    case EDI_EITR:
      return "Interchange trailer reference mismatch";
    case EDI_EITC:
      return "Interchange trailer count incorrect";
    case EDI_EGTR:
      return "Functional group trailer reference mismatch";
    case EDI_EGTC:
      return "Functional group trailer count incorrect";
    case EDI_EGMT:
      return "Functional group contains mixed transaction types";
    case EDI_EGMV:
      return "Functional group contains mixed transaction versions";
    case EDI_ETTR:
      return "Transaction trailer reference mismatch";
    case EDI_ETTC:
      return "Transaction trailer count incorrect";
    case EDI_EIEMPTY:
      return "Interchange contains no transactions";
    case EDI_EGEMPTY:
      return "Functional group contains no transactions";
    case EDI_ETEMPTY:
      return "Transaction contains no segments";
    case EDI_ENOTTTH:
      return "No transaction type in transaction header";
    case EDI_ENOMEM:
      return "Memory allocation error";
    }

  /* watch compiler warnings - if a case isn't handled it should complain */
  return "This shouldn't happen - please file a bug report";
}


char *
edi_event_string (edi_event_t event)
{
  switch (event)
    {
    case EDI_NONE:
      return "none";
    case EDI_INTERCHANGE:
      return "interchange";
    case EDI_ADVICE:
      return "servicestringadvice";
    case EDI_GROUP:
      return "functionalgroup";
    case EDI_TRANSACTION:
      return "transaction";
    case EDI_SECTION:
      return "section";
    case EDI_LOOP:
      return "loop";
    case EDI_SEGMENT:
      return "segment";
    case EDI_TAG:
      return "tag";
    case EDI_COMPOSITE:
      return "composite";
    case EDI_ELEMENT:
      return "element";
    case EDI_TS:
      return "ts";
    case EDI_ES:
      return "es";
    case EDI_SS:
      return "ss";
    case EDI_ST:
      return "st";
    case EDI_RI:
      return "ri";
    }
  return "unknown";
}


void
edi_advice_init (edi_advice_t *self)
{
  memset(self, 0, sizeof(edi_advice_t));

  edi_advice_set_rs(self, 0, ' ');
  edi_advice_set_ts(self, 0, ' ');
  edi_advice_set_es(self, 0, ' ');
  edi_advice_set_ss(self, 0, ' ');
  edi_advice_set_st(self, 0, ' ');
  edi_advice_set_ri(self, 0, ' ');
  edi_advice_set_dn(self, 0, ' ');

  self->has_ssa = 0;
}




void edi_advice_set_rs (edi_advice_t *self, int active, char c) {
  sprintf((char *) &(self->rs), "%c", c);
  self->rs[2] = active ? 1 : 0;
}

void edi_advice_set_ts (edi_advice_t *self, int active, char c) {
  sprintf((char *) &(self->ts), "%c", c);
  self->ts[2] = active ? 1 : 0;
}

void edi_advice_set_es (edi_advice_t *self, int active, char c) {
  sprintf((char *) &(self->es), "%c", c);
  self->es[2] = active ? 1 : 0;
}

void edi_advice_set_ss (edi_advice_t *self, int active, char c) {
  sprintf((char *) &(self->ss), "%c", c);
  self->ss[2] = active ? 1 : 0;
}

void edi_advice_set_st (edi_advice_t *self, int active, char c) {
  sprintf((char *) &(self->st), "%c", c);
  self->st[2] = active ? 1 : 0;
}

void edi_advice_set_ri (edi_advice_t *self, int active, char c) {
  sprintf((char *) &(self->ri), "%c", c);
  self->ri[2] = active ? 1 : 0;
}

void edi_advice_set_dn (edi_advice_t *self, int active, char c) {
  sprintf((char *) &(self->dn), "%c", c);
  self->dn[2] = active ? 1 : 0;
}



int edi_advice_get_rs (edi_advice_t *self, char *c) {
  *c = self->rs[0];
  return self->rs[2] ? 1 : 0;
}

int edi_advice_get_ts (edi_advice_t *self, char *c) {
  *c = self->ts[0];
  return self->ts[2] ? 1 : 0;
}

int edi_advice_get_es (edi_advice_t *self, char *c) {
  *c = self->es[0];
  return self->es[2] ? 1 : 0;
}

int edi_advice_get_ss (edi_advice_t *self, char *c) {
  *c = self->ss[0];
  return self->ss[2] ? 1 : 0;
}

int edi_advice_get_st (edi_advice_t *self, char *c) {
  *c = self->st[0];
  return self->st[2] ? 1 : 0;
}

int edi_advice_get_ri (edi_advice_t *self, char *c) {
  *c = self->ri[0];
  return self->ri[2] ? 1 : 0;
}

int edi_advice_get_dn (edi_advice_t *self, char *c) {
  *c = self->dn[0];
  return self->dn[2] ? 1 : 0;
}
































unsigned int edi_validate_edifact_numeric (char *element)
{
  FSAutomaton automaton;
  edi_evaluation evaluation = {0, 0, 0};
  unsigned int n, token;

  if(!element)
    return 0;
  
  EDIFACTNumeric_init(&automaton);

  for(n = 0; element[n] != ASCII_NUL; n++)
    {
      switch (element[n])
	{
	case ASCII_HYPHEN:
	  token = EDIFACTNumeric_MINUS;
	  break;
	case ASCII_PERIOD:
	case ASCII_COMMA:
	  token = EDIFACTNumeric_DECMARK;
	  break;
	case ASCII_0:
	case ASCII_1:
	case ASCII_2:
	case ASCII_3:
	case ASCII_4:
	case ASCII_5:
	case ASCII_6:
	case ASCII_7:
	case ASCII_8:
	case ASCII_9:
	  token = EDIFACTNumeric_NUMBER;
	  break;
	default:
	  token = EDIFACTNumeric_DEFAULT;
	}
      FSAProcess(&automaton, &evaluation, element[n], token);
    }

  return (evaluation.failure || evaluation.pending) ? 0 : evaluation.length;
}

unsigned int
edi_validate_edifact_alpha (char *element)
{
  unsigned int n, length, success;
  
  if(!element)
    return 0;

  for (length = 0, success = 1, n = 0; element[n] != ASCII_NUL; n++)
    /* Anything except a digit - see ISO 9735 clause A.1 */
    if ((element[n] >= ASCII_0) && (element[n] <= ASCII_9))
      success = 0;
    else
      length++;
  
  return success && length;
}

unsigned int
edi_validate_edifact_alphanumeric (char *element)
{
  unsigned int n, length;
  
  if(!element)
    return 0;
  
  for (length = 0, n = 0; element[n] != ASCII_NUL; n++)
      length++;
  
  return length;
}


unsigned int
edi_validate_integer_numeric (char *element)
{
  unsigned int n, length, success;

  if(!element)
    return 0;
  
  for (length = 0, success = 1, n = 0; element[n] != ASCII_NUL; n++)
    if ((element[n] <= ASCII_0) || (element[n] >= ASCII_9))
      success = 0;
    else
      length++;
  
  return success ? length : 0;
}



int edi_validate_element
(char *element, edi_data_type_t type, unsigned int min, unsigned int max)
{
  unsigned int length;
  
  if((!element || element[0] == ASCII_NUL))
    return min == 0;

  switch(type)
    {
    case EDI_ISO2382X:
      length = edi_validate_edifact_alphanumeric(element);
      break;
    case EDI_ISO2382A:
      length = edi_validate_edifact_alpha(element);
      break;
    case EDI_ISO2382N:
      length = edi_validate_edifact_numeric(element);
      break;
    case EDI_DECIMAL1:
      length = edi_validate_integer_numeric(element);
      break;
    case EDI_DECIMAL2:
      length = edi_validate_integer_numeric(element);
      break;
    case EDI_DECIMAL3:
      length = edi_validate_integer_numeric(element);
      break;
    case EDI_DECIMAL4:
      length = edi_validate_integer_numeric(element);
      break;
    default:
      length = strlen(element);
    }

  return (length && length >= min && length <= max);
}






void edi_evaluate_element
(edi_data_type_t criteria, char *element, edi_data_type_t as, void *into)
{
  double a_double;
  float a_float;
  long a_long;
  int an_int;

  switch (as)
    {
    case EDI_INT:
      an_int = element ? atoi (element) : 0;
      switch(criteria)
        {
        case EDI_DECIMAL1: an_int /= 10; break;
        case EDI_DECIMAL2: an_int /= 100; break;
        case EDI_DECIMAL3: an_int /= 1000; break;
        case EDI_DECIMAL4: an_int /= 10000; break;
        default: break;
        }
      *((int *) into) = an_int;
      break;
      
    case EDI_LONG:
      a_long = element ? atol (element) : 0;
      switch(criteria)
        {
        case EDI_DECIMAL1: a_long /= 10; break;
        case EDI_DECIMAL2: a_long /= 100; break;
        case EDI_DECIMAL3: a_long /= 1000; break;
        case EDI_DECIMAL4: a_long /= 10000; break;
        default: break;
        }
      *((long *) into) = a_long;
      break;
      
    case EDI_FLOAT:
      a_float = element ? atof (element) : 0;
      switch(criteria)
        {
        case EDI_DECIMAL1: a_float /= 10; break;
        case EDI_DECIMAL2: a_float /= 100; break;
        case EDI_DECIMAL3: a_float /= 1000; break;
        case EDI_DECIMAL4: a_float /= 10000; break;
        default: break;
        }
      *((float *) into) = a_float;
      break;


    case EDI_DOUBLE:
      a_double = element ? atof (element) : 0;
      switch(criteria)
        {
        case EDI_DECIMAL1: a_double /= 10; break;
        case EDI_DECIMAL2: a_double /= 100; break;
        case EDI_DECIMAL3: a_double /= 1000; break;
        case EDI_DECIMAL4: a_double /= 10000; break;
        default: break;
        }
      *((double *) into) = a_double;
      break;

    default:
      break;
    }

  return;
}
