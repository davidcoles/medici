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


/** \file giovanni.c

    \brief An example of how to write a Transaction Set Guidelines module.
    
    This is an example implementation of a Transaction Set Guidelines
    module.  It is included in the main MEDICI library for convenience
    and is not required by any core modules.  The example code
    supplied with MEDICI uses it to implement XML and PYX based TSG
    configuration files.

    The data is stored as linear lists so searching is relatively
    slow, but then, this is just an example. To create your own more
    efficient implementation simply create functions which can be
    referenced by the function pointers in the edi_directory_s
    structure and declare a "constructor" function which allocates and
    initialises the edi_directory_s and other data
    (edi_giovanni_create in this instance).

 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "giovanni.h"

#define cmpfn ((int (*)(void *, void *)) mystrcmp)

static int mystrcmp(char *s1, char *s2)
{
  if(s1 && s2)
    return strcmp(s1, s2);
  else if(s1)
    return -1;
  else
    return 1;
}


/**
   \defgroup edi_giovanni edi_giovanni
   \{
*/


edi_item_t *
edi_giovanni_find_element (edi_directory_t *directory, char *code)
{
  edi_giovanni_t *giovanni = (edi_giovanni_t *) directory;
  return giovanni && code ?
    (edi_item_t *) edi_list_find(&(giovanni->elements), code, cmpfn) : NULL;
}

edi_item_t *
edi_giovanni_find_composite (edi_directory_t *directory, char *code)
{
  edi_giovanni_t *giovanni = (edi_giovanni_t *) directory;
  return giovanni && code ?
    (edi_item_t *) edi_list_find(&(giovanni->composites), code, cmpfn) : NULL;
}

edi_item_t *
edi_giovanni_find_segment (edi_directory_t *directory, char *code)
{
  edi_giovanni_t *giovanni = (edi_giovanni_t *) directory;
  return giovanni && code ?
    (edi_item_t *)  edi_list_find(&(giovanni->segments), code, cmpfn) : NULL;
}

edi_item_t *edi_giovanni_find_codelist
(edi_directory_t *directory, char *element, char *code)
{
  edi_gitem_t *entity;
  
  if(!(entity = (edi_gitem_t *) edi_giovanni_find_element(directory, element)))
    return NULL;
  
  if(!(entity = (edi_gitem_t *) edi_list_find(&(entity->list), code, cmpfn)))
    return NULL;
  
  return (edi_item_t *) entity;
}








static unsigned int
segment_size (edi_directory_t *directory, char *ref)
{
  edi_gitem_t *entity;
  entity = (edi_gitem_t *) edi_giovanni_find_segment(directory, ref);
  return entity ? edi_list_length(&(entity->list)) : 0;
}

static edi_item_t segment_item
(edi_directory_t *directory, char *code, unsigned int i)
{
  edi_gitem_t *entity;
  edi_item_t item = EDI_NULL_ITEM, *pitem;
  edi_node_t *node;

  if(!(entity = (edi_gitem_t *) edi_giovanni_find_segment(directory, code)))
    return item;

  for(node = (edi_node_t *) edi_list_head(&(entity->list)); node;
      node = node->next)
    if(!i-- && (entity = (edi_gitem_t *) node->data))
      {
	if(entity->item.type)
	  pitem = edi_giovanni_find_composite(directory, entity->item.code);
	else
	  pitem = edi_giovanni_find_element(directory, entity->item.code);
	
	if(pitem)
	  item = *pitem;
	
	item.type = entity->item.type;
	item.repr = entity->item.repr;
	item.reqr = entity->item.reqr;
      }
  
  return item;
}


static unsigned int composite_size (edi_directory_t *directory, char *ref)
{
  edi_gitem_t *entity;
  entity = (edi_gitem_t *) edi_giovanni_find_composite(directory, ref);
  return entity ? edi_list_length(&(entity->list)) : 0;
}

