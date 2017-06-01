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
#include <stdarg.h>
#include <string.h>

#include "internal.h"
#include "frncsc.h"

static edi_directory_t *X12_V4011_SYSTEM(void);

#define self ((edi_x12_t *) SELF)


#define SERVICE SELF->service
#define MESSAGE SELF->message



typedef enum
{
  NONE = 0,
  ISA,
  GS,
  ST,
  SE,
  GE,
  IEA
}
edi_x12_code_t;

typedef struct
{
  char *string;
  edi_x12_code_t code;
}
edi_x12_table_t;

static edi_x12_table_t code_table[] = {
  {"ISA", ISA},
  {"GS", GS},
  {"ST", ST},
  {"SE", SE},
  {"GE", GE},
  {"IEA", IEA},
  {NULL, NONE}
};


static edi_x12_code_t
edi_x12_get_segment_code (edi_segment_t *segment)
{
  int n;

  if (!segment)
    return NONE;

  for (n = 0; code_table[n].string; n++)
    if (!edi_segment_cmp_code (segment, code_table[n].string))
      return code_table[n].code;
  return NONE;
}

static void set_params
(edi_parameters_t *p, edi_directory_t *d, edi_segment_t *s, ...)
{
  int key;
  va_list ap;
  char *element, *subelement, *value;

  if(!p)
    return;
  
  /*edi_parameters_set (p, Standard, "X12", LastParameter);*/
  edi_parameters_set (p, LastParameter);
  
  if (!d || !s)
    return;

  va_start (ap, s);
  while ((key = va_arg (ap, edi_parameter_t)))
    {
      if(key == LastParameter)
        break;

      element = va_arg (ap, char *);
      subelement = va_arg (ap, char *);
      
      value = edi_get_element_by_name (d, s, element, subelement);
      
      if (value && strlen (value))
        edi_parameters_set_one (p, (edi_parameter_t) key, value);

    }
  va_end (ap);
}




static void edi_x12_set_parameters
(edi_parser_t *SELF, edi_segment_t *segment, edi_parameters_t *parameters)
{
  char *transaction, *i14, *st329;
  int test = 0;

  switch (edi_x12_get_segment_code (segment))
    {
    case ISA:
      set_params (parameters, SERVICE, segment,
		  SendersId, "I06", NULL,
		  RecipientsId, "I07", NULL,
		  Date, "I08", NULL,
		  Time, "I09", NULL,
		  SyntaxVersionNumber, "I11", NULL,
		  SyntaxIdentifier, "I10", NULL,
		  InterchangeControlReference, "I12", NULL,
		  AcknowledgementRequest, "I13", NULL,
		  /*TestIndicator, "I14", NULL,*/
		  LastParameter);

      i14 = edi_get_element_by_name (SERVICE, segment, "I14", NULL);
      test = (i14 && !strcmp("T", i14)) ? 1 : 0;
      edi_parameters_set_one (parameters, TestIndicator, test ? "1" : "0");
      edi_parameters_set_one (parameters, Standard, "X12");
      break;
      
    case GS:
      set_params (parameters, SERVICE, segment,
		  FunctionalGroupId, "479", NULL,
		  SendersId, "142", NULL,
		  RecipientsId, "124", NULL,
		  Date, "373", NULL,
		  Time, "337", NULL,
		  FunctionalGroupReferenceNumber, "28", NULL,
		  ControllingAgency, "455", NULL,
		  MessageVersionNumber, "480", NULL,
		  LastParameter);
      break;
      
    case ST:
      transaction = edi_get_element_by_name(SERVICE, segment, "143", NULL);
      sprintf(self->tmp, "%04d", self->version);
      st329 = edi_get_element_by_name (SERVICE, segment, "329", NULL);
      edi_parameters_set (parameters,
			  MessageVersionNumber, "V",
			  MessageReleaseNumber, self->tmp,
			  MessageType, transaction,
			  MessageReferenceNumber, st329,
			  LastParameter);
      break;

    default:
      set_params (parameters, SERVICE, segment, LastParameter);
    }
}


static edi_error_t
edi_x12_segment (edi_parser_t *SELF)
{
  edi_parameters_t parameters;
  edi_segment_t *segment, *context;
  edi_x12_code_t segment_code, context_code;
  char *str1;
  
  segment = SELF->segment;
  context = edi_parser_peek_segment (SELF);
  segment_code = edi_x12_get_segment_code (segment);
  context_code = edi_x12_get_segment_code (context);
  
  edi_x12_set_parameters (SELF, segment, &parameters);

  /*
    if(segment_code)
    edi_parameters_set_one (&parameters, ServiceSegment, "Yes");
  */

  /* FIXME - handle nesting of ISA/GS/ST/SE/GE/IEA with FSA */
  /* FIXME - check transaction-set / group / interchange control numbers */
  /* FIXME - checks should be done in a separate fn before triggering events */

  switch (segment_code)
    {

    case ISA:
      if((str1 = edi_get_element_by_name(SERVICE, segment, "I11", NULL)))
	self->version = atoi (str1) * 10;
      edi_parser_handle_start (SELF, EDI_INTERCHANGE, &parameters);
      edi_parser_handle_segment (SELF, &parameters, SERVICE);
      self->transactions = 0;
      self->groups = 0;
      break;
      

    case GS:
      if((str1 = edi_get_element_by_name(SERVICE, segment, "480", NULL)))
	self->version = atoi (str1);
      edi_parser_handle_start (SELF, EDI_GROUP, &parameters);
      edi_parser_handle_segment (SELF, &parameters, SERVICE);
      self->transactions = 0;
      self->groups++;
      break;
      

    case ST:
      edi_parameters_set_one (&parameters, Standard, "X12");
      edi_parser_handle_start (SELF, EDI_TRANSACTION, &parameters);
      str1 = edi_get_element_by_name(SERVICE, segment, "143", NULL);
      MESSAGE = edi_parser_handle_directory (SELF, &parameters);
      /*edi_directory_head (MESSAGE, segment, &parameters, SELF, str1);*/
      edi_parser_transaction_head(SELF, segment, MESSAGE, str1);
      self->segments = 1;
      self->transactions++;
      break;
      

    case SE:
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) ||
	  atoi(str1) != ++self->segments)
	/*return*/ edi_parser_raise_error (SELF, EDI_ETTC);
      /*edi_directory_tail (MESSAGE, segment, &parameters, SELF);*/
      edi_parser_transaction_tail(SELF, segment, MESSAGE);
      edi_parser_handle_end (SELF, EDI_TRANSACTION, &parameters);
      break;
      
      
    case GE:
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) ||
	  atoi(str1) != self->transactions)
	/*return*/ edi_parser_raise_error (SELF, EDI_EGTC);
      edi_parser_handle_segment (SELF, &parameters, SERVICE);
      edi_parser_handle_end (SELF, EDI_GROUP, &parameters);
      break;
      
      
    case IEA:
      if (!(str1 = edi_segment_get_element (segment, 0, 0)) ||
	  atoi(str1) != self->groups)
	/*return*/ edi_parser_raise_error (SELF, EDI_EITC);
      edi_parser_handle_segment (SELF, &parameters, SERVICE);
      edi_parser_handle_end (SELF, EDI_INTERCHANGE, &parameters);
      SELF->done = 1;
      break;
      
      
    default:
      /*edi_directory_body (MESSAGE, segment, &parameters, SELF);*/
      edi_parser_transaction_body(SELF, segment, MESSAGE);
      self->segments++;
    }
  
  return EDI_ENONE;
}


static void
edi_x12_fini (edi_parser_t *SELF)
{
  
  if(MESSAGE)
    edi_directory_free (MESSAGE);
  MESSAGE = NULL;
  
  if(SERVICE)
    edi_directory_free (SERVICE);
  SERVICE = NULL;
}


