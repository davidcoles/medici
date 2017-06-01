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




typedef struct {
  /* edi_item_t must be first member because of interchangable pointers */
  edi_item_t item;
  edi_list_t list;
} edi_gitem_t;


typedef struct {
  edi_node_t *node;
  unsigned int reps;
} edi_giterator_t;

/* edi_directory_t is the first member of edi_giovanni_t. This is done
   for convenience so that we can use the pointers interchangeably and
   so we don't have to malloc another structure. */

typedef struct {
  edi_directory_t directory;
  edi_list_t segments;
  edi_list_t composites;
  edi_list_t elements;
  edi_list_t transactions;
  edi_list_t everything;
  edi_stack_t stack;
  edi_gitem_t *current;
  edi_error_t error;
  int transaction;
} edi_giovanni_t;



/* giovanni.c */
edi_item_t *edi_giovanni_find_element(edi_directory_t *, char *);
edi_item_t *edi_giovanni_find_composite(edi_directory_t *, char *);
edi_item_t *edi_giovanni_find_segment(edi_directory_t *, char *);
edi_item_t *edi_giovanni_find_codelist(edi_directory_t *, char *, char *);
edi_directory_t *edi_giovanni_create(void);
void edi_giovanni_free(edi_directory_t *);
