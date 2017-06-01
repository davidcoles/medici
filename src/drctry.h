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

#ifndef DRCTRY_H
#define DRCTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief ??? */
typedef struct edi_directory_s edi_directory_t;

/** \brief "Pure virtual" base structure for directory implementations.

    A "pure virtual" structure used for holding function pointers to
    the implementation of a directory (aka TSG).

*/


typedef void (*edi_callback_t) (void *, edi_parameters_t *, edi_directory_t *);
typedef void (*edi_callbacks_t) (void *, edi_event_t, edi_parameters_t *);

typedef void (*edi_sgmnth_t) (void *, edi_parameters_t *, edi_directory_t *);
typedef void (*edi_eventh_t) (void *, edi_event_t, edi_parameters_t *);

struct edi_directory_s
{
  /** \brief Pointer for directory implementation specific use. */
  void *user_data;

  /** \brief Called when a transaction starts */
  edi_error_t (*start)(edi_directory_t *, char *);

  /** \brief Called for each segment in a transaction */
  edi_error_t (*old_parse) (edi_directory_t *, edi_segment_t *,
			    edi_parameters_t *, edi_parser_t *,
			    edi_directory_t *);
  
  /** \brief Called for each segment in a transaction */
  edi_error_t (*parse)(edi_directory_t *, char *, void *,
		       edi_eventh_t, edi_eventh_t, edi_sgmnth_t);
  
  /** \brief Called when a transaction ends */
  edi_error_t (*end)(edi_directory_t *);

  /** \brief Called to free all resources for directory */
  void (*free)(edi_directory_t *);

  /** \brief Returns a string containing an element's name */
  char *(*element_name)(edi_directory_t *, char *);
  /** \brief Returns a string containing an element's description */
  char *(*element_desc)(edi_directory_t *, char *);
  /** \brief Returns a string containing implementation notes for element */
  char *(*element_note)(edi_directory_t *, char *);
  
  /** \brief Returns a string containing an composite's name */
  char *(*composite_name)(edi_directory_t *, char *);
  /** \brief Returns a string containing an composite's description */
  char *(*composite_desc)(edi_directory_t *, char *);
  /** \brief Returns a string containing implementation notes for composite */
  char *(*composite_note)(edi_directory_t *, char *);

  /** \brief Returns a string containing an segment's name */
  char *(*segment_name)(edi_directory_t *, char *);
  /** \brief Returns a string containing an segment's description */
  char *(*segment_desc)(edi_directory_t *, char *);
  /** \brief Returns a string containing implementation notes for segment */
  char *(*segment_note)(edi_directory_t *, char *);

  /** \brief Returns the number of elements in the segment's specification */
  unsigned int (*segment_size)(edi_directory_t *, char *);
  /** \brief Returns a structure describing the N'th element in a segment */
  edi_item_t   (*segment_item)(edi_directory_t *, char *, unsigned int);

  /** \brief Returns the number of elements in the composite's specification */
  unsigned int (*composite_size)(edi_directory_t *, char *);
  /** \brief Returns a structure describing the N'th element in a composite */
  edi_item_t   (*composite_item)(edi_directory_t *, char *, unsigned int);

  /** \brief Returns a structure describing an element */
  edi_item_t   (*element_repr)(edi_directory_t *, char *);

  /** \brief Returns a string containing an codelist's name */
  char *(*codelist_name)(edi_directory_t *, char *, char *);
  /** \brief Returns a string containing an codelist's description */
  char *(*codelist_desc)(edi_directory_t *, char *, char *);
  /** \brief Returns a string containing implementation notes for codelist */
  char *(*codelist_note)(edi_directory_t *, char *, char *);

  char (*segment_reqr)(edi_directory_t *, char *, char *);
  char (*composite_reqr)(edi_directory_t *, char *, char *);
  
  int (*segment_type)(edi_directory_t *, char *, char *);
  int (*element_indx)(edi_directory_t *, char *, char *, char *, int *, int *);
  int (*is_composite)(edi_directory_t *, char *, char *);

  /* obsolete */
  /* char **(*_composite_list)(edi_directory_t *, char *);
     char **(*_segment_list)(edi_directory_t *, char *);
     char **(*_codelist_list)(edi_directory_t *, char *);
     edi_data_representation_t (*_element_representation) (edi_directory_t *,
     char *);
  */
};

  /* drctry.c */
  edi_error_t edi_directory_start(edi_directory_t *, char *);
  edi_error_t edi_directory_parse(edi_directory_t *, char *, int, void *, edi_eventh_t, edi_eventh_t, edi_sgmnth_t, edi_eventh_t);
  void edi_directory_free(edi_directory_t *);
  int edi_directory_element_index(edi_directory_t *, char *, int *, int *);
  char *edi_directory_codelist_value(edi_directory_t *, char *, char *);
  char *edi_directory_element_name(edi_directory_t *, char *);
  char *edi_directory_element_desc(edi_directory_t *, char *);
  char *edi_directory_element_note(edi_directory_t *, char *);
  char *edi_directory_segment_name(edi_directory_t *, char *);
  char *edi_directory_segment_desc(edi_directory_t *, char *);
  char *edi_directory_segment_note(edi_directory_t *, char *);
  char *edi_directory_composite_name(edi_directory_t *, char *);
  char *edi_directory_composite_desc(edi_directory_t *, char *);
  char *edi_directory_composite_note(edi_directory_t *, char *);
  int edi_directory_is_composite(edi_directory_t *, char *, char *);
  char edi_directory_segment_reqr(edi_directory_t *, char *, char *);
  char edi_directory_composite_reqr(edi_directory_t *, char *, char *);
  char *edi_directory_codelist_name(edi_directory_t *, char *, char *);
  char *edi_directory_codelist_desc(edi_directory_t *, char *, char *);
  char *edi_directory_codelist_note(edi_directory_t *, char *, char *);
  unsigned int edi_directory_segment_size(edi_directory_t *, char *);
  unsigned int edi_directory_composite_size(edi_directory_t *, char *);
  edi_item_t edi_directory_composite_item(edi_directory_t *, char *, unsigned int);
  edi_item_t edi_directory_segment_item(edi_directory_t *, char *, unsigned int);
  edi_item_t edi_directory_element_repr(edi_directory_t *, char *);
  char *edi_get_element_by_name(edi_directory_t *, edi_segment_t *, char *, char *);
  char *edi_directory_find_element(edi_directory_t *, char *, int, int);
  char *edi_directory_find_composite(edi_directory_t *, char *, int);
  edi_error_t edi_directory_head(edi_directory_t *, edi_segment_t *, edi_parameters_t *, edi_parser_t *, char *);
  edi_error_t edi_directory_body(edi_directory_t *, edi_segment_t *, edi_parameters_t *, edi_parser_t *);
  edi_error_t edi_directory_tail(edi_directory_t *, edi_segment_t *, edi_parameters_t *, edi_parser_t *);



#ifdef __cplusplus
}
#endif

#endif /*DRCTRY_H*/