edi_error_t
edi_x12_init (edi_parser_t *SELF)
{
  memset(self, 0, sizeof(edi_x12_t)); /* mitigate bugs */
  
  SERVICE = X12_V4011_SYSTEM ();
  MESSAGE = NULL;
  
  SELF->syntax_fini = edi_x12_fini;
  SELF->sgmnt_handler = edi_x12_segment;

  return EDI_ENONE;
}







static edi_francesco_cmpsite_info_t compositeinfo[] = {
  {NULL, NULL, NULL}
};

static edi_francesco_cmpsite_list_t compositecontents[] = {
  {NULL, NULL, 0}
};

static edi_francesco_trnsctn_rule_t transactionsetrule[] = {
  {NULL, NULL, NULL, NULL, 0, 0, 0 }
};

static edi_francesco_segment_list_t segmentcontents[] = {
  { "GE" , "97"  , 'M', EDI_ELEMENT },
  { "GE" , "28"  , 'M', EDI_ELEMENT },
  { "GS" , "479" , 'M', EDI_ELEMENT },
  { "GS" , "142" , 'M', EDI_ELEMENT },
  { "GS" , "124" , 'M', EDI_ELEMENT },
  { "GS" , "373" , 'M', EDI_ELEMENT },
  { "GS" , "337" , 'M', EDI_ELEMENT },
  { "GS" , "28"  , 'M', EDI_ELEMENT },
  { "GS" , "455" , 'M', EDI_ELEMENT },
  { "GS" , "480" , 'M', EDI_ELEMENT },
  { "IEA", "I16" , 'M', EDI_ELEMENT },
  { "IEA", "I12" , 'M', EDI_ELEMENT },
  { "ISA", "I01" , 'M', EDI_ELEMENT },
  { "ISA", "I02" , 'M', EDI_ELEMENT },
  { "ISA", "I03" , 'M', EDI_ELEMENT },
  { "ISA", "I04" , 'M', EDI_ELEMENT },
  { "ISA", "I05" , 'M', EDI_ELEMENT },
  { "ISA", "I06" , 'M', EDI_ELEMENT },
  { "ISA", "I05" , 'M', EDI_ELEMENT },
  { "ISA", "I07" , 'M', EDI_ELEMENT },
  { "ISA", "I08" , 'M', EDI_ELEMENT },
  { "ISA", "I09" , 'M', EDI_ELEMENT },
  { "ISA", "I10" , 'M', EDI_ELEMENT },
  { "ISA", "I11" , 'M', EDI_ELEMENT },
  { "ISA", "I12" , 'M', EDI_ELEMENT },
  { "ISA", "I13" , 'M', EDI_ELEMENT },
  { "ISA", "I14" , 'M', EDI_ELEMENT },
  { "ISA", "I15" , 'M', EDI_ELEMENT },
  { "SE" , "96"  , 'M', EDI_ELEMENT },
  { "SE" , "329" , 'M', EDI_ELEMENT },
  { "ST" , "143" , 'M', EDI_ELEMENT },
  { "ST" , "329" , 'M', EDI_ELEMENT },
  {NULL, NULL, 0, 0}
};

static edi_francesco_segment_info_t segmentinfo[] = {
  { "GE" , "Functional Group Trailer", NULL },
  { "GS" , "Functional Group Header", NULL },
  { "IEA", "Interchange Control Trailer", NULL },
  { "ISA", "Interchange Control Header", NULL },
  { "SE" , "Transaction Set Trailer", NULL },
  { "ST" , "Transaction Set Header", NULL },
  {NULL, NULL, NULL}
};

static edi_francesco_element_info_t elementinfo[] = {
  { "I01", EDI_ENUMLIST,  2,  2, "Authorization Information Qualifier", NULL },
  { "I02", EDI_ISO2382X, 10, 10, "Authorization Information", NULL },
  { "I03", EDI_ENUMLIST,  2,  2, "Security Information Qualifier", NULL },
  { "I04", EDI_ISO2382X, 10, 10, "Security Information", NULL },
  { "I05", EDI_ENUMLIST,  2,  2, "Interchange ID Qualifier", NULL },
  { "I06", EDI_ISO2382X, 15, 15, "Interchange Sender ID", NULL },
  { "I07", EDI_ISO2382X, 15, 15, "Interchange Receiver ID", NULL },
  { "I08", EDI_DATESPEC,  6,  6, "Interchange Date", NULL },
  { "I09", EDI_TIMESPEC,  4,  4, "Interchange Time", NULL },
  { "I10", EDI_ENUMLIST,  1,  1, "Interchange Control Standards Identifier", NULL },
  { "I11", EDI_ENUMLIST,  5,  5, "Interchange Control Version Number", NULL },
  { "I12", EDI_ISO2382N,  9,  9, "Interchange Control Number", NULL },
  { "I13", EDI_ENUMLIST,  1,  1, "Acknowledgment Requested", NULL },
  { "I14", EDI_ENUMLIST,  1,  1, "Usage Indicator", NULL },
  { "I15", EDI_ENUMLIST,  1,  1, "Component Element Separator", NULL },
  { "I16", EDI_ISO2382N,  1,  5, "Number of Included Functional Groups", NULL },
  {  "28", EDI_ISO2382N,  1,  9, "Group Control Number", NULL },
  {  "96", EDI_ISO2382N,  1, 10, "Number of Included Segments", NULL },
  {  "97", EDI_ISO2382N,  1,  6, "Number of Transaction Sets Included", NULL },
  { "124", EDI_ISO2382X,  2, 15, "Application Receiver's Code", NULL },
  { "142", EDI_ISO2382X,  2, 15, "Application Sender's Code", NULL },
  { "143", EDI_ENUMLIST,  3,  3, "Transaction Set Identifier Code", NULL },
  { "329", EDI_ISO2382X,  4,  9, "Transaction Set Control Number", NULL },
  { "337", EDI_TIMESPEC,  4,  8, "Time", NULL },
  { "373", EDI_DATESPEC,  8,  8, "Date", NULL },
  { "455", EDI_ENUMLIST,  1,  2, "Responsible Agency Code", NULL },
  { "479", EDI_ENUMLIST,  2,  2, "Functional Identifier Code", NULL },
  { "480", EDI_ISO2382X,  1, 12, "Version / Release / Industry Identifier Code", NULL },
  {NULL, 0, 0, 0, NULL, NULL}
};

