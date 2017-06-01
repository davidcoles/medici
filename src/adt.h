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

#ifndef ADT_H
#define ADT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*edi_key_compare_t) (void *, void *);
typedef unsigned long (*edi_key_hash_t) (void *, unsigned int);
typedef void (*edi_traverse_handler_t)(void *, void *, void *);
typedef void (*edi_free_t)(void *);

typedef struct edi_node_s edi_node_t;
typedef struct
{
  unsigned long length;
  edi_node_t *first;
  edi_node_t *last;
}
edi_list_t;

typedef edi_list_t edi_stack_t;
typedef edi_list_t edi_queue_t;

typedef struct
{
  edi_node_t *root;
}
edi_tree_t;

typedef struct
{
  unsigned long blck;
  unsigned long size;
  void *data;
}
edi_buffer_t;

typedef struct
{
  unsigned int size;
  edi_list_t *list;
  edi_key_compare_t cmp;
  edi_key_hash_t hash;
}
edi_hash_t;

struct edi_node_s
{
  void *key;
  void *data;
  edi_node_t *prev;
  edi_node_t *next;
  int used;
};


#define edi_stack_init  edi_list_init
#define edi_stack_clear edi_list_clear
#define edi_stack_push  edi_list_push
#define edi_stack_peek  edi_list_peek
#define edi_stack_pop   edi_list_pop
#define edi_stack_size  edi_list_length

#define edi_queue_init     edi_list_init
#define edi_queue_drain    edi_list_drain
#define edi_queue_queue    edi_list_push
#define edi_queue_dequeue  edi_list_shift
#define edi_queue_unqueue  edi_list_pop
#define edi_queue_length   edi_list_length

/* adt.c */
void edi_list_init(edi_list_t *);
int edi_list_push(edi_list_t *, void *);
int edi_list_push_key(edi_list_t *, void *, void *);
void *edi_list_pop(edi_list_t *);
void *edi_list_peek(edi_list_t *);
void *edi_list_shift(edi_list_t *);
int edi_list_unshift(edi_list_t *, void *);
int edi_list_unshift_key(edi_list_t *, void *, void *);
void *edi_list_head(edi_list_t *);
void edi_list_drain(edi_list_t *, edi_free_t);
void edi_list_clear(edi_list_t *, edi_free_t);
unsigned long edi_list_length(edi_list_t *);
void *edi_list_exists(edi_list_t *, void *, edi_key_compare_t);
void *edi_list_find(edi_list_t *, void *, edi_key_compare_t);
void edi_tree_init(edi_tree_t *);
void edi_tree_preorder(edi_tree_t *, void *, edi_traverse_handler_t);
void edi_tree_inorder(edi_tree_t *, void *, edi_traverse_handler_t);
void edi_tree_postorder(edi_tree_t *, void *, edi_traverse_handler_t);
void edi_tree_clear(edi_tree_t *, edi_free_t);
int edi_tree_insert(edi_tree_t *, unsigned long, void *);
void *edi_tree_delete(edi_tree_t *, unsigned long);
int edi_tree_replace(edi_tree_t *, unsigned long, void *, edi_free_t);
int edi_tree_exists(edi_tree_t *, unsigned long);
void *edi_tree_find(edi_tree_t *, unsigned long);
void edi_buffer_init(edi_buffer_t *);
int edi_buffer_append(edi_buffer_t *, void *, unsigned long);
void edi_buffer_clear(edi_buffer_t *);
unsigned long edi_buffer_size(edi_buffer_t *);
void *edi_buffer_data(edi_buffer_t *);
int edi_hash_init(edi_hash_t *, unsigned int, edi_key_compare_t, edi_key_hash_t);
int edi_hash_store(edi_hash_t *, void *, void *);
void *edi_hash_exists(edi_hash_t *, void *);
void *edi_hash_fetch(edi_hash_t *, void *);
void edi_hash_traverse(edi_hash_t *, void *, edi_traverse_handler_t);
void edi_hash_clear(edi_hash_t *, edi_free_t);
unsigned long gtk_hash(void *, unsigned int);
unsigned long tcl_hash(void *, unsigned int);
unsigned long x31_hash(void *, unsigned int);
int edi_adt_unit_tests(void);

#ifdef __cplusplus
}
#endif

#endif /*ADT_H*/