static edi_item_t composite_item
(edi_directory_t *directory, char *code, unsigned int i)
{
  edi_gitem_t *entity;
  edi_item_t item = EDI_NULL_ITEM, *pitem;
  edi_node_t *node;

  if(!code)
    return item;
  
  if(!(entity = (edi_gitem_t *) edi_giovanni_find_composite(directory, code)))
    return item;
  
  for(node = (edi_node_t *) edi_list_head(&(entity->list)); node;
      node = node->next)
    if(!i-- && (entity = (edi_gitem_t *)  node->data))
      {
	if((pitem = edi_giovanni_find_element(directory, entity->item.code)))
	  item = *pitem;

	item.reqr = entity->item.reqr;
      }

  return item;
}


































static char *codelist_name (edi_directory_t *directory, char *ref1, char *ref2)
{
  edi_item_t *item = edi_giovanni_find_codelist(directory, ref1, ref2);
  return item ? item->name : NULL;
}

static char *codelist_desc (edi_directory_t *directory, char *ref1, char *ref2)
{
  edi_item_t *item = edi_giovanni_find_codelist(directory, ref1, ref2);
  return item ? item->desc : NULL;
}

static char *codelist_note (edi_directory_t *directory, char *ref1, char *ref2)
{
  edi_item_t *item = edi_giovanni_find_codelist(directory, ref1, ref2);
  return item ? item->note : NULL;
}


static char *element_name (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_element(directory, ref);
  return item ? item->name : NULL;
}

static char *element_desc (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_element(directory, ref);
  return item ? item->desc : NULL;
}

static char *element_note (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_element(directory, ref);
  return item ? item->note : NULL;
}


static char *composite_name (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_composite(directory, ref);
  return item ? item->name : NULL;
}

static char *composite_desc (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_composite(directory, ref);
  return item ? item->desc : NULL;
}

static char *composite_note (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_composite(directory, ref);
  return item ? item->note : NULL;
}


static char *segment_name (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_segment(directory, ref);
  return item ? item->name : NULL;
}

static char *segment_desc (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_segment(directory, ref);
  return item ? item->desc : NULL;
}

static char *segment_note (edi_directory_t *directory, char *ref)
{
  edi_item_t *item = edi_giovanni_find_segment(directory, ref);
  return item ? item->note : NULL;
}

static edi_item_t element_repr (edi_directory_t *directory, char *code)
{
  edi_item_t *item, null = EDI_NULL_ITEM;
  return (item = edi_giovanni_find_element(directory, code)) ? *item : null;
}




























































static edi_node_t *first_node (edi_gitem_t *entity)
{
  edi_list_t *list;
  list = &(entity->list);
  return list->first;
}




static edi_error_t
start_transaction(edi_directory_t *directory, char *transaction)
{
  edi_giovanni_t *giovanni = (edi_giovanni_t *) directory;
  edi_gitem_t *entity;
  edi_giterator_t *iterator;
  
  if(!giovanni)
    return EDI_EBADTSG;
  
  edi_stack_init(&(giovanni->stack));
  
  if(!(entity = (edi_gitem_t *) edi_list_find(&(giovanni->transactions),
					      transaction, cmpfn)))
    return EDI_ETUNKNOWN;

  giovanni->transaction = 1;
  
  if(!(iterator = (edi_giterator_t *) malloc(sizeof(edi_giterator_t))))
    return EDI_ENOMEM;
  
  iterator->node = first_node(entity);
  iterator->reps = 0;

  edi_stack_push(&(giovanni->stack), iterator);

  return EDI_ENONE;
}


static edi_error_t
end_transaction(edi_directory_t *directory)
{
  edi_giovanni_t *giovanni = (edi_giovanni_t *) directory;
  edi_stack_t *stack;
  edi_error_t error = EDI_ENONE;
  
  if(!giovanni)
    return EDI_EBADTSG;
  
  /* shorthand */
  stack = &(giovanni->stack);
  
  if(edi_stack_size(stack))
    error = EDI_ECORRUPT;
  
  /* clear down stack */
  while(edi_stack_size(stack))
    free(edi_stack_pop(stack));

  giovanni->transaction = 0;
  
  return error;
}