static edi_francesco_codelst_info_t codelistinfo[] = {
  { "I14", "I"  , "Information", NULL },
  { "I14", "P"  , "Production Data", NULL },
  { "I14", "T"  , "Test Data", NULL },
  { "I13", "0"  , "No Acknowledgment Requested", NULL },
  { "I13", "1"  , "Interchange Acknowledgment Requested", NULL },
  { "I11", "00200", "Standard Issued as ANSI X12.5-1987", NULL },
  { "I11", "00201", "Draft Standard for Trial Use Approved by ASC X12 Through August 1988", NULL },
  { "I11", "00204", "Draft Standard for Trial Use Approved by ASC X12 Through May 1989", NULL },
  { "I11", "00300", "Standard Issued as ANSI X12.5-1992", NULL },
  { "I11", "00301", "Draft Standard for Trial Use Approved for Publication by ASC X12 Procedures Review Board Through October 1990", NULL },
  { "I11", "00302", "Draft Standard for Trial Use Approved for Publication by ASC X12 Procedures Review Board Through October 1991", NULL },
  { "I11", "00303", "Draft Standard for Trial Use Approved for Publication by ASC X12 Procedures Review Board Through October 1992", NULL },
  { "I11", "00304", "Draft Standards for Trial Use Approved for Publication by ASC X12 Procedures Review Board through October 1993", NULL },
  { "I11", "00305", "Draft Standards for Trial Use Approved for Publication by ASC X12 Procedures Review Board through October 1994", NULL },
  { "I11", "00306", "Draft Standards for Trial Use Approved for Publication by ASC X12 Procedures Review Board through October 1995", NULL },
  { "I11", "00307", "Draft Standards for Trial Use Approved for Publication by ASC X12 Procedures Review Board through October 1996", NULL },
  { "I11", "00400", "Standard Issued as ANSI X12.5-1997", NULL },
  { "I11", "00401", "Draft Standards for Trial Use Approved for Publication by ASC X12 Procedures Review Board through October 1997", NULL },
  { "I11", "00402", "Draft Standards for Trial Use Approved for Publication by ASC X12 Procedures Review Board through October 1998", NULL },
  { "I10", "U"  , "U.S. EDI Community of ASC X12, TDCC, and UCS", NULL },
  { "I03", "00" , "No Security Information Present (No Meaningful Information in I04)", NULL },
  { "I03", "01" , "Password", NULL },
  { "I01", "00" , "No Authorization Information Present (No Meaningful Information in I02)", NULL },
  { "I01", "01" , "UCS Communications ID", NULL },
  { "I01", "02" , "EDX Communications ID", NULL },
  { "I01", "03" , "Additional Data Identification", NULL },
  { "I01", "04" , "Rail Communications ID", NULL },
  { "I01", "05" , "Department of Defense (DoD) Communication Identifier", NULL },
  { "I01", "06" , "United States Federal Government Communication Identifier", NULL },
  { "I05", "01" , "Duns (Dun + Bradstreet)", NULL },
  { "I05", "02" , "SCAC (Standard Carrier Alpha Code)", NULL },
  { "I05", "03" , "FMC (Federal Maritime Commission)", NULL },
  { "I05", "04" , "IATA (International Air Transport Association)", NULL },
  { "I05", "08" , "UCC EDI Communications ID (Comm ID)", NULL },
  { "I05", "09" , "X.121 (CCITT)", NULL },
  { "I05", "10" , "Department of Defense (DoD) Activity Address Code", NULL },
  { "I05", "11" , "DEA (Drug Enforcement Administration)", NULL },
  { "I05", "12" , "Phone (Telephone Companies)", NULL },
  { "I05", "13" , "UCS Code (The UCS Code is a Code Used for UCS Transmissions; it includes the Area Code and Telephone Number of a Modem; it Does Not Include Punctuation, Blanks or Access Code)", NULL },
  { "I05", "14" , "Duns Plus Suffix", NULL },
  { "I05", "15" , "Petroleum Accountants Society of Canada Company Code", NULL },
  { "I05", "16" , "Duns Number With 4-Character Suffix", NULL },
  { "I05", "17" , "American Bankers Association (ABA) Transit Routing Number (Including Check Digit, 9 Digit)", NULL },
  { "I05", "18" , "Association of American Railroads (AAR) Standard Distribution Code", NULL },
  { "I05", "19" , "EDI Council of Australia (EDICA) Communications ID Number (COMM ID)", NULL },
  { "I05", "20" , "Health Industry Number (HIN)", NULL },
  { "I05", "21" , "Integrated Postsecondary Education Data System, or (IPEDS)", NULL },
  { "I05", "22" , "Federal Interagency Commission on Education, or FICE", NULL },
  { "I05", "23" , "National Center for Education Statistics Common Core of Data 12-Digit Number for Pre-K-Grade 12 Institutes, or NCES", NULL },
  { "I05", "24" , "The College Board's Admission Testing Program 4-Digit Code of Postsecondary Institutes, or ATP", NULL },
  { "I05", "25" , "American College Testing Program 4-Digit Code of Postsecondary Institutions, or ACT", NULL },
  { "I05", "26" , "Statistics of Canada List of Postsecondary Institutions", NULL },
  { "I05", "27" , "Carrier Identification Number as assigned by Health Care Financing Administration (HCFA)", NULL },
  { "I05", "28" , "Fiscal Intermediary Identification Number as assigned by Health Care Financing Administration (HCFA)", NULL },
  { "I05", "29" , "Medicare Provider and Supplier Identification Number as assigned by Health Care Financing Administration (HCFA)", NULL },
  { "I05", "30" , "U.S. Federal Tax Identification Number", NULL },
  { "I05", "31" , "Jurisdiction Identification Number Plus 4 as assigned by the International Association of Industrial Accident Boards and Commissions (IAIABC)", NULL },
  { "I05", "32" , "U.S. Federal Employer Identification Number (FEIN)", NULL },
  { "I05", "33" , "National Association of Insurance Commissioners Company Code (NAIC)", NULL },
  { "I05", "34" , "Medicaid Provider and Supplier Identification Number as assigned by individual State Medicaid Agencies in conjunction with Health Care Financing Administration (HCFA)", NULL },
  { "I05", "35" , "Statistics Canada Canadian College Student Information System Institution Codes", NULL },
  { "I05", "36" , "Statistics Canada University Student Information System Institution Codes", NULL },
  { "I05", "37" , "Society of Property Information Compilers and Analysts", NULL },
  { "I05", "AM" , "Association Mexicana del Codigo de Producto (AMECOP) Communication ID", NULL },
  { "I05", "NR" , "National Retail Merchants Association (NRMA) - Assigned", NULL },
  { "I05", "SN" , "Standard Address Number", NULL },
  { "I05", "ZZ" , "Mutually Defined", NULL },	 
  { "143", "100", "Insurance Plan Description", NULL },
  { "143", "101", "Name and Address Lists", NULL },
  { "143", "102", "Associated Data", NULL },
  { "143", "103", "Abandoned Property Filings", NULL },
  { "143", "104", "Air Shipment Information", NULL },
  { "143", "105", "Business Entity Filings", NULL },
  { "143", "106", "Motor Carrier Rate Proposal", NULL },
  { "143", "107", "Request for Motor Carrier Rate Proposal", NULL },
  { "143", "108", "Response to a Motor Carrier Rate Proposal", NULL },
  { "143", "109", "Vessel Content Details", NULL },
  { "143", "110", "Air Freight Details and Invoice", NULL },
  { "143", "111", "Individual Insurance Policy and Client Information", NULL },
  { "143", "112", "Property Damage Report", NULL },
  { "143", "120", "Vehicle Shipping Order", NULL },
  { "143", "121", "Vehicle Service", NULL },
  { "143", "124", "Vehicle Damage", NULL },
  { "143", "125", "Multilevel Railcar Load Details", NULL },
  { "143", "126", "Vehicle Application Advice", NULL },
  { "143", "127", "Vehicle Baying Order", NULL },
  { "143", "128", "Dealer Information", NULL },
  { "143", "129", "Vehicle Carrier Rate Update", NULL },
  { "143", "130", "Student Educational Record (Transcript)", NULL },
  { "143", "131", "Student Educational Record (Transcript) Acknowledgment", NULL },
  { "143", "135", "Student Loan Application", NULL },
  { "143", "138", "Education Testing Results Request and Report", NULL },
  { "143", "139", "Student Loan Guarantee Result", NULL },
  { "143", "140", "Product Registration", NULL },
  { "143", "141", "Product Service Claim Response", NULL },
  { "143", "142", "Product Service Claim", NULL },
  { "143", "143", "Product Service Notification", NULL },
  { "143", "144", "Student Loan Transfer and Status Verification", NULL },
  { "143", "146", "Request for Student Educational Record (Transcript)", NULL },
  { "143", "147", "Response to Request for Student Educational Record (Transcript)", NULL },
  { "143", "148", "Report of Injury, Illness or Incident", NULL },
  { "143", "149", "Notice of Tax Adjustment or Assessment", NULL },
  { "143", "150", "Tax Rate Notification", NULL },
  { "143", "151", "Electronic Filing of Tax Return Data Acknowledgment", NULL },
  { "143", "152", "Statistical Government Information", NULL },
  { "143", "153", "Unemployment Insurance Tax Claim or Charge Information", NULL },
  { "143", "154", "Uniform Commercial Code Filing", NULL },
  { "143", "155", "Business Credit Report", NULL },
  { "143", "157", "Notice of Power of Attorney", NULL },
  { "143", "159", "Motion Picture Booking Confirmation", NULL },
  { "143", "160", "Transportation Automatic Equipment Identification", NULL },
  { "143", "161", "Train Sheet", NULL },
  { "143", "163", "Transportation Appointment Schedule Information", NULL },
  { "143", "170", "Revenue Receipts Statement", NULL },
  { "143", "175", "Court and Law Enforcement Notice", NULL },
  { "143", "176", "Court Submission", NULL },
  { "143", "180", "Return Merchandise Authorization and Notification", NULL },
  { "143", "185", "Royalty Regulatory Report", NULL },
  { "143", "186", "Insurance Underwriting Requirements Reporting", NULL },
  { "143", "188", "Educational Course Inventory", NULL },
  { "143", "189", "Application for Admission to Educational Institutions", NULL },
  { "143", "190", "Student Enrollment Verification", NULL },
  { "143", "191", "Student Loan Pre-Claims and Claims", NULL },
  { "143", "194", "Grant or Assistance Application", NULL },
  { "143", "195", "Federal Communications Commission (FCC) License Application", NULL },
  { "143", "196", "Contractor Cost Data Reporting", NULL },
  { "143", "197", "Real Estate Title Evidence", NULL },
  { "143", "198", "Loan Verification Information", NULL },
  { "143", "199", "Real Estate Settlement Information", NULL },
  { "143", "200", "Mortgage Credit Report", NULL },
  { "143", "201", "Residential Loan Application", NULL },
  { "143", "202", "Secondary Mortgage Market Loan Delivery", NULL },
  { "143", "203", "Secondary Mortgage Market Investor Report", NULL },
  { "143", "204", "Motor Carrier Load Tender", NULL },
  { "143", "205", "Mortgage Note", NULL },
  { "143", "206", "Real Estate Inspection", NULL },
  { "143", "210", "Motor Carrier Freight Details and Invoice", NULL },
  { "143", "211", "Motor Carrier Bill of Lading", NULL },
  { "143", "212", "Motor Carrier Delivery Trailer Manifest", NULL },
  { "143", "213", "Motor Carrier Shipment Status Inquiry", NULL },
  { "143", "214", "Transportation Carrier Shipment Status Message", NULL },
  { "143", "215", "Motor Carrier Pick-up Manifest", NULL },
  { "143", "216", "Motor Carrier Shipment Pick-up Notification", NULL },
  { "143", "217", "Motor Carrier Loading and Route Guide", NULL },
  { "143", "218", "Motor Carrier Tariff Information", NULL },
  { "143", "219", "Logistics Service Request", NULL },
  { "143", "220", "Logistics Service Response", NULL },
  { "143", "222", "Cartage Work Assignment", NULL },
  { "143", "223", "Consolidators Freight Bill and Invoice", NULL },
  { "143", "224", "Motor Carrier Summary Freight Bill Manifest", NULL },
  { "143", "225", "Response to a Cartage Work Assignment", NULL },
  { "143", "240", "Motor Carrier Package Status", NULL },
  { "143", "242", "Data Status Tracking", NULL },
  { "143", "244", "Product Source Information", NULL },
  { "143", "248", "Account Assignment/Inquiry and Service/Status", NULL },
  { "143", "249", "Animal Toxicological Data", NULL },
  { "143", "250", "Purchase Order Shipment Management Document", NULL },
  { "143", "251", "Pricing Support", NULL },
  { "143", "252", "Insurance Producer Administration", NULL },
  { "143", "255", "Underwriting Information Services", NULL },
  { "143", "256", "Periodic Compensation", NULL },
  { "143", "260", "Application for Mortgage Insurance Benefits", NULL },
  { "143", "261", "Real Estate Information Request", NULL },
  { "143", "262", "Real Estate Information Report", NULL },
  { "143", "263", "Residential Mortgage Insurance Application Response", NULL },
  { "143", "264", "Mortgage Loan Default Status", NULL },
  { "143", "265", "Real Estate Title Insurance Services Order", NULL },
  { "143", "266", "Mortgage or Property Record Change Notification", NULL },
  { "143", "267", "Individual Life, Annuity and Disability Application", NULL },
  { "143", "268", "Annuity Activity", NULL },
  { "143", "270", "Eligibility, Coverage or Benefit Inquiry", NULL },
  { "143", "271", "Eligibility, Coverage or Benefit Information", NULL },
  { "143", "272", "Property and Casualty Loss Notification", NULL },
  { "143", "273", "Insurance/Annuity Application Status", NULL },
  { "143", "275", "Patient Information", NULL },
  { "143", "276", "Health Care Claim Status Request", NULL },
  { "143", "277", "Health Care Claim Status Notification", NULL },
  { "143", "278", "Health Care Services Review Information", NULL },
  { "143", "280", "Voter Registration Information", NULL },
  { "143", "285", "Commercial Vehicle Safety and Credentials Information Exchange", NULL },
  { "143", "286", "Commercial Vehicle Credentials", NULL },
  { "143", "288", "Wage Determination", NULL },
  { "143", "290", "Cooperative Advertising Agreements", NULL },
  { "143", "300", "Reservation (Booking Request) (Ocean)", NULL },
  { "143", "301", "Confirmation (Ocean)", NULL },
  { "143", "303", "Booking Cancellation (Ocean)", NULL },
  { "143", "304", "Shipping Instructions", NULL },
  { "143", "306", "Dock Receipt", NULL },
  { "143", "309", "Customs Manifest", NULL },
  { "143", "310", "Freight Receipt and Invoice (Ocean)", NULL },
  { "143", "311", "Canadian Customs Information", NULL },
  { "143", "312", "Arrival Notice (Ocean)", NULL },
  { "143", "313", "Shipment Status Inquiry (Ocean)", NULL },
  { "143", "315", "Status Details (Ocean)", NULL },
  { "143", "317", "Delivery/Pickup Order", NULL },
  { "143", "319", "Terminal Information", NULL },
  { "143", "321", "Demurrage Guarantee (Ocean)", NULL },
  { "143", "322", "Terminal Operations and Intermodal Ramp Activity", NULL },
  { "143", "323", "Vessel Schedule and Itinerary (Ocean)", NULL },
  { "143", "324", "Vessel Stow Plan (Ocean)", NULL },
  { "143", "325", "Consolidation of Goods in Container", NULL },
  { "143", "326", "Consignment Summary List", NULL },
  { "143", "350", "Customs Status Information", NULL },
  { "143", "352", "U.S. Customs Carrier General Order Status", NULL },
  { "143", "353", "Customs Events Advisory Details", NULL },
  { "143", "354", "U.S. Customs Automated Manifest Archive Status", NULL },
  { "143", "355", "U.S. Customs Acceptance/Rejection", NULL },
  { "143", "356", "U.S. Customs Permit to Transfer Request", NULL },
  { "143", "357", "U.S. Customs In-Bond Information", NULL },
  { "143", "358", "Customs Consist Information", NULL },
  { "143", "361", "Carrier Interchange Agreement (Ocean)", NULL },
  { "143", "362", "Cargo Insurance Advice of Shipment", NULL },
  { "143", "404", "Rail Carrier Shipment Information", NULL },
  { "143", "410", "Rail Carrier Freight Details and Invoice", NULL },
  { "143", "411", "Freight Details and Invoice Summary (Rail)", NULL },
  { "143", "414", "Rail Carhire Settlements", NULL },
  { "143", "417", "Rail Carrier Waybill Interchange", NULL },
  { "143", "418", "Rail Advance Interchange Consist", NULL },
  { "143", "419", "Advance Car Disposition", NULL },
  { "143", "420", "Car Handling Information", NULL },
  { "143", "421", "Estimated Time of Arrival and Car Scheduling", NULL },
  { "143", "422", "Shipper's Car Order", NULL },
  { "143", "423", "Rail Industrial Switch List", NULL },
  { "143", "425", "Rail Waybill Request", NULL },
  { "143", "426", "Rail Revenue Waybill", NULL },
  { "143", "429", "Railroad Retirement Activity", NULL },
  { "143", "431", "Railroad Station Master File", NULL },
  { "143", "432", "Rail Deprescription", NULL },
  { "143", "433", "Railroad Reciprocal Switch File", NULL },
  { "143", "434", "Railroad Mark Register Update Activity", NULL },
  { "143", "435", "Standard Transportation Commodity Code Master", NULL },
  { "143", "436", "Locomotive Information", NULL },
  { "143", "437", "Railroad Junctions and Interchanges Activity", NULL },
  { "143", "440", "Shipment Weights", NULL },
  { "143", "451", "Railroad Event Report", NULL },
  { "143", "452", "Railroad Problem Log Inquiry or Advice", NULL },
  { "143", "453", "Railroad Service Commitment Advice", NULL },
  { "143", "455", "Railroad Parameter Trace Registration", NULL },
  { "143", "456", "Railroad Equipment Inquiry or Advice", NULL },
  { "143", "460", "Railroad Price Distribution Request or Response", NULL },
  { "143", "463", "Rail Rate Reply", NULL },
  { "143", "466", "Rate Request", NULL },
  { "143", "468", "Rate Docket Journal Log", NULL },
  { "143", "470", "Railroad Clearance", NULL },
  { "143", "475", "Rail Route File Maintenance", NULL },
  { "143", "485", "Ratemaking Action", NULL },
  { "143", "486", "Rate Docket Expiration", NULL },
  { "143", "490", "Rate Group Definition", NULL },
  { "143", "492", "Miscellaneous Rates", NULL },
  { "143", "494", "Scale Rate Table", NULL },
  { "143", "500", "Medical Event Reporting", NULL },
  { "143", "501", "Vendor Performance Review", NULL },
  { "143", "503", "Pricing History", NULL },
  { "143", "504", "Clauses and Provisions", NULL },
  { "143", "511", "Requisition", NULL },
  { "143", "517", "Material Obligation Validation", NULL },
  { "143", "521", "Income or Asset Offset", NULL },
  { "143", "527", "Material Due-In and Receipt", NULL },
  { "143", "536", "Logistics Reassignment", NULL },
  { "143", "540", "Notice of Employment Status", NULL },
  { "143", "561", "Contract Abstract", NULL },
  { "143", "567", "Contract Completion Status", NULL },
  { "143", "568", "Contract Payment Management Report", NULL },
  { "143", "601", "U.S. Customs Export Shipment Information", NULL },
  { "143", "602", "Transportation Services Tender", NULL },
  { "143", "620", "Excavation Communication", NULL },
  { "143", "622", "Intermodal Ramp Activity", NULL },
  { "143", "625", "Well Information", NULL },
  { "143", "650", "Maintenance Service Order", NULL },
  { "143", "715", "Intermodal Group Loading Plan", NULL },
  { "143", "805", "Contract Pricing Proposal", NULL },
  { "143", "806", "Project Schedule Reporting", NULL },
  { "143", "810", "Invoice", NULL },
  { "143", "811", "Consolidated Service Invoice/Statement", NULL },
  { "143", "812", "Credit/Debit Adjustment", NULL },
  { "143", "813", "Electronic Filing of Tax Return Data", NULL },
  { "143", "814", "General Request, Response or Confirmation", NULL },
  { "143", "815", "Cryptographic Service Message", NULL },
  { "143", "816", "Organizational Relationships", NULL },
  { "143", "818", "Commission Sales Report", NULL },
  { "143", "819", "Operating Expense Statement", NULL },
  { "143", "820", "Payment Order/Remittance Advice", NULL },
  { "143", "821", "Financial Information Reporting", NULL },
  { "143", "822", "Account Analysis", NULL },
  { "143", "823", "Lockbox", NULL },
  { "143", "824", "Application Advice", NULL },
  { "143", "826", "Tax Information Exchange", NULL },
  { "143", "827", "Financial Return Notice", NULL },
  { "143", "828", "Debit Authorization", NULL },
  { "143", "829", "Payment Cancellation Request", NULL },
  { "143", "830", "Planning Schedule with Release Capability", NULL },
  { "143", "831", "Application Control Totals", NULL },
  { "143", "832", "Price/Sales Catalog", NULL },
  { "143", "833", "Mortgage Credit Report Order", NULL },
  { "143", "834", "Benefit Enrollment and Maintenance", NULL },
  { "143", "835", "Health Care Claim Payment/Advice", NULL },
  { "143", "836", "Procurement Notices", NULL },
  { "143", "837", "Health Care Claim", NULL },
  { "143", "838", "Trading Partner Profile", NULL },
  { "143", "839", "Project Cost Reporting", NULL },
  { "143", "840", "Request for Quotation", NULL },
  { "143", "841", "Specifications/Technical Information", NULL },
  { "143", "842", "Nonconformance Report", NULL },
  { "143", "843", "Response to Request for Quotation", NULL },
  { "143", "844", "Product Transfer Account Adjustment", NULL },
  { "143", "845", "Price Authorization Acknowledgment/Status", NULL },
  { "143", "846", "Inventory Inquiry/Advice", NULL },
  { "143", "847", "Material Claim", NULL },
  { "143", "848", "Material Safety Data Sheet", NULL },
  { "143", "849", "Response to Product Transfer Account Adjustment", NULL },
  { "143", "850", "Purchase Order", NULL },
  { "143", "851", "Asset Schedule", NULL },
  { "143", "852", "Product Activity Data", NULL },
  { "143", "853", "Routing and Carrier Instruction", NULL },
  { "143", "854", "Shipment Delivery Discrepancy Information", NULL },
  { "143", "855", "Purchase Order Acknowledgment", NULL },
  { "143", "856", "Ship Notice/Manifest", NULL },
  { "143", "857", "Shipment and Billing Notice", NULL },
  { "143", "858", "Shipment Information", NULL },
  { "143", "859", "Freight Invoice", NULL },
  { "143", "860", "Purchase Order Change Request - Buyer Initiated", NULL },
  { "143", "861", "Receiving Advice/Acceptance Certificate", NULL },
  { "143", "862", "Shipping Schedule", NULL },
  { "143", "863", "Report of Test Results", NULL },
  { "143", "864", "Text Message", NULL },
  { "143", "865", "Purchase Order Change Acknowledgment/Request - Seller Initiated", NULL },
  { "143", "866", "Production Sequence", NULL },
  { "143", "867", "Product Transfer and Resale Report", NULL },
  { "143", "868", "Electronic Form Structure", NULL },
  { "143", "869", "Order Status Inquiry", NULL },
  { "143", "870", "Order Status Report", NULL },
  { "143", "871", "Component Parts Content", NULL },
  { "143", "872", "Residential Mortgage Insurance Application", NULL },
  { "143", "875", "Grocery Products Purchase Order", NULL },
  { "143", "876", "Grocery Products Purchase Order Change", NULL },
  { "143", "877", "Manufacturer Coupon Family Code Structure", NULL },
  { "143", "878", "Product Authorization/De-Authorization", NULL },
  { "143", "879", "Price Information", NULL },
  { "143", "880", "Grocery Products Invoice", NULL },
  { "143", "881", "Manufacturer Coupon Redemption Detail", NULL },
  { "143", "882", "Direct Store Delivery Summary Information", NULL },
  { "143", "883", "Market Development Fund Allocation", NULL },
  { "143", "884", "Market Development Fund Settlement", NULL },
  { "143", "885", "Retail Account Characteristics", NULL },
  { "143", "886", "Customer Call Reporting", NULL },
  { "143", "887", "Coupon Notification", NULL },
  { "143", "888", "Item Maintenance", NULL },
  { "143", "889", "Promotion Announcement", NULL },
  { "143", "891", "Deduction Research Report", NULL },
  { "143", "893", "Item Information Request", NULL },
  { "143", "894", "Delivery/Return Base Record", NULL },
  { "143", "895", "Delivery/Return Acknowledgment or Adjustment", NULL },
  { "143", "896", "Product Dimension Maintenance", NULL },
  { "143", "920", "Loss or Damage Claim - General Commodities", NULL },
  { "143", "924", "Loss or Damage Claim - Motor Vehicle", NULL },
  { "143", "925", "Claim Tracer", NULL },
  { "143", "926", "Claim Status Report and Tracer Reply", NULL },
  { "143", "928", "Automotive Inspection Detail", NULL },
  { "143", "940", "Warehouse Shipping Order", NULL },
  { "143", "943", "Warehouse Stock Transfer Shipment Advice", NULL },
  { "143", "944", "Warehouse Stock Transfer Receipt Advice", NULL },
  { "143", "945", "Warehouse Shipping Advice", NULL },
  { "143", "947", "Warehouse Inventory Adjustment Advice", NULL },
  { "143", "980", "Functional Group Totals", NULL },
  { "143", "990", "Response To a Load Tender", NULL },
  { "143", "994", "Administrative Message", NULL },
  { "143", "996", "File Transfer", NULL },
  { "143", "997", "Functional Acknowledgment", NULL },
  { "143", "998", "Set Cancellation", NULL },
  { "455", "T"  , "Transportation Data Coordinating Committee (TDCC)", NULL },
  { "455", "X"  , "Accredited Standards Committee X12", NULL },
  { "479", "AA" , "Account Analysis (822)", NULL },
  { "479", "AB" , "Logistics Service Request (219)", NULL },
  { "479", "AC" , "Associated Data (102)", NULL },
  { "479", "AD" , "Individual Life, Annuity and Disability Application (267)", NULL },
  { "479", "AF" , "Application for Admission to Educational Institutions (189)", NULL },
  { "479", "AG" , "Application Advice (824)", NULL },
  { "479", "AH" , "Logistics Service Response (220)", NULL },
  { "479", "AI" , "Automotive Inspection Detail (928)", NULL },
  { "479", "AK" , "Student Educational Record (Transcript) Acknowledgment (131)", NULL },
  { "479", "AL" , "Set Cancellation (998) and Application Acceptance/Rejection Advice (499)", NULL },
  { "479", "AN" , "Return Merchandise Authorization and Notification (180)", NULL },
  { "479", "AO" , "Income or Asset Offset (521)", NULL },
  { "479", "AP" , "Abandoned Property Filings (103)", NULL },
  { "479", "AR" , "Warehouse Stock Transfer Shipment Advice (943)", NULL },
  { "479", "AS" , "Transportation Appointment Schedule Information (163)", NULL },
  { "479", "AT" , "Animal Toxicological Data (249)", NULL },
  { "479", "AW" , "Warehouse Inventory Adjustment Advice (947)", NULL },
  { "479", "BC" , "Business Credit Report (155)", NULL },
  { "479", "BE" , "Benefit Enrollment and Maintenance (834)", NULL },
  { "479", "BF" , "Business Entity Filings (105)", NULL },
  { "479", "BL" , "Motor Carrier Bill of Lading (211)", NULL },
  { "479", "BS" , "Shipment and Billing Notice (857)", NULL },
  { "479", "CA" , "Purchase Order Change Acknowledgment/Request - Seller Initiated (865)", NULL },
  { "479", "CB" , "Unemployment Insurance Tax Claim or Charge Information (153)", NULL },
  { "479", "CC" , "Clauses and Provisions (504)", NULL },
  { "479", "CD" , "Credit/Debit Adjustment (812)", NULL },
  { "479", "CE" , "Cartage Work Assignment (222)", NULL },
  { "479", "CF" , "Corporate Financial Adjustment Information (844 and 849)", NULL },
  { "479", "CG" , "Administrative Message (994)", NULL },
  { "479", "CH" , "Car Handling Information (420)", NULL },
  { "479", "CI" , "Consolidated Service Invoice/Statement (811)", NULL },
  { "479", "CJ" , "Manufacturer Coupon Family Code Structure (877)", NULL },
  { "479", "CK" , "Manufacturer Coupon Redemption Detail (881)", NULL },
  { "479", "CM" , "Component Parts Content (871)", NULL },
  { "479", "CN" , "Coupon Notification (887)", NULL },
  { "479", "CO" , "Cooperative Advertising Agreements (290)", NULL },
  { "479", "CP" , "Electronic Proposal Information (251, 805)", NULL },
  { "479", "CR" , "Rail Carhire Settlements (414)", NULL },
  { "479", "CS" , "Cryptographic Service Message (815)", NULL },
  { "479", "CT" , "Application Control Totals (831)", NULL },
  { "479", "CV" , "Commercial Vehicle Safety and Credentials Information Exchange (285)", NULL },
  { "479", "D3" , "Contract Completion Status (567)", NULL },
  { "479", "D4" , "Contract Abstract (561)", NULL },
  { "479", "D5" , "Contract Payment Management Report (568)", NULL },
  { "479", "DA" , "Debit Authorization (828)", NULL },
  { "479", "DD" , "Shipment Delivery Discrepancy Information (854)", NULL },
  { "479", "DF" , "Market Development Fund Allocation (883)", NULL },
  { "479", "DI" , "Dealer Information (128)", NULL },
  { "479", "DM" , "Shipper's Car Order (422)", NULL },
  { "479", "DS" , "Data Status Tracking (242)", NULL },
  { "479", "DX" , "Direct Exchange Delivery and Return Information (894, 895)", NULL },
  { "479", "EC" , "Educational Course Inventory (188)", NULL },
  { "479", "ED" , "Student Educational Record (Transcript) (130)", NULL },
  { "479", "EI" , "Railroad Equipment Inquiry or Advice (456)", NULL },
  { "479", "ER" , "Revenue Receipts Statement (170)", NULL },
  { "479", "ES" , "Notice of Employment Status (540)", NULL },
  { "479", "EV" , "Railroad Event Report (451)", NULL },
  { "479", "EX" , "Excavation Communication (620)", NULL },
  { "479", "FA" , "Functional Acknowledgment (997)", NULL },
  { "479", "FB" , "Freight Invoice (859)", NULL },
  { "479", "FC" , "Court and Law Enforcement Information (175, 176)", NULL },
  { "479", "FG" , "Motor Carrier Loading and Route Guide (217)", NULL },
  { "479", "FH" , "Motor Carrier Tariff Information (218)", NULL },
  { "479", "FR" , "Financial Reporting (821, 827)", NULL },
  { "479", "FT" , "File Transfer (996)", NULL },
  { "479", "GB" , "Average Agreement Demurrage (423)", NULL },
  { "479", "GC" , "Damage Claim Transaction Sets (920, 924, 925, 926)", NULL },
  { "479", "GE" , "General Request, Response or Confirmation (814)", NULL },
  { "479", "GF" , "Response to a Load Tender (990)", NULL },
  { "479", "GL" , "Intermodal Group Loading Plan (715)", NULL },
  { "479", "GP" , "Grocery Products Invoice (880)", NULL },
  { "479", "GR" , "Statistical Government Information (152)", NULL },
  { "479", "GT" , "Grant or Assistance Application (194)", NULL },
  { "479", "HB" , "Eligibility, Coverage or Benefit Information (271)", NULL },
  { "479", "HC" , "Health Care Claim (837)", NULL },
  { "479", "HI" , "Health Care Services Review Information (278)", NULL },
  { "479", "HN" , "Health Care Claim Status Notification (277)", NULL },
  { "479", "HP" , "Health Care Claim Payment/Advice (835)", NULL },
  { "479", "HR" , "Health Care Claim Status Request (276)", NULL },
  { "479", "HS" , "Eligibility, Coverage or Benefit Inquiry (270)", NULL },
  { "479", "IA" , "Air Freight Details and Invoice (110, 980)", NULL },
  { "479", "IB" , "Inventory Inquiry/Advice (846)", NULL },
  { "479", "IC" , "Rail Advance Interchange Consist (418)", NULL },
  { "479", "ID" , "Insurance/Annuity Application Status (273)", NULL },
  { "479", "IE" , "Insurance Producer Administration (252)", NULL },
  { "479", "IF" , "Individual Insurance Policy and Client Information (111)", NULL },
  { "479", "IG" , "Direct Store Delivery Summary Information (882)", NULL },
  { "479", "II" , "Rail Freight Details and Invoice Summary (411)", NULL },
  { "479", "IJ" , "Report of Injury, Illness or Incident (148)", NULL },
  { "479", "IM" , "Motor Carrier Freight Details and Invoice (210, 980)", NULL },
  { "479", "IN" , "Invoice Information (810,819)", NULL },
  { "479", "IO" , "Ocean Shipment Billing Details (310, 312, 980)", NULL },
  { "479", "IP" , "Intermodal Ramp Activity (622)", NULL },
  { "479", "IR" , "Rail Carrier Freight Details and Invoice (410, 980)", NULL },
  { "479", "IS" , "Estimated Time of Arrival and Car Scheduling (421)", NULL },
  { "479", "KM" , "Commercial Vehicle Credentials (286)", NULL },
  { "479", "LA" , "Federal Communications Commission (FCC) License Application (195)", NULL },
  { "479", "LB" , "Lockbox (823)", NULL },
  { "479", "LI" , "Locomotive Information (436)", NULL },
  { "479", "LN" , "Property and Casualty Loss Notification (272)", NULL },
  { "479", "LR" , "Logistics Reassignment (536)", NULL },
  { "479", "LS" , "Asset Schedule (851)", NULL },
  { "479", "LT" , "Student Loan Transfer and Status Verification (144)", NULL },
  { "479", "MA" , "Motor Carrier Summary Freight Bill Manifest (224)", NULL },
  { "479", "MC" , "Request for Motor Carrier Rate Proposal (107)", NULL },
  { "479", "MD" , "Department of Defense Inventory Management (527)", NULL },
  { "479", "ME" , "Mortgage Origination (198, 200, 201, 261, 262, 263, 833, 872)", NULL },
  { "479", "MF" , "Market Development Funds Settlement (884)", NULL },
  { "479", "MG" , "Mortgage Servicing Transaction Sets (203, 206, 260, 264, 266)", NULL },
  { "479", "MH" , "Motor Carrier Rate Proposal (106)", NULL },
  { "479", "MI" , "Motor Carrier Shipment Status Inquiry (213)", NULL },
  { "479", "MJ" , "Secondary Mortgage Market Loan Delivery (202)", NULL },
  { "479", "MK" , "Response to a Motor Carrier Rate Proposal (108)", NULL },
  { "479", "MM" , "Medical Event Reporting (500)", NULL },
  { "479", "MN" , "Mortgage Note (205)", NULL },
  { "479", "MO" , "Maintenance Service Order (650)", NULL },
  { "479", "MP" , "Motion Picture Booking Confirmation (159)", NULL },
  { "479", "MQ" , "Consolidators Freight Bill and Invoice (223)", NULL },
  { "479", "MR" , "Multilevel Railcar Load Details (125)", NULL },
  { "479", "MS" , "Material Safety Data Sheet (848)", NULL },
  { "479", "MT" , "Electronic Form Structure (868)", NULL },
  { "479", "MV" , "Material Obligation Validation (517)", NULL },
  { "479", "MW" , "Rail Waybill Response (427)", NULL },
  { "479", "MX" , "Material Claim (847)", NULL },
  { "479", "MY" , "Response to a Cartage Work Assignment (225)", NULL },
  { "479", "MZ" , "Motor Carrier Package Status (240)", NULL },
  { "479", "NC" , "Nonconformance Report (842)", NULL },
  { "479", "NL" , "Name and Address Lists (101)", NULL },
  { "479", "NP" , "Notice of Power of Attorney (157)", NULL },
  { "479", "NT" , "Notice of Tax Adjustment or Assessment (149)", NULL },
  { "479", "OC" , "Cargo Insurance Advice of Shipment (362)", NULL },
  { "479", "OG" , "Order Group - Grocery (875, 876)", NULL },
  { "479", "OR" , "Organizational Relationships (816)", NULL },
  { "479", "OW" , "Warehouse Shipping Order (940)", NULL },
  { "479", "PA" , "Price Authorization Acknowledgment/Status (845)", NULL },
  { "479", "PB" , "Railroad Parameter Trace Registration (455)", NULL },
  { "479", "PC" , "Purchase Order Change Request - Buyer Initiated (860)", NULL },
  { "479", "PD" , "Product Activity Data (852)", NULL },
  { "479", "PE" , "Periodic Compensation (256)", NULL },
  { "479", "PF" , "Annuity Activity (268)", NULL },
  { "479", "PG" , "Insurance Plan Description (100)", NULL },
  { "479", "PH" , "Pricing History (503)", NULL },
  { "479", "PI" , "Patient Information (275)", NULL },
  { "479", "PJ" , "Project Schedule Reporting (806)", NULL },
  { "479", "PK" , "Project Cost Reporting (839) and Contractor Cost Data Reporting (196)", NULL },
  { "479", "PL" , "Railroad Problem Log Inquiry or Advice (452)", NULL },
  { "479", "PN" , "Product Source Information (244)", NULL },
  { "479", "PO" , "Purchase Order (850)", NULL },
  { "479", "PQ" , "Property Damage Report (112)", NULL },
  { "479", "PR" , "Purchase Order Acknowledgement (855)", NULL },
  { "479", "PS" , "Planning Schedule with Release Capability (830)", NULL },
  { "479", "PT" , "Product Transfer and Resale Report (867)", NULL },
  { "479", "PU" , "Motor Carrier Shipment Pick-up Notification (216)", NULL },
  { "479", "PV" , "Purchase Order Shipment Management Document (250)", NULL },
  { "479", "PY" , "Payment Cancellation Request (829)", NULL },
  { "479", "QG" , "Product Information (878, 879, 888, 889, 893, 896)", NULL },
  { "479", "QM" , "Transportation Carrier Shipment Status Message (214)", NULL },
  { "479", "QO" , "Ocean Shipment Status Information (313, 315)", NULL },
  { "479", "RA" , "Payment Order/Remittance Advice (820)", NULL },
  { "479", "RB" , "Railroad Clearance (470)", NULL },
  { "479", "RC" , "Receiving Advice/Acceptance Certificate (861)", NULL },
  { "479", "RD" , "Royalty Regulatory Report (185)", NULL },
  { "479", "RE" , "Warehouse Stock Receipt Advice (944)", NULL },
  { "479", "RH" , "Railroad Reciprocal Switch File (433)", NULL },
  { "479", "RI" , "Routing and Carrier Instruction (853)", NULL },
  { "479", "RJ" , "Railroad Mark Register Update Activity (434)", NULL },
  { "479", "RK" , "Standard Transportation Commodity Code Master (435)", NULL },
  { "479", "RL" , "Rail Industrial Switch List (423)", NULL },
  { "479", "RM" , "Railroad Station Master File (431)", NULL },
  { "479", "RN" , "Requisition Transaction (511)", NULL },
  { "479", "RO" , "Ocean Booking Information (300, 301,303)", NULL },
  { "479", "RP" , "Commission Sales Report (818)", NULL },
  { "479", "RQ" , "Request for Quotation (840) and Procurement Notices (836)", NULL },
  { "479", "RR" , "Response to Request For Quotation (843)", NULL },
  { "479", "RS" , "Order Status Information (869, 870)", NULL },
  { "479", "RT" , "Report of Test Results (863)", NULL },
  { "479", "RU" , "Railroad Retirement Activity (429)", NULL },
  { "479", "RV" , "Railroad Junctions and Interchanges Activity (437)", NULL },
  { "479", "RW" , "Rail Revenue Waybill (426)", NULL },
  { "479", "RX" , "Rail Deprescription (432)", NULL },
  { "479", "RY" , "Request for Student Educational Record (Transcript) (146)", NULL },
  { "479", "RZ" , "Response to Request for Student Educational Record (Transcript) (147)", NULL },
  { "479", "SA" , "Air Shipment Information (104)", NULL },
  { "479", "SB" , "Switch Rails (424)", NULL },
  { "479", "SC" , "Price/Sales Catalog (832)", NULL },
  { "479", "SD" , "Student Loan Pre-Claims and Claims (191)", NULL },
  { "479", "SE" , "Shipper's Export Declaration (601)", NULL },
  { "479", "SG" , "SG Receiving Advice - Grocery (885)", NULL },
  { "479", "SH" , "Ship Notice/Manifest (856)", NULL },
  { "479", "SI" , "Shipment Information (858)", NULL },
  { "479", "SJ" , "Transportation Automatic Equipment Identification (160)", NULL },
  { "479", "SL" , "Student Loan Application and Guarantee (135, 139)", NULL },
  { "479", "SM" , "Motor Carrier Load Tender (204)", NULL },
  { "479", "SN" , "Rail Route File Maintenance (475)", NULL },
  { "479", "SO" , "Ocean Shipment Information (304, 306, 309, 311, 317, 319, 321, 322, 323, 324, 325, 350, 352, 353, 354, 355, 356, 357, 358, 361)", NULL },
  { "479", "SP" , "Specifications/Technical Information (841)", NULL },
  { "479", "SQ" , "Production Sequence (866)", NULL },
  { "479", "SR" , "Rail Carrier Shipment Information (404, 419)", NULL },
  { "479", "SS" , "Shipping Schedule (862)", NULL },
  { "479", "ST" , "Railroad Service Commitment Advice (453)", NULL },
  { "479", "SU" , "Account Assignment/Inquiry and Service/Status (248)", NULL },
  { "479", "SV" , "Student Enrollment Verification (190)", NULL },
  { "479", "SW" , "Warehouse Shipping Advice (945)", NULL },
  { "479", "TA" , "Electronic Filing of Tax Return Data Acknowledgment (151)", NULL },
  { "479", "TC" , "Court Submission (176)", NULL },
  { "479", "TD" , "Trading Partner Profile (838)", NULL },
  { "479", "TF" , "Electronic Filing of Tax Return Data (813)", NULL },
  { "479", "TI" , "Tax Information Exchange (826)", NULL },
  { "479", "TM" , "Motor Carrier Delivery Trailer Manifest (212)", NULL },
  { "479", "TN" , "Tax Rate Notification (150)", NULL },
  { "479", "TO" , "Real Estate Title Services (197, 199, 265)", NULL },
  { "479", "TP" , "Rail Rate Transactions (460, 463, 466, 468, 485, 486, 490, 492, 494)", NULL },
  { "479", "TR" , "Train Sheet (161)", NULL },
  { "479", "TS" , "Transportation Services Tender (602)", NULL },
  { "479", "TT" , "Education Testing Results Request and Report (138)", NULL },
  { "479", "TX" , "Text Message (864)", NULL },
  { "479", "UA" , "Retail Account Characteristics (885)", NULL },
  { "479", "UB" , "Customer Call Reporting (886)", NULL },
  { "479", "UC" , "Uniform Commercial Code Filing (154)", NULL },
  { "479", "UD" , "Deduction Research Report (891)", NULL },
  { "479", "UI" , "Underwriting Information Services (255)", NULL },
  { "479", "UP" , "Motor Carrier Pick-up Manifest (215)", NULL },
  { "479", "UW" , "Insurance Underwriting Requirements Reporting (186)", NULL },
  { "479", "VA" , "Vehicle Application Advice (126)", NULL },
  { "479", "VB" , "Vehicle Baying Order (127)", NULL },
  { "479", "VC" , "Vehicle Shipping Order (120)", NULL },
  { "479", "VD" , "Vehicle Damage (124)", NULL },
  { "479", "VE" , "Vessel Content Details (109)", NULL },
  { "479", "VH" , "Vehicle Carrier Rate Update (129)", NULL },
  { "479", "VI" , "Voter Registration Information (280)", NULL },
  { "479", "VS" , "Vehicle Service (121)", NULL },
  { "479", "WA" , "Product Service Transaction Sets (140, 141, 142, 143)", NULL },
  { "479", "WB" , "Rail Carrier Waybill Interchange (417)", NULL },
  { "479", "WG" , "Vendor Performance Review (501)", NULL },
  { "479", "WI" , "Wage Determination (288)", NULL },
  { "479", "WL" , "Well Information (625)", NULL },
  { "479", "WR" , "Shipment Weights (440)", NULL },
  { "479", "WT" , "Rail Waybill Request (425)", NULL },
  { "480", "001000", "ASC X12 Standards Approved by ANSI in 1983", NULL },
  { "480", "002000", "ASC X12 Standards Approved by ANSI in Feb, 1986", NULL },
  { "480", "002001", "Draft Standards Approved by ASC X12 in November 1987", NULL },
  { "480", "002002", "Draft Standards Approved by ASC X12 through February 1988", NULL },
  { "480", "002003", "Draft Standards Approved by ASC X12 through August 1988", NULL },
  { "480", "002031", "Draft Standards Approved by ASC X12 Through February 1989", NULL },
  { "480", "002040", "Draft Standards Approved by ASC X12 Through May 1989", NULL },
  { "480", "002041", "Draft Standards Approved by ASC X12 Through October 1989", NULL },
  { "480", "002042", "Draft Standards Approved By ASC X12 Through February 1990", NULL },
  { "480", "003000", "ASC X12 Standards Approved by ANSI in 1992", NULL },
  { "480", "003010", "Draft Standards Approved By ASC X12 Through June 1990", NULL },
  { "480", "003011", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through February 1991", NULL },
  { "480", "003012", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through June 1991", NULL },
  { "480", "003020", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through October 1991", NULL },
  { "480", "003021", "Draft Standards Approved for Publication By ASC X12 Procedures Review Board through February 1992", NULL },
  { "480", "003022", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through June 1992", NULL },
  { "480", "003030", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board Through October 1992", NULL },
  { "480", "003031", "Draft Standards for Publication by ASC X12 Procedures Review Board Through February 1993", NULL },
  { "480", "003032", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board Through June 1993", NULL },
  { "480", "003040", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through October 1993", NULL },
  { "480", "003041", "Draft Standards Approved for Publication by ASC by ASC X12 Procedures Review Board through February 1994", NULL },
  { "480", "003042", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through June 1994", NULL },
  { "480", "003050", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through October 1994", NULL },
  { "480", "003051", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through February 1995", NULL },
  { "480", "003052", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through June 1995", NULL },
  { "480", "003060", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through October 1995", NULL },
  { "480", "003061", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through February 1996", NULL },
  { "480", "003062", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through June 1996", NULL },
  { "480", "003070", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through October 1996", NULL },
  { "480", "003071", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through February 1997", NULL },
  { "480", "003072", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through June 1997", NULL },
  { "480", "004000", "ASC X12 Standards Approved by ANSI in 1997", NULL },
  { "480", "004010", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through October 1997", NULL },
  { "480", "004011", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through February 1998", NULL },
  { "480", "004012", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through June 1998", NULL },
  { "480", "004020", "Draft Standards Approved for Publication by ASC X12 Procedures Review Board through October 1998", NULL },
  {NULL, NULL, NULL, NULL}
};

static edi_directory_t *X12_V4011_SYSTEM(void)
{
  return (edi_directory_t *) edi_francesco_create(elementinfo,
						  compositeinfo,
						  segmentinfo,
						  compositecontents,
						  segmentcontents,
						  codelistinfo,
						  transactionsetrule);
}
