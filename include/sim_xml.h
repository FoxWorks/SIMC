////////////////////////////////////////////////////////////////////////////////
/// @file
///
/// @brief XML Parsing Core Functions
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2015, Black Phoenix
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the GNU Lesser General Public License as published by the Free Software
/// Foundation; either version 2 of the License, or (at your option) any later
/// version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
/// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
/// details.
///
/// You should have received a copy of the GNU Lesser General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place - Suite 330, Boston, MA  02111-1307, USA.
///
/// Further information about the GNU Lesser General Public License can also be found on
/// the world wide web at http://www.gnu.org.
////////////////////////////////////////////////////////////////////////////////
#ifndef SIM_XML_H
#define SIM_XML_H
#ifdef __cplusplus
extern "C" {
#endif

typedef void* SIMC_XML_DOCUMENT;
typedef void* SIMC_XML_ELEMENT;
typedef void* SIMC_XML_ATTRIBUTE;

int SIMC_XML_Open(const char* filename, SIMC_XML_DOCUMENT** xmldoc, SIMC_Callback_XMLSyntaxError* syntaxError, void* userdata);
int SIMC_XML_OpenString(const char* string, SIMC_XML_DOCUMENT** xmldoc, SIMC_Callback_XMLSyntaxError* syntaxError, void* userdata);
int SIMC_XML_Close(SIMC_XML_DOCUMENT* xmldoc);
int SIMC_XML_GetRootElement(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT** xmlelement, const char* name);
int SIMC_XML_GetElement(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlrootelement, SIMC_XML_ELEMENT** xmlelement, const char* name);
int SIMC_XML_GetAttribute(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, char** value);
int SIMC_XML_GetAttributeInt(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, int* value);
int SIMC_XML_GetAttributeDouble(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, double* value);
int SIMC_XML_GetText(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, char** value);
int SIMC_XML_GetName(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, char** value);
int SIMC_XML_Iterate(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, SIMC_XML_ELEMENT** xmlnested_element, const char* name);
int SIMC_XML_GetFirstAttribute(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, SIMC_XML_ATTRIBUTE** xmlnested_attribute);
int SIMC_XML_IterateAttributes(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ATTRIBUTE* xmlattribute, SIMC_XML_ATTRIBUTE** xmlnested_attribute);
int SIMC_XML_GetAttributeText(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ATTRIBUTE* xmlattribute, char** value);
int SIMC_XML_GetAttributeName(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ATTRIBUTE* xmlattribute, char** value);

int SIMC_XML_Create(SIMC_XML_DOCUMENT** xmldoc);
int SIMC_XML_Save(SIMC_XML_DOCUMENT* xmldoc, const char* filename);
int SIMC_XML_SaveString(SIMC_XML_DOCUMENT* xmldoc, char** description);
int SIMC_XML_AddRootElement(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT** xmlelement, const char* name);
int SIMC_XML_AddElement(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlrootelement, SIMC_XML_ELEMENT** xmlelement, const char* name);
int SIMC_XML_AddAttribute(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, const char* value);
int SIMC_XML_AddAttributeDouble(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, double value);
int SIMC_XML_SetText(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* value);

#ifdef __cplusplus
}
#endif
#endif
