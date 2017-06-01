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

#ifndef PRMTRS_H
#define PRMTRS_H

/* This is intended to be a superset of envelope fields from all syntaxes */
typedef enum
{
  LastParameter = 0,
  MinParameter,

  /*Event,*/
  /*Function,*/
  /*Description,*/
  /*ServiceSegment,*/

  /* data element dictionary */
  Code,
  Name,
  Desc,
  Note,
  List,

  /* miscellaneous extras */
  Context,
  Standard,
  Element,
  Subelement,

  /* message envelope items */
  AcknowledgementRequest,
  AddressForReverseRouting,
  ApplicationPassword,
  ApplicationReference,
  AssociationAssignedCode,
  CommonAccessReference,
  CommunicationsAgreementID,
  ControllingAgency,
  Date,
  FirstAndLastTransfer,
  FreeFormText,
  FunctionalGroupId,
  FunctionalGroupReferenceNumber,
  InterchangeControlCount,
  InterchangeControlReference,
  MessageReferenceNumber,
  MessageReleaseNumber,
  MessageType,
  MessageVersionNumber,
  NumberOfMessages,
  NumberOfSegmentsInTheMessage,
  PartnerIdCodeQualifier,
  ProcessingPriorityCode,
  RecipientsId,
  RecipientsIdCodeQualifier,
  RecipientsName,
  RecipientsReference,
  RecipientsReferencePassword,
  RecipientsReferencePasswordQualifier,
  RoutingAddress,
  SectionId,
  SendersId,
  SendersIdCodeQualifier,
  SendersName,
  SendersReference,
  SequenceOfTransfers,
  ServiceStringAdvice,
  SyntaxIdentifier,
  SyntaxVersionNumber,
  TestIndicator,
  TextReferenceCode,
  TransactionCode,
  TransactionType,
  Time,

  /* service string advice */
  TagSeparator,
  SubelementSeparator,
  ElementSeparator,
  DecimalNotation,
  ReleaseIndicator,
  RepetitionSeparator,
  SegmentTerminator,


  MaxParameter
}
edi_parameter_t;


typedef struct edi_parameters_s
{
  const char *value[MaxParameter];
  unsigned int element, subelement;
}
edi_parameters_t;


void edi_parameters_set(edi_parameters_t *, ...);
void edi_parameters_set_one(edi_parameters_t *, edi_parameter_t, const char *);
const char *edi_parameters_get(edi_parameters_t *, edi_parameter_t);
const char *edi_parameters_get_string(edi_parameter_t);


#endif /*PRMTRS_H*/
