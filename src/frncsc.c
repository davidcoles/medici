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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "frncsc.h"

#define CACHE 1

typedef struct {
  edi_directory_t directory;
  edi_stack_t stack;
  edi_hash_t hash;
  char transaction[16];

  edi_francesco_element_info_t *element_info;
  edi_francesco_cmpsite_info_t *cmpsite_info;
  edi_francesco_segment_info_t *segment_info;
  edi_francesco_cmpsite_list_t *cmpsite_list;
  edi_francesco_segment_list_t *segment_list;
  edi_francesco_codelst_info_t *codelst_info;
  edi_francesco_trnsctn_rule_t *trnsctn_rule;
} fdata;

typedef struct {
  edi_francesco_trnsctn_rule_t *rule;
  int    reps;
} fnode;


static edi_francesco_trnsctn_rule_t *getfirstrule
(edi_francesco_trnsctn_rule_t *rules, char *transaction, char *container)
{
  edi_francesco_trnsctn_rule_t *first = rules;

  if(!first || !transaction)
    return NULL;
  
  while(first->transaction)
    {
      if(!strcmp(first->transaction, transaction))
	{
	  if(!first->container && !container)
	    return first;
	  
	  if(first->container && container &&
	     !strcmp(first->container, container))
	    return first;
	}
      first++;
    }
  
  return NULL;
}

static edi_francesco_trnsctn_rule_t
*getnextrule (edi_francesco_trnsctn_rule_t *curr)
{
  edi_francesco_trnsctn_rule_t *next = curr + 1;
  
  if(!curr || !curr->transaction || !next->transaction)
    return NULL;
  
  if(!strcmp(next->transaction, curr->transaction))
    {
      if(!next->container && !curr->container)
	return next;
      
      if(next->container && curr->container &&
	 !strcmp(next->container, curr->container))
	return next;
    }
  
  return NULL;
}






static edi_francesco_element_info_t *
find_element_info (fdata *d, char *code)
{
  edi_francesco_element_info_t *info;

  if(!code)
    return NULL;

  for (info = d->element_info; info->code; info++)
    if (!strcmp (code, info->code))
      return info;
  
  return NULL;
}

static edi_francesco_cmpsite_info_t *
find_cmpsite_info (fdata *d, char *code)
{
  edi_francesco_cmpsite_info_t *info;

  if(!code)
    return NULL;
  
  for (info = d->cmpsite_info; info->code; info++)
    if (!strcmp (code, info->code))
      return info;
  
  return NULL;
}

static edi_francesco_segment_info_t *
find_segment_info (fdata *d, char *code)
{
  edi_francesco_segment_info_t *info;
  
  if(!code)
    return NULL;

  for (info = d->segment_info; info->code; info++)
    if (!strcmp (code, info->code))
      return info;
  
  return NULL;
}

static edi_francesco_codelst_info_t *
find_codelst_info (fdata *d, char *element, char *value)
{
  edi_francesco_codelst_info_t *info;
  
  if(!element || !value)
    return NULL;

  for (info = d->codelst_info; info->element; info++)
    if (!strcmp (element, info->element) &&
	!strcmp (value, info->value))
      return info;

  return NULL;
}

static char *
segment_name (edi_directory_t *d, char *code)
{
  edi_francesco_segment_info_t *s;
  return (s = find_segment_info (d->user_data, code)) ? s->name : NULL;
}

static char *
segment_desc (edi_directory_t *d, char *code)
{
  edi_francesco_segment_info_t *s;
  return (s = find_segment_info (d->user_data, code)) ? s->desc : NULL;
}

static char *
composite_name (edi_directory_t *d, char *code)
{
  edi_francesco_cmpsite_info_t *s;
  return (s = find_cmpsite_info (d->user_data, code)) ?
    s->name : NULL;
}

static char *
composite_desc (edi_directory_t *d, char *code)
{
  edi_francesco_cmpsite_info_t *s;
  return (s = find_cmpsite_info (d->user_data, code)) ?
    s->desc : NULL;
}

static char *
element_name (edi_directory_t *d, char *code)
{
  edi_francesco_element_info_t *s;
  return (s = find_element_info (d->user_data, code)) ? s->name : NULL;
}

static char *
element_desc (edi_directory_t *d, char *code)
{
  edi_francesco_element_info_t *s;
  return (s = find_element_info (d->user_data, code)) ? s->desc : NULL;
}

