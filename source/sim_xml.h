////////////////////////////////////////////////////////////////////////////////
/// @file
///
/// @brief XML Parsing Core Functions
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2013, Black Phoenix
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///   - Redistributions of source code must retain the above copyright
///     notice, this list of conditions and the following disclaimer.
///   - Redistributions in binary form must reproduce the above copyright
///     notice, this list of conditions and the following disclaimer in the
///     documentation and/or other materials provided with the distribution.
///   - Neither the name of the author nor the names of the contributors may
///     be used to endorse or promote products derived from this software without
///     specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
/// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
/// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
/// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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