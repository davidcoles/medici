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

#include "adt.h"

static void
edi_node_init (edi_node_t * self, void *key, void *data)
{
  self->key = key;
  self->data = data;
  self->prev = NULL;
  self->next = NULL;
  self->used = 1;
}

static edi_node_t *
edi_node_create (void *key, void *data)
{
  edi_node_t *node;

  if ((node = (edi_node_t *) malloc (sizeof (edi_node_t))))
    edi_node_init (node, key, data);

  return node;
}




/**********************************************************************
 * List
 * Perl style - use a list as a queue or a stack
 **********************************************************************/

void
edi_list_init (edi_list_t * self)
{
  self->length = 0;
  self->first = NULL;
  self->last = NULL;
}

int
edi_list_push (edi_list_t * self, void *data)
{
  return edi_list_push_key (self, NULL, data);
}


int
edi_list_push_key (edi_list_t * self, void *key, void *data)
{
  edi_node_t *node;

  if (!(node = edi_node_create (key, data)))
    return 0;

  node->prev = self->last;
  self->last = node;

  if (node->prev)
    node->prev->next = node;
  else
    self->first = node;

  self->length++;

  return 1;
}


void *
edi_list_pop (edi_list_t * self)
{
  edi_node_t *node;
  void *data;

  if (!(node = self->last))
    return NULL;

  self->last = node->prev;

  if (self->last)
    self->last->next = NULL;
  else
    self->first = NULL;

  self->length--;

  if ((!self->first || !self->last) && self->length)
    fprintf (stderr, "edi_list_pop: no first/last but length not 0!\n");

  data = node->data;
  free (node);
  return data;
}


void *
edi_list_peek (edi_list_t * self)
{
  return self->last ? self->last->data : NULL;
}


void *
edi_list_shift (edi_list_t * self)
{
  edi_node_t *node;
  void *data;

  if (!(node = self->first))
    return NULL;

  self->first = node->next;

  if (self->first)
    self->first->prev = NULL;
  else
    self->last = NULL;

  self->length--;


  if ((!self->first || !self->last) && self->length)
    fprintf (stderr, "edi_list_shift: no first/last but length not 0!\n");
  
  data = node->data;
  free (node);
  return data;
}



int
edi_list_unshift (edi_list_t * self, void *data)
{
  return edi_list_unshift_key (self, NULL, data);
}


int
edi_list_unshift_key (edi_list_t * self, void *key, void *data)
{
  edi_node_t *node;

  if (!(node = edi_node_create (key, data)))
    return 0;
  
  node->next = self->first;
  self->first = node;
  
  if (node->next)
    node->next->prev = node;
  else
    self->last = node;

  if ((!self->first || !self->last) && self->length)
    fprintf (stderr, "edi_list_unshift: no first/last but length not 0!\n");
  
  self->length++;
  return 1;
}


void *
edi_list_head (edi_list_t * self)
{
  return self->first;
}

void
edi_list_drain (edi_list_t * self, edi_free_t freefn)
{
  void *data;
  
  while (self->length)
    if ((data = edi_list_shift (self)) && freefn)
      freefn (data);
}


void
edi_list_clear (edi_list_t * self, edi_free_t freefn)
{
  void *data;

  while (self->length)
    if ((data = edi_list_pop (self)) && freefn)
      freefn (data);
}

unsigned long
edi_list_length (edi_list_t * self)
{
  return self->length;
}


void *
edi_list_exists (edi_list_t * self, void * key, edi_key_compare_t cmpfn)
{
  edi_node_t *node;

  for (node = self->first; node; node = node->next)
    if (cmpfn (node->key, key) == 0)
      return node;

  return NULL;
}

void *
edi_list_find (edi_list_t * self, void * key, edi_key_compare_t cmpfn)
{
  edi_node_t *node;

  if((node = (edi_node_t *) edi_list_exists(self, key, cmpfn)))
    return node->data;

  return NULL;
}




/**********************************************************************
 * Tree
 **********************************************************************/

void
edi_tree_init (edi_tree_t * self)
{
  self->root = NULL;
}

static void edi_tree_traverse
(edi_node_t * node,
 void *user,
 edi_traverse_handler_t preorder,
 edi_traverse_handler_t inorder,
 edi_traverse_handler_t postorder,
 void (*nodefn) (void *),
 void (*datafn) (void *))
{

  if (preorder && node->used)
    preorder (user, node->key, node->data);

  if (node->prev)
    edi_tree_traverse (node->prev, user,
		      preorder, inorder, postorder, nodefn, datafn);

  if (inorder && node->used)
    inorder (user, node->key, node->data);

  if (node->next)
    edi_tree_traverse (node->next, user,
		      preorder, inorder, postorder, nodefn, datafn);

  if (postorder && node->used)
    postorder (user, node->key, node->data);

  if (datafn)
    datafn (node->data);

  if (nodefn)
    nodefn (node);
}