static void giovanni_free(edi_directory_t *directory)
{
  edi_giovanni_t *giovanni = (edi_giovanni_t *) directory;
  edi_gitem_t *item;
  
  if(!giovanni)
    return;
  
  /* clear down the finder lists (but NOT the data that the nodes point to) */
  
  edi_list_drain(&(giovanni->elements), NULL);
  edi_list_drain(&(giovanni->segments), NULL);
  edi_list_drain(&(giovanni->composites), NULL);
  edi_list_drain(&(giovanni->transactions), NULL);
  
  /* Now remove each item in turn */
  
  while((item = (edi_gitem_t *) edi_list_shift(&(giovanni->everything))))
    {
      /*fprintf(stderr, "%6ld\n", edi_list_length(&(giovanni->everything)));*/
      
      /* clear the item's list (but NOT the data that the nodes point to) */
      edi_list_drain(&(item->list), NULL);
      
      /* free the character strings associated with this item */
      free(item->item.code);
      free(item->item.name);
      free(item->item.desc);
      free(item->item.note);

      /* free the memory for the item structure itself */
      free(item);
    }

  /* finally, free the memory for the directory structure */
  free(giovanni);
}









/* This routine is probably not quite buggy as it was, since i have
   gone through it and commented the logic */

static edi_error_t iterate_transaction
(edi_directory_t *directory, char *code,
 void *userdata,
 edi_eventh_t start,
 edi_eventh_t end,
 edi_sgmnth_t segment)
{
  edi_giovanni_t *giovanni = (edi_giovanni_t *) directory;
  edi_giterator_t *iterator, *new_iterator;
  edi_gitem_t *entity, *new_entity;  
  edi_parameters_t parameters;
  edi_stack_t *stack;
  edi_error_t error = EDI_EBADTSG;
  edi_node_t *node;
  
  edi_parameters_set(&parameters, LastParameter);
  
  /* check for non-null directory, otherwise we will segfault when
     dereferencing members */
  
  if(!giovanni)
    return EDI_EBADTSG;
  
  /* if the start_transaction method failed (no such transaction
     known) then we should probably just hand off the segment */
  
  if(!giovanni->transaction)
    {
      if(segment)
	segment (userdata, &parameters, directory);
      return EDI_ENONE;
    }
  
  /* shorthand */
  stack = &(giovanni->stack);
  
  /* an iterator at the top of the stack allows us to track progress
     through the current loop */
  
  while((iterator = (edi_giterator_t *) edi_stack_peek(stack)))
    {
      /* whilst there are still segments or loops in the container
         loop the node member will contain a pointer to the next
         member. if the node is null then there are no more left and
         we need to clear down this instance of the loop and return to
         the parent */
      
      if(!iterator->node)
        {
	  /* clear down and deallocate the memory for the expired iterator */
          free(edi_stack_pop(stack));
	  
          /* inform application that the end of the loop has been reached */
          if(edi_stack_peek(stack) && end)
	    end (userdata, EDI_LOOP, NULL);
	  
	  /* now that we have completed an instance of the containing
             loop we need to update the count of the number of times
             it has been completed. if there is nothing on the stack
             then we are at the end of the message description */
	  
	  if((iterator = (edi_giterator_t *) edi_stack_peek(stack)))
	    iterator->reps++;

	  /* skip back to the top for another iteration */
          continue;
        }
      
      /* find a pointer to the information about the segment/loop.
         'node' is just the entry in the list implementation, we want
         the data which is being stored at that position in the
         list. if the data pointer is null then the function which
         built the tsg did bad */
      
      if(!(entity = (edi_gitem_t *) iterator->node->data))
	return EDI_EBADTSG;
      
      /* if we have exceeded the reps for this item move on to the
         next one in the list - hmmm, should never end up being
         _greater_ than the reps, but it doesn't hurt to check ... */
      
      if(iterator->reps >= entity->item.reps)
        {
          iterator->reps = 0;
          iterator->node = iterator->node->next;
          continue;
        }
      
      /* is this item marking the start of a loop ? */
      
      if(entity->item.type)
        {
	  /* find the first item in the list of children of this
             loop. if there are no members in the list or the data
             pointer is null then the tsg builder function did bad. */
	  
          if(!(node = first_node(entity)) || !node->data)
            {
              error = EDI_EBADTSG;
              goto reject_segment;
            }
	  
          /* a restriction in this code is that the first item in a
             loop cannot itself be a loop, but must be a mandatory
             segment. all tsg examples that i have seen conform to
             this - otherwise we will have to recurse up to see if the
             loop gets taken or not and it gets more complex to parse */
	  
          if(((edi_gitem_t *) node->data)->item.type)
            {
              error = EDI_EBADTSG;
              goto reject_segment;
            }
          
	  
	  /* everything looks good so far - now we need to create a
             new iterator to track progress through this loop */
	  
	  /* actually, we probably don't have to allocate the new
             iterator until the first segment is matched, but what the
             hoo? */
	  
          if(!(new_iterator = (edi_giterator_t *) malloc(sizeof(edi_gitem_t))))
	    {
	      error = EDI_ENOMEM;
	      goto reject_segment;
	    }
	  
	  /* Sanity checks */
	  /* didn't we just do this? 
	  if(!(node = first_node(entity)) || !node->data)
            {
              error = EDI_EBADTSG;
              goto reject_segment;
	      } */

	  /* point this new iterator at the first item in the child list */
	  
          new_iterator->node = node;
          new_iterator->reps = 0;
          new_entity = (edi_gitem_t *) node->data;
          
          /* does the application provided segment code match the
             first item in the new loop? */
	  
          if(code && !mystrcmp(new_entity->item.code, code))
            {
	      /* Yes, push this iterator onto the stack */
	      
              if(!edi_stack_push(stack, new_iterator))
		{
		  error = EDI_ENOMEM;
		  goto reject_segment;
		}
	      
	      /* Set up parameters to pass to the application for the
                 containing loop and trigger it */
	      
	      if(start)
		{
		  /* populate parameters with context-specific info */
		  edi_parameters_set(&parameters, LastParameter);
		  edi_parameters_set_one(&parameters, Code, entity->item.code);
		  edi_parameters_set_one(&parameters, Name, entity->item.name);
		  edi_parameters_set_one(&parameters, Desc, entity->item.desc);
		  edi_parameters_set_one(&parameters, Note, entity->item.note);
		  start (userdata, EDI_LOOP, &parameters);
		}
	      
	      /* update iterator and entity to reflect the new scope */
	      iterator = new_iterator;
	      entity = new_entity;
	      /* surely this gets done down at 'accept_segment' */
              /* iterator->reps++; */
	      
	      /* our work here is done, now all that remains is to
                 inform the appliction of the successful matching of
                 the segment */
	      
              goto accept_segment;
            }
	  
	  /* no, the application provided segment code did not match
             the first item in the new loop -- discard the new
             iterator as we won't be needing it */
	  
	  free(new_iterator);
          
	  /* if the loop was mandatory and we have not already matched
	     the minimum repetitions (FIXME - i'm assuming it's always
	     a minimum of 1 here), then we have an error */
	  
          if(entity->item.reqr && !iterator->reps)
            {
              error = EDI_EREQUIRED;
              goto reject_segment;
            }
          
	  /* ok, so the loop either wasn't mandatory or we already
             reached the minimum number of repetitions. we're done
             with it now, so move on the next loop/segment */
	  
          iterator->reps = 0;
          iterator->node = iterator->node->next;

	  /* skip back to the top */
          continue;
        }
      
      /* ok, the item was not a loop, so it must must be a
         segment. does the appliction provided segment code match? if
         so then accept the segment */
      
      if(code && !(mystrcmp(entity->item.code, code)))
	goto accept_segment;
      
      /* no. if the segment was mandatory and we have not already
	 matched minimum count (FIXME - i'm assuming it's always a
	 minimum of 1 here), then we have an error */
      
      if(entity->item.reqr && !iterator->reps)
        {
          error = EDI_EREQUIRED;
          goto reject_segment;
        }
      
      /* ok, it wasn't mandatory, so just update the iterator to point */
      /* to the next loop/segment and start the process again */
      
      iterator->reps = 0;
      iterator->node = iterator->node->next;
    }
  
  /* We have fallen off the bottom of the stack - this is an error,
     unless 'code' was null (to indicate a clear down of the stack at
     the end of a message) */
  
  if(!code)
    return EDI_ENONE;
  
  error = EDI_ECORRUPT;
  
 reject_segment:
  /*fprintf(stderr, "--- %s\n", code);*/
  if(segment)
    segment (userdata, &parameters, directory);
  return error;
  
 accept_segment:
  /*fprintf(stderr, "+++ %s %s\n", code, entity->item.desc);*/
  
  edi_parameters_set(&parameters, LastParameter);
  edi_parameters_set_one(&parameters, Code, entity->item.code);
  edi_parameters_set_one(&parameters, Name, entity->item.name);
  edi_parameters_set_one(&parameters, Desc, entity->item.desc);
  edi_parameters_set_one(&parameters, Note, entity->item.note);
  if(segment)
    segment (userdata, &parameters, directory);
  iterator->reps++;
  return EDI_ENONE;
}





