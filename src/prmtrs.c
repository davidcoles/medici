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
#include <stdarg.h>
#include <string.h>

#include "internal.h"

/** \file prmtrs.c
    
    \brief Access methods for the edi_parameters_t structure.

*/

/**
   \defgroup edi_parameters edi_parameters
   \{
*/


/**
   \brief Set edi_parameters_t fields to the list of key, value pairs.
   
   \param p Pointer to the edi_parameters_t structure to modify.
   \param ... List of key, value pairs and punctuated with a trailing
   LastParameter.
   
   Any existing edi_parameters_t fields are cleared and the fields
   are then defined to be the set listed. There should then be a
   trailing LastParameter argument to indicate to the va_list that
   the final parameter has been reached.
*/

void
edi_parameters_set (edi_parameters_t *p, ...)
{
  unsigned int key;
  va_list ap;

  if (!p)
    return;

  for (key = LastParameter; key < MaxParameter; key++)
    p->value[key] = NULL;

  va_start (ap, p);
  while ((key = va_arg (ap, edi_parameter_t)))
    p->value[key] = va_arg (ap, char *);
  va_end (ap);
}



/**
   \brief Set a single edi_parameters_t key to a value.
   
   \param p Pointer to the edi_parameters_t structure to modify.
   \param k Key to set.
   \param v Value to set the key to.
   
   The specified key in the edi_parameters_t structure will be set to
   the specified value. No other modification is made to the
   structure.
   
*/

void edi_parameters_set_one
(edi_parameters_t *p, edi_parameter_t k, const char *v)
{
  p->value[k] = v;
}



/**
   \brief Query the value of an edi_parameters_t key.
   
   \param p Pointer to the edi_parameters_t structure to query.
   \param k Key to query.
   \return Pointer to the value string or NULL if the parameter has not
   been set.
   
   The specified key in the edi_parameters_t structure will be set to
   the specified value. No other modification is made to the
   structure.
   
*/

const char *
edi_parameters_get (edi_parameters_t *p, edi_parameter_t k)
{
  return p->value[k];
}



/**
   \brief Obtains a string description of a parameter.
   
   \param p The parameter for which a description is required.
   \return Pointer to the string describing the parameter.

*/

const char *
edi_parameters_get_string (edi_parameter_t p)
{
  switch (p)
    {
    case LastParameter:
      return "LastParameter";

      /*case Event:
	return "Event";*/
      /*case Function:
	return "Function";*/

    case Context:
      return "Context";
    case Code:
      return "Code";
    case Name:
      return "Name";
    case Desc:
      return "Desc";
    case Note:
      return "Note";
    case List:
      return "List";
    case Standard:
      return "Standard";

    case Element:
      return "Element";
    case Subelement:
      return "Subelement";

    case MinParameter:
      return "MinParameter";
    case MaxParameter:
      return "MaxParameter";

    case AcknowledgementRequest:
      return "AcknowledgementRequest";
    case AddressForReverseRouting:
      return "AddressForReverseRouting";
    case ApplicationPassword:
      return "ApplicationPassword";
    case ApplicationReference:
      return "ApplicationReference";
    case AssociationAssignedCode:
      return "AssociationAssignedCode";
    case CommonAccessReference:
      return "CommonAccessReference";
    case CommunicationsAgreementID:
      return "CommunicationsAgreementID";
    case ControllingAgency:
      return "ControllingAgency";
    case Date:
      return "Date";
    case FirstAndLastTransfer:
      return "FirstAndLastTransfer";
    case FreeFormText:
      return "FreeFormText";
    case FunctionalGroupId:
      return "FunctionalGroupId";
    case FunctionalGroupReferenceNumber:
      return "FunctionalGroupReferenceNumber";
    case InterchangeControlCount:
      return "InterchangeControlCount";
    case InterchangeControlReference:
      return "InterchangeControlReference";
    case MessageReferenceNumber:
      return "MessageReferenceNumber";
    case MessageReleaseNumber:
      return "MessageReleaseNumber";
    case MessageType:
      return "MessageType";
    case MessageVersionNumber:
      return "MessageVersionNumber";
    case NumberOfMessages:
      return "NumberOfMessages";
    case NumberOfSegmentsInTheMessage:
      return "NumberOfSegmentsInTheMessage";
    case PartnerIdCodeQualifier:
      return "PartnerIdCodeQualifier";
    case ProcessingPriorityCode:
      return "ProcessingPriorityCode";
    case RecipientsId:
      return "RecipientsId";
    case RecipientsIdCodeQualifier:
      return "RecipientsIdCodeQualifier";
    case RecipientsName:
      return "RecipientsName";
    case RecipientsReference:
      return "RecipientsReference";
    case RecipientsReferencePassword:
      return "RecipientsReferencePassword";
    case RecipientsReferencePasswordQualifier:
      return "RecipientsReferencePasswordQualifier";
    case RoutingAddress:
      return "RoutingAddress";
    case SectionId:
      return "SectionId";
    case SendersId:
      return "SendersId";
    case SendersIdCodeQualifier:
      return "SendersIdCodeQualifier";
    case SendersName:
      return "SendersName";
    case SendersReference:
      return "SendersReference";
    case SequenceOfTransfers:
      return "SequenceOfTransfers";
    case ServiceStringAdvice:
      return "ServiceStringAdvice";
    case SyntaxIdentifier:
      return "SyntaxIdentifier";
    case SyntaxVersionNumber:
      return "SyntaxVersionNumber";
    case TestIndicator:
      return "TestIndicator";
    case TextReferenceCode:
      return "TextReferenceCode";
    case TransactionCode:
      return "TransactionCode:";
    case TransactionType:
      return "TransactionType";
    case Time:
      return "Time";


    case TagSeparator:
      return "TagSeparator";
    case SubelementSeparator:
      return "SubelementSeparator";
    case ElementSeparator:
      return "ElementSeparator";
    case DecimalNotation:
      return "DecimalNotation";
    case ReleaseIndicator:
      return "ReleaseIndicator";
    case RepetitionSeparator:
      return "RepetitionSeparator";
    case SegmentTerminator:
      return "SegmentTerminator";


      /*case ServiceSegment:
	return "ServiceSegment";*/
    }

  /* watch compiler warnings - if a case isn't handled it should complain */
  return "UndefinedParameterString";
}



/** \} */