void
edi_tree_preorder (edi_tree_t * self, void *user, edi_traverse_handler_t h)
{
  if (self->root)
    edi_tree_traverse (self->root, user, h, NULL, NULL, NULL, NULL);
}

void
edi_tree_inorder (edi_tree_t * self, void *user, edi_traverse_handler_t h)
{
  if (self->root)
    edi_tree_traverse (self->root, user, NULL, h, NULL, NULL, NULL);
}

void
edi_tree_postorder (edi_tree_t * self, void *user, edi_traverse_handler_t h)
{
  if (self->root)
    edi_tree_traverse (self->root, user, NULL, NULL, h, NULL, NULL);
}


void
edi_tree_clear (edi_tree_t * self, edi_free_t freefn)
{
  if (self->root)
    edi_tree_traverse (self->root, NULL, NULL, NULL, NULL, free, freefn);

  self->root = NULL;
}

static edi_node_t *
edi_tree_findnode (edi_tree_t * self, unsigned long key, edi_node_t *** rptr)
{
  edi_node_t **root = NULL, *node = NULL;

  if (!self->root)
    root = &(self->root);
  else
    {
      node = self->root;

      while (node)
	{
	  if ((void *) key < node->key)
	    {
	      root = &(node->prev);
	      node = node->prev;
	    }
	  else if ((void *) key > node->key)
	    {
	      root = &(node->next);
	      node = node->next;
	    }
	  else
	    {
	      break;
	    }
	}
    }

  if (rptr)
    *rptr = root;
  return node;
}

int
edi_tree_insert (edi_tree_t * self, unsigned long key, void *data)
{
  edi_node_t **root = NULL, *node = NULL;

  if ((node = edi_tree_findnode (self, key, &root)))
    {
      if (node->used)
	return 0;

      node->used = 1;
      node->data = data;
    }
  else
    {
      if (!(node = edi_node_create ((void *) key, data)))
	return 0;
    }

  /* set the dangling pointer on the parent node (or tree root) */
  /* it may already be set, in which case this is a no-op */
  *root = node;

  /* success! */
  return 1;
}

void *
edi_tree_delete (edi_tree_t * self, unsigned long key)
{
  edi_node_t *node;
  void *data = NULL;

  if ((node = edi_tree_findnode (self, key, NULL)))
    {
      data = node->data;
      node->data = NULL;
      node->used = 0;
    }

  return data;
}

int edi_tree_replace
(edi_tree_t * self, unsigned long key, void *data, edi_free_t freefn)
{
  void *old;

  if ((old = edi_tree_delete (self, key)) && freefn)
    freefn (old);

  return edi_tree_insert (self, key, data);
}

int
edi_tree_exists (edi_tree_t * self, unsigned long key)
{
  edi_node_t *node;
  node = edi_tree_findnode (self, key, NULL);

  if (!node)
    return 0;

  return node->used ? 1 : 0;
}

void *
edi_tree_find (edi_tree_t * self, unsigned long key)
{
  edi_node_t *node;
  node = edi_tree_findnode (self, key, NULL);

  if (!node)
    return NULL;

  return node->used ? node->data : NULL;
}




/**********************************************************************
 * Buffer
 **********************************************************************/

void edi_buffer_init (edi_buffer_t *self)
{
  self->blck = 0;
  self->size = 0;
  self->data = NULL;
}

int edi_buffer_append (edi_buffer_t *self, void *data, unsigned long size)
{
  void *ptr;
  unsigned long blck;

  if(!data || !size)
    return 1;

#define BLCKMULT 64
  
  if((self->size + size + 1) > self->blck)
    {
      blck = ((self->size + size + 1) / BLCKMULT) + 1; /* +1 for trailing \0 */
      blck *= BLCKMULT;
      ptr = self->data ? realloc(self->data, blck) : malloc(blck);
      if(ptr)
	{
	  self->data = ptr;
	  self->blck = blck; /* surely we need to update blck! */
	}
      else
	return 0;
    }
  
  memmove((char *)self->data + self->size, (char *)data, size);
  self->size += size;
  /* mitigate unterminated data being used as a char pointer */
  *((char *) self->data + self->size) = '\0';
  return 1;
}

void edi_buffer_clear (edi_buffer_t *self)
{
  free(self->data);
  self->blck = 0;
  self->size = 0;
  self->data = NULL;
}

unsigned long edi_buffer_size (edi_buffer_t *self)
{
  return self->size;
}

void *edi_buffer_data (edi_buffer_t *self)
{
  return self->data;
}