/* FIXME - very inneficient, probably */

static int
element_indx (edi_directory_t *d, char *segment,
	      char *element, char *subelement, int *xp, int *yp)     
{
  edi_item_t item;
  unsigned int n, x;
  
  *xp = -1;
  *yp = -1;

  if(!segment || !element)
    return 0;
  
  x = segment_size(d, segment);
  
  for(n = 0; n < x; n++)
    {
      item = segment_item(d, segment, n);
      
      if(item.code && !strcmp(item.code, element))
	{
	  *xp = n;
	  goto found_element;
	}
    }
  
  return 0;
  
 found_element:
  
  if(!item.type)
    {
      *yp = 0;

      /* FIXME - subelement should be null if this is a simple element */
      if(subelement)
	return 0;
      
      return 1;
    }
  
  if(!subelement)
    return 0;
  
  x = composite_size(d, element);
  
  for(n = 0; n < x; n++)
    {
      item = composite_item(d, element, n);
      
      if(item.code && !strcmp(item.code, subelement))
	{
	  *yp = n;
	  goto found_subelement;
	}
    }
  
  return 0;

 found_subelement:
  
  return 1;
}






edi_directory_t *edi_giovanni_create(void)
{
  edi_giovanni_t *giovanni;
  edi_directory_t *directory;
  
  if(!(giovanni = (edi_giovanni_t *) malloc(sizeof(edi_giovanni_t))))
    return NULL;
  
  /* quick and dirty way to initialise all the data structures */
  memset(giovanni, 0, sizeof(edi_giovanni_t));
  
  directory = (edi_directory_t *) giovanni;
  
  /* not strictly necessary as directory == giovanni */
  directory->user_data = giovanni;
  
  /* set all the function pointers in the edi_directory_t struct to
     point to the functions defined in this file. this allows the user
     to call (for example): 
     
     edi_directory_element_name(dptr, "1001");
     
     without having to be aware of the underlying implementation.
  */
  
  directory->element_indx = element_indx;

  directory->element_name = element_name;
  directory->element_desc = element_desc;
  directory->element_note = element_note;
  directory->element_repr = element_repr;
  
  directory->segment_name = segment_name;
  directory->segment_desc = segment_desc;
  directory->segment_note = segment_note;
  directory->segment_size = segment_size;
  directory->segment_item = segment_item;
  
  directory->composite_name = composite_name;
  directory->composite_desc = composite_desc;
  directory->composite_note = composite_note;
  directory->composite_size = composite_size;
  directory->composite_item = composite_item;
  
  directory->codelist_name = codelist_name;
  directory->codelist_desc = codelist_desc;
  directory->codelist_note = codelist_note;
  
  directory->start = start_transaction;
  directory->parse = iterate_transaction;
  directory->end = end_transaction;

  directory->free = giovanni_free;
  
  return directory;
}