/*static edi_data_representation_t
element_representation (edi_directory_t *d, char *code)
{
  edi_data_representation_t r = { EDI_BUFFER, 0, EDI_ISO2382X, 0, 0};
  edi_francesco_element_info_t *e;

  if ((e = find_element_info(d->user_data, code)))
    {
      r.minsize = e->minsize;
      r.minsize = e->maxsize;
      r.type = e->datatype;

      r.fixed = (e->minsize == e->maxsize);
      r.size = e->maxsize;
    }

  return r;
}*/


static edi_item_t element_repr
(edi_directory_t *d, char *code)
{
  edi_item_t item = EDI_NULL_ITEM;
  edi_francesco_element_info_t *e;
  
  if ((e = find_element_info(d->user_data, code)))
    {
      item.code = e->code;
      item.min = e->minsize;
      item.max = e->maxsize;
      item.repr = e->datatype;
    }

  return item;
}





static edi_francesco_cmpsite_list_t *
standaloneCompositeContents (fdata *d, char *composite, char *content)
{
  edi_francesco_cmpsite_list_t *cmpsite_list =
    d->cmpsite_list;

  int n;
  for (n = 0; cmpsite_list[n].composite; n++)
    if (!strcmp (composite, cmpsite_list[n].composite) &&
        !strcmp (content, cmpsite_list[n].content))
      return cmpsite_list + n;
  return NULL;
}

static edi_francesco_segment_list_t *
standaloneSegmentContents (fdata *d, char *segment, char *content)
{
  edi_francesco_segment_list_t *contents;

  for (contents = d->segment_list; contents->segment; contents++)
    if (!strcmp (segment, contents->segment) &&
        !strcmp (content, contents->content))
      return contents;

  return NULL;
}



static char
getSegmentContentReqr (edi_directory_t *d, char *segment, char *content)
{
  edi_francesco_segment_list_t *contents;

  if(!(contents = standaloneSegmentContents (d->user_data, segment, content)))
    return 'C';
  
  return contents->requirement;
}

static char
getCompositeContentReqr (edi_directory_t *d, char *composite, char *content)
{
  edi_francesco_cmpsite_list_t *contents;

  if(!(contents = standaloneCompositeContents (d->user_data, composite,
					       content)))
    return 'C';
  
  return contents->requirement;
}





static int
segment_type (edi_directory_t *d, char *segment, char *content)
{
  edi_francesco_segment_list_t *segment_list;
  return ((segment_list =
	   standaloneSegmentContents(d->user_data, segment, content))) ?
    segment_list->type : 0;
}

static int
isElementComposite (edi_directory_t *d, char *segment, char *content)
{
  return (segment_type(d, segment, content) == EDI_COMPOSITE);
}

static char *
codelist_name (edi_directory_t *d, char *element, char *value)
{
  edi_francesco_codelst_info_t *c;
  
  if(!d || !element || !value)
    return NULL;

  return (c = find_codelst_info (d->user_data, element, value)) ?
    c->name : NULL;
}


static int
element_indx (edi_directory_t *d, char *segment,
		char *element, char *subelement, int *xp, int *yp)
{
  edi_francesco_segment_list_t *segment_list =
    ((fdata *) d->user_data)->segment_list;
  edi_francesco_cmpsite_list_t *cmpsite_list =
    ((fdata *) d->user_data)->cmpsite_list;
  
  int x = -1, y = -1;
  
  for (; segment_list->segment; segment_list++)
    {
      if (!strcmp (segment, segment_list->segment))
        {
          x++;
          if (!strcmp (element, segment_list->content))
            break;
        }
    }

  if (subelement)
    {
      for (; cmpsite_list->composite;
	   cmpsite_list++)
        if (!strcmp (element, cmpsite_list->composite))
          {
            y++;
            if (!strcmp (subelement, cmpsite_list->content))
              break;
          }
    }
  else
    y = 0;
  
  if (xp)
    *xp = x;

  if (yp)
    *yp = y;

  return (x != -1 && y != -1);
}






/* this is probably never going to be used again */

