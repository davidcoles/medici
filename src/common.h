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

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define EDI_MAJOR_VERSION 0
#define EDI_MINOR_VERSION 1
#define EDI_MICRO_VERSION 9
#define EDI_MEDICI_VERSION "MEDICI_0.1.9"

typedef struct edi_parser_s edi_parser_t;

typedef struct
{
  int major;
  int minor;
  int micro;
} edi_medici_version_t;

typedef enum
{
  /* FIXME - Assign permanent numbers to these once they have settled */
  EDI_ENONE,     /* Success */
  EDI_EAPP,      /* Application error */
  EDI_ECHARSET,  /* Character not allowed in character set */
  EDI_ESEGMENT,  /* Segment contains invalid elements */
  EDI_ESYNTAX,   /* Unknown syntax */
  EDI_ERULES,    /* Rules corrupt */
  EDI_EEOF,      /* Premature end of file */
  EDI_EGARBAGE,  /* Trailing garbage */
  EDI_EREQUIRED, /* Required segment missing */
  EDI_EENVELOPE, /* Envelope corrupt */

  EDI_ECORRUPT,   /* Transaction corrupt */
  EDI_ETUNKNOWN,  /* Unknown transaction */
  EDI_EBADTSG,    /* TSG badness */

  EDI_EITR,      /* Interchange trailer reference mismatch */
  EDI_EITC,      /* Interchange trailer count incorrect */
  EDI_EGTR,      /* Functional group trailer reference mismatch */
  EDI_EGTC,      /* Functional group trailer count incorrect */
  EDI_EGMT,      /* Functional group contains mixed transaction types */
  EDI_EGMV,      /* Functional group contains mixed transaction versions */
  EDI_ETTR,      /* Transaction trailer reference mismatch */
  EDI_ETTC,      /* Transaction trailer count incorrect */
  
  EDI_EIEMPTY,   /* Interchange contains no transactions */  
  EDI_EGEMPTY,   /* Functional group contains no transactions */  
  EDI_ETEMPTY,   /* Transaction contains no segments */  

  EDI_ENOTTTH,   /* No transaction type in transaction header */
  
  EDI_ENOMEM     /* Memory allocation error */
}
edi_error_t;


/**
   \brief Structural event types.  Represents the various abstract
   "elements" that are implied by the sequence of tags in an EDI
   stream.
*/
typedef enum
{
  EDI_NONE,
  EDI_INTERCHANGE,
  EDI_ADVICE,
  EDI_GROUP,
  EDI_TRANSACTION,
  EDI_SECTION,
  EDI_LOOP,
  EDI_SEGMENT,
  EDI_TAG,
  EDI_COMPOSITE,
  EDI_ELEMENT,
  EDI_TS,
  EDI_ES,
  EDI_SS,
  EDI_ST,
  EDI_RI
}
edi_event_t;

#define EDI_TRADACOMS1 EDI_DECIMAL1
#define EDI_TRADACOMS2 EDI_DECIMAL2
#define EDI_TRADACOMS3 EDI_DECIMAL3
#define EDI_TRADACOMS4 EDI_DECIMAL4

#define EDI_ENUMLIST EDI_ISO2382X
#define EDI_TIMESPEC EDI_ISO2382N
#define EDI_DATESPEC EDI_ISO2382N

typedef enum
{
  EDI_ISO2382X,
  EDI_ISO2382A,
  EDI_ISO2382N,

  EDI_DECIMAL1,
  EDI_DECIMAL2,
  EDI_DECIMAL3,
  EDI_DECIMAL4,

  EDI_INTEGER,
  EDI_REAL,

  EDI_INT,
  EDI_LONG,
  EDI_FLOAT,
  EDI_DOUBLE
}
edi_data_type_t;


/**
   \brief Symbols representing the implemented syntax types.

*/

typedef enum
{
  EDI_UNKNOWN = 0,
  EDI_ANY = 0,
  EDI_EDIFACT = 1,
  EDI_UNGTDI = 2,
  EDI_X12 = 3,
  EDI_IMP = 4
}
edi_interchange_type_t;

typedef enum
{
  EDI_PNONE = 0,
  EDI_PCHARSET = 1<<EDI_ECHARSET,/*Allow characters outside character set*/
  EDI_PSEGMENT = 1<<EDI_ESEGMENT,/*Allow segments which do not conform to TSG*/
  EDI_PTUNKNOWN = 1<<EDI_ETUNKNOWN,
  EDI_PCORRUPT = 1<<EDI_ECORRUPT,
  EDI_PREQUIRED = 1<<EDI_EREQUIRED
}
edi_pragma_t;