/**********************************************************************
 * Had problems with strdup on some platforms - can't remember why now
 * (maybe defined as a macro or something?). No matter, this will do.
 **********************************************************************/

static char *mystrdup(const char *src)
{
  char *dst = NULL;
  unsigned int n;
  
  if(!src || !src[0])
    return dst;
  
  n = strlen(src);
  
  if((dst = malloc(n + 1)))
    {
      strncpy(dst, src, n);
      dst[n] = '\0';
    }
  
  return dst;
}


/**********************************************************************
 * Code for building TSG from parsed XML/PYX
 **********************************************************************/

#define MY_ELEMENT     0
#define MY_COMPOSITE   1
#define MY_SEGMENT     2
#define MY_CODELIST    3
#define MY_TRANSACTION 4
#define MY_VALUE       5
#define MY_ELEMREF     6
#define MY_SEGREF      7
#define MY_LOOP        8
#define MY_COMPONENT   9
#define MY_UNKNOWN    -1

static void tsg_elemattr (edi_gitem_t *gitem, const char **attr)
{
  int i;
  const char *key, *value;
  
  for (i = 0; attr[i]; i += 2)
    {
      key = attr[i];
      value = attr[i + 1];

      if(!value)
        continue;
      
      if(!strcmp(key, "code")) 
        gitem->item.code = mystrdup(value);
      else if(!strcmp(key, "name"))
        gitem->item.name = mystrdup(value);
      else if(!strcmp(key, "desc")) 
        gitem->item.desc = mystrdup(value);
      else if(!strcmp(key, "func")) 
        gitem->item.desc = mystrdup(value);
      else if(!strcmp(key, "note")) 
        gitem->item.note = mystrdup(value);
      else if(!strcmp(key, "min")) 
        gitem->item.min = atoi(value);
      else if(!strcmp(key, "max")) 
        gitem->item.max = atoi(value);
      else if(!strcmp(key, "reps")) 
        gitem->item.reps = atoi(value);
      
      else if(!strcmp(key, "reqr"))
	{
	  if(!strcmp(value, "mandatory"))
	    gitem->item.reqr = 1;
	  else if(!strcmp(value, "conditional"))
	    gitem->item.reqr = 0;
	  else 
	    gitem->item.reqr = 0;
	}
      
      else if(!strcmp(key, "repr"))
	{
	  if(!strcmp(value, "alpha"))
	    gitem->item.repr = EDI_ISO2382A;
	  else if(!strcmp(value, "numeric")) 
	    gitem->item.repr = EDI_ISO2382N;
	  else if(!strcmp(value, "mixed")) 
	    gitem->item.repr = EDI_ISO2382X;
	  else if(!strcmp(value, "enum")) 
	    gitem->item.repr = EDI_ENUMLIST;
	  else if(!strcmp(value, "1decimal")) 
	    gitem->item.repr = EDI_DECIMAL1;
	  else if(!strcmp(value, "2decimal")) 
	    gitem->item.repr = EDI_DECIMAL2;
	  else if(!strcmp(value, "3decimal")) 
	    gitem->item.repr = EDI_DECIMAL3;
	  else if(!strcmp(value, "4decimal")) 
	    gitem->item.repr = EDI_DECIMAL4;
	  else if(!strcmp(value, "integer")) 
	    gitem->item.repr = EDI_INTEGER;
	  else if(!strcmp(value, "real")) 
	    gitem->item.repr = EDI_REAL;
	}
      
      else if(!strcmp(key, "type"))
	{
	  if(!strcmp(value, "simple"))
	    gitem->item.type = 0;
	  else if(!strcmp(value, "composite"))
	    gitem->item.type = 1;
	}
    }
}