static edi_error_t iterate_transaction
(edi_directory_t *directory, char *code, edi_parameters_t *parameters,
 void *userdata,
 edi_callbacks_t start,
 edi_callbacks_t end,
 edi_callback_t segment)
{
  edi_francesco_trnsctn_rule_t *loop;
  fdata *self = directory->user_data;
  fnode *node;
  
  /* This would indicate that something was seriously wrong */
  if(!(code))
    return EDI_ECORRUPT;

  if(!(node = edi_stack_peek(&(self->stack))))
    {
      /* there are no rules on the stack - just handle the segment */
      /*edi_parser_handle_segment (parser, parameters, d ? d : SELF);*/
      if(segment)
	segment (userdata, parameters, directory);
      return EDI_ENONE;
    }
  
  for(;;)
    {
      /* If the maximum repetition for the rule is met (or exceeded,
         though that should not happen) move on to the next rule */
      
      if(node->rule->reps && node->reps >= node->rule->reps)
	{
	  /* FIXME - handle this in a proper debugging framework */
	  
	  if(node->reps > node->rule->reps) /* This should never happen */
	    fprintf(stderr, "Warning: rule max reps exceeded!\n");
	  
	  goto nextrule;
	}


      /* Handle start of a loop */
      if(node->rule->type == EDI_LOOP) 
	{
	  if(!(loop = getfirstrule(self->trnsctn_rule,
				   self->transaction,
				   node->rule->sequence)))
	    {
	      fprintf(stderr, "**** %s %p\n", code, (void *) node->rule);
	      return EDI_ERULES;
	    }
	  
	  /************************************************************
	   * FIXME - we assume here that the first rule in a loop is
	   * not also a loop - to the best of my knowledge the first
	   * rule in a loop is always a mandatory segment with maximum
	   * use 1
	   ************************************************************/
	  
	  /* Does segment match this rule */
	  if(!strcmp(code, loop->code))
	    {
	      /* Yes - push rule onto stack */
	      edi_parameters_set_one(parameters, Code, node->rule->code);
	      /*edi_parser_handle_start (parser, EDI_LOOP, parameters);*/
	      if(start)
                start (userdata, EDI_LOOP, parameters);
	      
	      if(!(node = malloc(sizeof(fnode))))
		return EDI_ENOMEM;
	      
	      node->rule = loop;
	      node->reps = 0;
	      
	      if(!edi_stack_push(&(self->stack), node))
		return EDI_ENOMEM;
	    }
	  else
	    goto nextrule;
	}
      
      
      /* Does this segment match the rule in question? */
      
      if(!strcmp(code, node->rule->code))
	{
	  node->reps++;
	  /*edi_parameters_set(&parameters, LastParameter);*/
	  /*edi_parser_handle_segment (parser, parameters, d ? d : SELF);*/
	  if(segment)
	    segment (userdata, parameters, directory);
	  return EDI_ENONE;
	}
      
      /* Is the rule is a mandatory segment? */

      if((node->rule->requirement == 'M') && (node->reps == 0))
	  return EDI_EREQUIRED;      


      /* FIXME - is there a need to do checks based on other
         requirement values here ('O', 'C') ? */

      
    nextrule: /* Because sometimes gotos are just simpler ... */
      
      if((node->rule = getnextrule(node->rule)))
	node->reps = 0;
      else
	{
	  /* We have run out of rules for this loop */
	  /* First we take the current rule off the stack */
	  free(edi_stack_pop(&(self->stack)));
	  
	  /* If there are no more rules on the stack we are SOL */
	  if(!(node = edi_stack_peek(&(self->stack))))
	    return EDI_ECORRUPT;
	  
	  /* Inform the application about the termination of the loop */
	  edi_parameters_set(parameters, Code, node->rule->code);
	  /*edi_parser_handle_end (parser, EDI_LOOP, parameters);*/
	  if(end)
            end (userdata, EDI_LOOP, parameters);
	}
    }
}


static edi_error_t iterate_transaction_wrapper
(edi_directory_t *directory,
 edi_segment_t *segment,
 edi_parameters_t *parameters,
 edi_parser_t *parser,
 edi_directory_t *d)
{
  return iterate_transaction(directory,
                             edi_segment_get_code(segment),
                             parameters,
                             parser,
                             (edi_callbacks_t) edi_parser_handle_start,
                             (edi_callbacks_t) edi_parser_handle_end,
                             (edi_callback_t) edi_parser_handle_segment);
}













static edi_error_t
start_transaction(edi_directory_t *SELF, char *transaction)
{
  fdata *self = SELF->user_data;
  fnode *node;
  edi_francesco_trnsctn_rule_t *rule;

  if(!(rule = getfirstrule(self->trnsctn_rule, transaction, NULL)))
    return EDI_ETUNKNOWN;
  
    strcpy(self->transaction, transaction);
  if(!(node = malloc(sizeof(fnode))))
    return EDI_ENOMEM;  

  if(!edi_stack_push(&(self->stack), (void *) node))
    {
      free(node);
      return EDI_ENOMEM;
    }
  
  strcpy(self->transaction, transaction);
  node->reps = 0;
  node->rule = rule;
  return EDI_ENONE;
}