typedef struct
{
  unsigned int size;
  int fixed;
  int precision;
  edi_data_type_t type;
  unsigned int minsize;
  unsigned int maxsize;
}
edi_data_representation_t;

typedef struct {
  char rs[3]; /* reserved ??? */
  char ts[3]; /* tag separator */
  char es[3]; /* element separator */
  char ss[3]; /* sub-element separator */
  char st[3]; /* segment terminator */
  char ri[3]; /* release indicator */
  char dn[3]; /* decimal notation */
  int has_ssa;
}
edi_advice_t;

#define edi_advice_is_rs(a,c) (a->rs[2] && c == a->rs[0])
#define edi_advice_is_ts(a,c) (a->ts[2] && c == a->ts[0])
#define edi_advice_is_es(a,c) (a->es[2] && c == a->es[0])
#define edi_advice_is_ss(a,c) (a->ss[2] && c == a->ss[0])
#define edi_advice_is_st(a,c) (a->st[2] && c == a->st[0])
#define edi_advice_is_ri(a,c) (a->ri[2] && c == a->ri[0])


typedef struct {
  unsigned int length;
  int failure;
  int pending;
} edi_evaluation;


/**
   \brief Internal representation of TSG items.

   edi_item_t is used to represent TSG item (segment, element, loop,
   etc.) internally. Not all fields are used in all item types.

*/

typedef struct {
  /** \brief Code of the item (eg. UNB, 1001, BOPBNK). */
  char *code;

  /** \brief A natural language short name for the item. */

  char *name;

  /** \brief A longer natural language description of the item's function. */
  char *desc;

  /** \brief Implementation notes in natural language format. */
  char *note;

  /** \brief Tricky. Used to distinguish between simple/composite elements
      and segments/loops in transaction */
  int type;

  /** \brief Boolean value describing requirement.
      0 - not required, !0 - required */
  int reqr;

  /** \brief Kind of deprecated but still used in transaction rules */
  unsigned int reps;

  /** \brief Minimum size of element */
  unsigned int min;

  /** \brief Maximum size of element */
  unsigned int max;

  /** \brief Representation of element */
  edi_data_type_t repr;
} edi_item_t;

#define EDI_NULL_ITEM {NULL,NULL,NULL,NULL,0,0,0,0,0,(edi_data_type_t)0}


/* src/common.c */
/*char *edi_get_element_by_namex(edi_directory_t *, edi_segment_t *, char *);*/
/*char *edi_get_element_by_name(edi_directory_t *, edi_segment_t *, char *, char *);*/
char *edi_itype_string (edi_interchange_type_t);
char *edi_error_string(edi_error_t);
char *edi_event_string(edi_event_t);
void edi_advice_init(edi_advice_t *);
void edi_advice_set_rs(edi_advice_t *, int, char);
void edi_advice_set_ts(edi_advice_t *, int, char);
void edi_advice_set_es(edi_advice_t *, int, char);
void edi_advice_set_ss(edi_advice_t *, int, char);
void edi_advice_set_st(edi_advice_t *, int, char);
void edi_advice_set_ri(edi_advice_t *, int, char);
void edi_advice_set_dn(edi_advice_t *, int, char);
int edi_advice_get_rs(edi_advice_t *, char *);
int edi_advice_get_ts(edi_advice_t *, char *);
int edi_advice_get_es(edi_advice_t *, char *);
int edi_advice_get_ss(edi_advice_t *, char *);
int edi_advice_get_st(edi_advice_t *, char *);
int edi_advice_get_ri(edi_advice_t *, char *);
int edi_advice_get_dn(edi_advice_t *, char *);
unsigned int edi_validate_edifact_numeric(char *);
unsigned int edi_validate_edifact_alpha(char *);
unsigned int edi_validate_edifact_alphanumeric(char *);
unsigned int edi_validate_integer_numeric(char *);
int edi_validate_element(char *, edi_data_type_t, unsigned int, unsigned int);
void edi_evaluate_element(edi_data_type_t, char *, edi_data_type_t, void *);

#ifdef __cplusplus
}
#endif

#endif /*COMMON_H*/