static int tsg_elemtype (const char *el)
{
  if(!el)
    return MY_UNKNOWN;
  
  if(!strcmp(el, "segment"))
    return MY_SEGMENT;
  else if (!strcmp(el, "composite"))
    return MY_COMPOSITE;
  else if (!strcmp(el, "element"))
    return MY_ELEMENT;
  else if (!strcmp(el, "codelist"))
    return MY_CODELIST;
  else if (!strcmp(el, "transaction"))
    return MY_TRANSACTION;
  else if (!strcmp(el, "value"))
    return MY_VALUE;
  else if (!strcmp(el, "elemref"))
    return MY_ELEMREF;
  else if (!strcmp(el, "segref"))
    return MY_SEGREF;
  else if (!strcmp(el, "loop"))
    return MY_LOOP;
  else if (!strcmp(el, "component"))
    return MY_COMPONENT;
  else
    return MY_UNKNOWN;
}


void edi_giovanni_start(void *data, const char *el, const char **attr)
{
  int type;
  edi_gitem_t *gitem;
  edi_list_t *list = NULL;
  edi_giovanni_t *tsg = (edi_giovanni_t *) data;
  
  if((type = tsg_elemtype(el)) == MY_UNKNOWN)
    return;
  
  /* FIXME - handle failures better */
  if(!(gitem = malloc(sizeof(edi_gitem_t))))
    {
      tsg->current = NULL;
      return;
    }
  
  edi_list_unshift(&(tsg->everything), gitem);
  
  memset(gitem, 0, sizeof(edi_gitem_t));
  
  tsg_elemattr(gitem, attr);
  
  switch(type)
    {
    case MY_SEGMENT:
      list = &(tsg->segments);
      tsg->current = gitem;
      break;
    case MY_COMPOSITE:
      list = &(tsg->composites);
      tsg->current = gitem;
      break;
    case MY_ELEMENT:
      list = &(tsg->elements);
      tsg->current = gitem;
      break;
    case MY_TRANSACTION:
      list = &(tsg->transactions);
      edi_stack_push (&(tsg->stack), gitem);
      tsg->current = gitem;
      break;
    case MY_LOOP:
      gitem->item.type = 1;
      list = tsg->current ? &(tsg->current->list) : NULL;
      edi_stack_push (&(tsg->stack), gitem);
      tsg->current = gitem;
      break;
    case MY_CODELIST:
      /* associate codelist values with corresponding element */
      tsg->current = (edi_gitem_t *)
	edi_giovanni_find_element((edi_directory_t *) tsg, gitem->item.code);
      break;
    case MY_COMPONENT:
    case MY_ELEMREF:
    case MY_VALUE:
    case MY_SEGREF:
      list = tsg->current ? &(tsg->current->list) : NULL;
      break;
    default:
      list = NULL;
      break;
    }
  
  if(list)
    edi_list_push_key(list, gitem->item.code, gitem);
}


void edi_giovanni_end(void *data, const char *el)
{
  edi_giovanni_t *tsg = (edi_giovanni_t *) data;
  
  switch(tsg_elemtype(el))
    {
    case MY_TRANSACTION:
    case MY_LOOP:
      edi_stack_pop (&(tsg->stack));
      tsg->current = edi_stack_peek(&(tsg->stack));
      break;
    default:
      break;
    }
}

void edi_giovanni_clear(void *data)
{
  edi_giovanni_t *tsg = (edi_giovanni_t *) data;

  edi_stack_clear (&(tsg->stack), NULL);
}





/** \} */
