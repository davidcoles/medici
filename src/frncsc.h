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

#define StandaloneElementInfo         edi_francesco_element_info_t
#define StandaloneCompositeInfo       edi_francesco_cmpsite_info_t
#define StandaloneSegmentInfo         edi_francesco_segment_info_t
#define StandaloneCodelistInfo        edi_francesco_codelst_info_t
#define StandaloneSegmentContents     edi_francesco_segment_list_t
#define StandaloneCompositeContents   edi_francesco_cmpsite_list_t
#define TransactionSetRule            edi_francesco_trnsctn_rule_t

typedef struct {
  char *code;
  int  datatype;
  int  minsize;
  int  maxsize;
  char *name;
  char *desc;
} edi_francesco_element_info_t;

typedef struct {
  char *code;
  char *name;
  char *desc;
} edi_francesco_cmpsite_info_t;

typedef struct {
  char *composite;
  char *content;
  char requirement;
} edi_francesco_cmpsite_list_t;

typedef struct {
  char *code;
  char *name;
  char *desc;
} edi_francesco_segment_info_t;

typedef struct {
  char *segment;
  char *content;
  char requirement;
  int type;
} edi_francesco_segment_list_t;

typedef struct {
  char *element;
  char *value;
  char *name;
  char *desc;
  /*  char *reference;*/
} edi_francesco_codelst_info_t;

typedef struct {
  char *transaction;
  char *container;
  char *sequence;
  char *code;
  char requirement;
  int  reps;
  int  type;
} edi_francesco_trnsctn_rule_t;

#define EDI_SEGMENT   1
#define EDI_LOOP      2
#define EDI_ELEMENT   3
#define EDI_COMPOSITE 4
#define EDI_COMPONENT 5
#define EDI_SEGLIST   6
#define EDI_CODELIST  7


#if 0
typedef enum {
  EDI_DTX, /* alphanumeric */
  EDI_DTA, /* alpha only */
  EDI_DTN, /* numeric - integer / float */
  EDI_DTF, /* float */
  EDI_DTI, /* integer */
  EDI_DT1, /* implied 1 decimal place */
  EDI_DT2, /* implied 2 decimal place */
  EDI_DT3, /* implied 3 decimal place */
  EDI_DT4  /* implied 4 decimal place */
} edi_datatype_t;
#endif


/* frncsc.c */
void *edi_francesco_create(edi_francesco_element_info_t *,
			   edi_francesco_cmpsite_info_t *,
			   edi_francesco_segment_info_t *,
			   edi_francesco_cmpsite_list_t *,
			   edi_francesco_segment_list_t *,
			   edi_francesco_codelst_info_t *,
			   edi_francesco_trnsctn_rule_t *);

void edi_francesco_free(edi_directory_t *);