static edi_error_t
end_transaction(edi_directory_t *SELF)
{
  fdata *self = SELF->user_data;
  edi_stack_clear(&(self->stack), free);
  return EDI_ENONE;
}













static unsigned int segment_size (edi_directory_t *d, char *code)
{
  edi_francesco_segment_list_t *segment_list =
    ((fdata *)d->user_data)->segment_list;
  int n, b;
  
  for (n = b = 0; segment_list[n].segment; n++)
    if (!strcmp (code, segment_list[n].segment))
      b++;
  return b;
}

static edi_item_t segment_item
(edi_directory_t *d, char *code, unsigned int i)
{
  edi_francesco_segment_list_t *segment_list =
    ((fdata *)d->user_data)->segment_list;

  int n;
  
  edi_item_t item = EDI_NULL_ITEM;

  for (n = 0; segment_list[n].segment; n++)
    if (!strcmp (code, segment_list[n].segment))
      break;
  
  
  item.code = segment_list[n+i].content;
  item.type = segment_list[n+i].type == EDI_COMPOSITE ? 1 : 0;
  item.reqr = segment_list[n+i].requirement == 'M' ? 1 : 0;
  return item;
}




















static unsigned int composite_size (edi_directory_t *d, char *code)
{
    edi_francesco_cmpsite_list_t *cmpsite_list =
      ((fdata *)d->user_data)->cmpsite_list;
    int n, b;
    
    for (n = b = 0; cmpsite_list[n].composite; n++)
      if (!strcmp (code, cmpsite_list[n].composite))
	b++;

    return b;
}

static edi_item_t composite_item
(edi_directory_t *d, char *code, unsigned int i)
{
  edi_francesco_cmpsite_list_t *cmpsite_list =
    ((fdata *)d->user_data)->cmpsite_list;
  edi_item_t item = EDI_NULL_ITEM;
  int n;
  
  for (n = 0; cmpsite_list[n].composite; n++)
    if (!strcmp (code, cmpsite_list[n].composite))
      break;
  
  item.code = cmpsite_list[n+i].content;
  item.type = 0;
  item.reqr = cmpsite_list[n+i].requirement == 'M' ? 1 : 0;


  return item;
}

























void * edi_francesco_create
(
 edi_francesco_element_info_t *element_info,
 edi_francesco_cmpsite_info_t *cmpsite_info,
 edi_francesco_segment_info_t *segment_info,
 edi_francesco_cmpsite_list_t *cmpsite_list,
 edi_francesco_segment_list_t *segment_list,
 edi_francesco_codelst_info_t *codelst_info,
 edi_francesco_trnsctn_rule_t *trnsctn_rule
 )
{
  edi_directory_t *directory = NULL;
  fdata *self;
  
  if((self = malloc(sizeof(fdata)))) {
    directory = (edi_directory_t *) self;
    
    memset(self, 0, sizeof(fdata));
    
    self->element_info = element_info;
    self->cmpsite_info = cmpsite_info;
    self->segment_info = segment_info;
    self->cmpsite_list = cmpsite_list;
    self->segment_list = segment_list;
    self->codelst_info = codelst_info;
    self->trnsctn_rule = trnsctn_rule;
    
    edi_stack_init (&(self->stack));
    /*edi_hash_init (&(self->hash), 1847, 
      (int(*)(void *,void*)) strcmp, gtk_hash);*/
    
    directory->start = start_transaction;
    directory->old_parse = iterate_transaction_wrapper;
    directory->end   = end_transaction;
    
    directory->segment_name = segment_name;
    directory->segment_desc = segment_desc;
    directory->segment_size = segment_size;
    directory->segment_item = segment_item;

    directory->composite_name = composite_name;
    directory->composite_desc = composite_desc;
    directory->composite_size = composite_size;
    directory->composite_item = composite_item;
    
    directory->element_indx = element_indx;
    directory->element_name = element_name;
    directory->element_desc = element_desc;
    directory->element_repr = element_repr;

    directory->is_composite = isElementComposite;
    directory->segment_type = segment_type;

    directory->codelist_name = codelist_name;

    directory->segment_reqr = getSegmentContentReqr;
    directory->composite_reqr = getCompositeContentReqr;
    directory->free = edi_francesco_free;

    directory->user_data = self;
  }
  
  return directory;
}

void edi_francesco_free(edi_directory_t * francesco)
{
  free(francesco);
}
