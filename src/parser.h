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

#ifndef PARSER_H
#define PARSER_H

typedef void (*edi_syntax_fini_t) (edi_parser_t *);
typedef edi_parameters_t* (*edi_parser_info_t) (edi_parser_t *);

typedef void (*edi_separator_handler_t) (void *, edi_event_t, char);
typedef void (*edi_complete_handler_t) (void *);
typedef void (*edi_character_handler_t) (void *, char *, long);
typedef void (*edi_end_handler_t) (void *, edi_event_t);
typedef void (*edi_start_handler_t) (void *, edi_event_t, edi_parameters_t *);
typedef void (*edi_segment_handler_t) (void *, edi_parameters_t *,
				       edi_segment_t *, edi_directory_t *);
typedef edi_directory_t*(*edi_directory_handler_t)(void *, edi_parameters_t *);

typedef edi_error_t (*edi_syntax_sgmnt_t) (edi_parser_t *);

typedef edi_start_handler_t edi_structure_handler_t;

/**
   \brief Union of all the supported syntaxes

   This union is the first member of the edi_parser_s structure.
*/

typedef union
{
  edi_edifact_t edifact;
  edi_ungtdi_t ungtdi;
  edi_x12_t x12;
  edi_imp_t imp;
}
edi_syntax_t;


/** 
    \brief The main parser struct.
*/

struct edi_parser_s
{
  /*edi_syntax_t syntax;*/
  union
  {
    edi_edifact_t edifact;
    edi_ungtdi_t ungtdi;
    edi_x12_t x12;
  } syntax;
  
  /* maybe put these in a struct? */
  edi_syntax_fini_t syntax_fini;
  edi_syntax_sgmnt_t sgmnt_handler;

  edi_interchange_type_t interchange_type;


  edi_stack_t stack;
  edi_segment_t *segment;
  edi_error_t error;
  edi_pragma_t pragma;
  edi_advice_t *advice;

  unsigned long segment_count;

  edi_buffer_t parse_buffer;

  int de;
  int cde;

  /* private */
  void *user_data;

  /*int handler_arg;*/

  edi_error_handler_t error_handler;
  edi_error_handler_t warning_handler;
  edi_token_handler_t token_handler;  
  edi_character_handler_t text_handler;
  edi_character_handler_t default_handler;

  edi_start_handler_t start_handler;
  edi_end_handler_t end_handler;
  edi_complete_handler_t complete_handler;

  edi_separator_handler_t separator_handler;

  edi_segment_handler_t segment_handler;
  edi_directory_handler_t directory_handler;

  edi_tokeniser_t tokeniser;
  edi_queue_t token_queue;

  edi_directory_t *service;
  edi_directory_t *message;

  int done;
};


/* parser.c */
void edi_parser_init(edi_parser_t *);
void edi_parser_reset(edi_parser_t *);
edi_parser_t *edi_parser_create(edi_interchange_type_t);
void edi_parser_fini(edi_parser_t *);
void edi_parser_free(edi_parser_t *);
long edi_parser_parse(edi_parser_t *, char *, long, int);
unsigned long edi_parser_get_byte_index(edi_parser_t *);
unsigned long edi_parser_get_segment_index(edi_parser_t *);
int edi_parser_get_error_code(edi_parser_t *);
edi_interchange_type_t edi_parser_interchange_type(edi_parser_t *);
edi_parameters_t *edi_parser_info(edi_parser_t *);
edi_directory_t *edi_parser_service(edi_parser_t *);
edi_directory_t *edi_parser_message(edi_parser_t *);
edi_pragma_t edi_set_pragma_t(edi_parser_t *, edi_pragma_t);
int edi_parser_is_complete(edi_parser_t *);
edi_error_t edi_parser_raise_error(edi_parser_t *, edi_error_t);
void edi_parser_handle_segment(edi_parser_t *, edi_parameters_t *, edi_directory_t *);
edi_directory_t *edi_parser_handle_directory(edi_parser_t *, edi_parameters_t *);
void edi_parser_handle_start(edi_parser_t *, edi_event_t, edi_parameters_t *);
void edi_parser_handle_end(edi_parser_t *, edi_event_t, edi_parameters_t *);
void edi_parser_handle_token(edi_parser_t *, edi_token_t *);
void edi_parser_handle_text(edi_parser_t *, char *, int);
void edi_parser_handle_default(edi_parser_t *, char *, int);
void edi_parser_handle_separator(edi_parser_t *, edi_event_t, char);
void edi_parser_transaction_head(edi_parser_t *, edi_segment_t *, edi_directory_t *, char *);
void edi_parser_transaction_body(edi_parser_t *, edi_segment_t *, edi_directory_t *);
void edi_parser_transaction_tail(edi_parser_t *, edi_segment_t *, edi_directory_t *);
int edi_parser_push_segment(edi_parser_t *, edi_segment_t *);
void edi_parser_pop_segment(edi_parser_t *);
edi_segment_t *edi_parser_peek_segment(edi_parser_t *);
edi_start_handler_t edi_parser_set_start_handler(edi_parser_t *, edi_structure_handler_t);
edi_end_handler_t edi_parser_set_end_handler(edi_parser_t *, edi_end_handler_t);
edi_error_handler_t edi_parser_set_error_handler(edi_parser_t *, edi_error_handler_t);
edi_error_handler_t edi_parser_set_warning_handler(edi_parser_t *, edi_error_handler_t);
edi_token_handler_t edi_parser_set_token_handler(edi_parser_t *, edi_token_handler_t);
edi_directory_handler_t edi_parser_set_directory_handler(edi_parser_t *, edi_directory_handler_t);
edi_segment_handler_t edi_parser_set_segment_handler(edi_parser_t *, edi_segment_handler_t);
edi_character_handler_t edi_parser_set_text_handler(edi_parser_t *, edi_character_handler_t);
edi_character_handler_t edi_parser_set_default_handler(edi_parser_t *, edi_character_handler_t);
edi_complete_handler_t edi_parser_set_complete_handler(edi_parser_t *, edi_complete_handler_t);
edi_separator_handler_t edi_parser_set_separator_handler(edi_parser_t *, edi_separator_handler_t);
void *edi_parser_set_user_data(edi_parser_t *, void *);
int edi_parser_token_handler(void *, edi_token_t *);
void edi_parser_itype_handler(void *, edi_interchange_type_t);
void edi_parser_cmplt_handler(void *);
void edi_parser_segment_events(edi_parser_t *, edi_directory_t *);


#endif /*PARSER_H*/