/**********************************************************************
 * Hash
 **********************************************************************/

static int edi_hash_cmp (void *a, void *b)
{
  if(a < b)
    return -1;
  
  if(a > b)
    return 1;
  
  return 0;
}

static unsigned long edi_hash_hash (void *v, unsigned int size)
{
  return (unsigned long) v;
}


int edi_hash_init
(edi_hash_t *self, unsigned int size, edi_key_compare_t cmpfn, edi_key_hash_t hashfn)
{
  if (!(self->list = (edi_list_t *) malloc (sizeof (edi_list_t) * size)))
    return 0;
  
  self->size = size;
  self->cmp = cmpfn ? cmpfn : edi_hash_cmp;
  self->hash = hashfn ? hashfn : edi_hash_hash;
  
  for (size = 0; size < self->size; size++)
    edi_list_init (self->list + size);
  
  return 1;
}

int
edi_hash_store (edi_hash_t *self, void *key, void *data)
{
  edi_list_t *list;
  unsigned int hash = self->hash (key, self->size);
  list = self->list + (hash % self->size);
  return edi_list_unshift_key (list, key, data);
}

void *edi_hash_exists (edi_hash_t *self, void *key)
{
  edi_list_t *list;

  list = self->list + (self->hash (key, self->size) % self->size);
  return edi_list_exists (list, key, self->cmp);
}


void *edi_hash_fetch (edi_hash_t *self, void *key)
{
  edi_node_t *node;
  
  if((node = (edi_node_t *) edi_hash_exists(self, key)))
    return node->data;

  return NULL;
}


void edi_hash_traverse
(edi_hash_t *self, void *user, edi_traverse_handler_t handler)
{
  unsigned int n;
  edi_node_t *node;
  
  for(n = 0; n < self->size; n++)
    {
      node = (self->list + n)->first;
      while(node)
	{
	  handler(user, node->key, node->data);
	  node = node->next;
	}      
    }
}

void
edi_hash_clear (edi_hash_t *self, edi_free_t freefn)
{
  unsigned int n;
  
  for(n = 0; n < self->size; n++)
    edi_list_clear(self->list + n, freefn);
}




/* Next three hash functions are courtesy of their various packages. */
/* Might not actually use these, but i've just put the code fragments */
/* here to remind me */


unsigned long gtk_hash (void *v, unsigned int i)
{
  char *s = (char *) v;
  const char *p;
  unsigned long h = 0, g;

  for(p = s; *p != '\0'; p += 1) {
    h = ( h << 4 ) + *p;
    if ( ( g = h & 0xf0000000 ) ) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }

  return h;
}


unsigned long tcl_hash(void *v, unsigned int i)
{
  char *string = (char *) v;
  unsigned int result = 0;
  int c;

  while (1) {
    c = *string;
    string++;
    if (c == 0) {
      break;
    }
    result += (result<<3) + c;
  }
  return result;
}


unsigned long x31_hash (void *v, unsigned int i)
{
  char *p = (char *) v;
  unsigned long h = 0;
  for(; *p != '\0'; p += 1) {
    h = ( h << 5 ) - h + *p;
  }
  return h;
} 




/**********************************************************************
 * Unit tests
 **********************************************************************/

/* FIXME - write proper unit tests */
/* for now, if this doesn't bomb, assume it works */

static char *numbers[] = 
{
  "zero",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",
  "nineteen",
  "twenty",
};

static void
edi_adt_unit_tests_data (void *user, void *key, void *data)
{
  printf ("-%2ld: %s\n", (unsigned long) key, (char *) data);
}

static void
edi_adt_unit_tests_atad (void *user, void *key, void *data)
{
  edi_tree_t *tree = (edi_tree_t *) user;
  printf ("+%2ld: %s\n", (unsigned long) data, (char *) key);
  edi_tree_insert (tree, (unsigned long) data, key);
}

int
edi_adt_unit_tests (void)
{
  unsigned int n;
  edi_tree_t tree;
  edi_hash_t hash;

  edi_tree_init (&tree);
  edi_hash_init (&hash, 13, (int (*) (void *, void *)) strcmp, x31_hash);

  /* store numbers in the hash with the name of the number as key and
     the numeric value as data */

  for(n = 0; n < (sizeof(numbers) / sizeof(char *)); n++)
    edi_hash_store(&hash, numbers[n], (void *) &numbers[n]);
  
  /* traverse the hash, inserting records into the tree in a "random" order */
  edi_hash_traverse (&hash, &tree, edi_adt_unit_tests_atad);

  /* traverse the tree in order */
  edi_tree_inorder (&tree, NULL, edi_adt_unit_tests_data);
  
  edi_tree_clear (&tree, NULL);
  edi_hash_clear (&hash, NULL);

  return 1;
}
