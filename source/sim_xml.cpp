////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2013, Black Phoenix
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
#include <tinyxml.h>
#include "sim_core.h"
#include "sim_xml.h"

int SIMC_XML_Open(const char* filename, SIMC_XML_DOCUMENT** xmldoc, SIMC_Callback_XMLSyntaxError* syntaxError, void* userdata) {
	if (!filename) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = new TiXmlDocument(filename);
	if (!doc->LoadFile(filename)) {
		*xmldoc = 0;
		if (doc->ErrorId() == TiXmlDocument::TIXML_ERROR_OPENING_FILE) {
			return SIMC_ERROR_FILE;
		} else {
			if (syntaxError) syntaxError(userdata,doc->ErrorDesc());
			return SIMC_ERROR_SYNTAX;
		}
	}
	*xmldoc = (SIMC_XML_DOCUMENT*)(doc);
	return SIMC_OK;
}

int SIMC_XML_OpenString(const char* string, SIMC_XML_DOCUMENT** xmldoc, SIMC_Callback_XMLSyntaxError* syntaxError, void* userdata) {
	if (!string) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = new TiXmlDocument();
	doc->Parse(string, 0, TIXML_ENCODING_UTF8);
	if (doc->Error()) {
		*xmldoc = 0;
		if (syntaxError) syntaxError(userdata,doc->ErrorDesc());
		return SIMC_ERROR_SYNTAX;
	}
	*xmldoc = (SIMC_XML_DOCUMENT*)(doc);
	return SIMC_OK;
}

int SIMC_XML_Close(SIMC_XML_DOCUMENT* xmldoc) {
	if (!xmldoc) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	delete doc;
	return SIMC_OK;
}

int SIMC_XML_GetRootElement(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT** xmlelement, const char* name) {
	if (!name) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlNode* node = doc->FirstChild(name);
	*xmlelement = (SIMC_XML_ELEMENT*)node;
	return SIMC_OK;
}

int SIMC_XML_GetElement(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlrootelement, SIMC_XML_ELEMENT** xmlelement, const char* name) {
	//if (!name) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;
	if (!xmlrootelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlrootelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;

	if (name) {
		TiXmlNode* node = element->FirstChild(name);
		*xmlelement = (SIMC_XML_ELEMENT*)node;
	} else {
		TiXmlNode* node = element->FirstChild();
		*xmlelement = (SIMC_XML_ELEMENT*)node;
	}
	return SIMC_OK;
}

int SIMC_XML_GetAttribute(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, char** value) {
	if (!name) return SIMC_ERROR_INTERNAL;
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;
	*value = (char*)element->Attribute(name);
	if (!(*value)) *value = "";
	return SIMC_OK;
}

int SIMC_XML_GetAttributeInt(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, int* value) {
	if (!name) return SIMC_ERROR_INTERNAL;
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;
	if (!element->Attribute(name,value)) {
		*value = 0;
	}
	return SIMC_OK;
}

int SIMC_XML_GetAttributeDouble(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, double* value) {
	if (!name) return SIMC_ERROR_INTERNAL;
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;
	if (!element->Attribute(name,value)) {
		*value = 0.0;
	}
	return SIMC_OK;
}

int SIMC_XML_GetText(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, char** value) {
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;
	*value = (char*)element->GetText();
	return SIMC_OK;
}

int SIMC_XML_GetName(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, char** value) {
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;
	*value = (char*)element->Value();
	return SIMC_OK;
}

int SIMC_XML_Iterate(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, SIMC_XML_ELEMENT** xmlnested_element, const char* name) {
	//if (!name) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;
	if (!xmlnested_element) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlNode* node = (TiXmlNode*)xmlelement;
	TiXmlNode* nested_node = (TiXmlNode*)(*xmlnested_element);
	TiXmlElement* element = node->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;

	if (name) {
		TiXmlNode* new_node = element->IterateChildren(name, nested_node);
		*xmlnested_element = (SIMC_XML_ELEMENT*)new_node;
	} else {
		TiXmlNode* new_node = element->IterateChildren(nested_node);
		*xmlnested_element = (SIMC_XML_ELEMENT*)new_node;
	}
	return SIMC_OK;
}

int SIMC_XML_GetFirstAttribute(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, SIMC_XML_ATTRIBUTE** xmlnested_attribute) {
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;
	if (!xmlnested_attribute) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlNode* node = (TiXmlNode*)xmlelement;
	TiXmlElement* element = node->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;

	TiXmlAttribute* new_attribute = element->FirstAttribute();
	*xmlnested_attribute = (SIMC_XML_ATTRIBUTE*)new_attribute;
	return SIMC_OK;
}

int SIMC_XML_IterateAttributes(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ATTRIBUTE* xmlattribute, SIMC_XML_ATTRIBUTE** xmlnested_attribute) {
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlattribute) return SIMC_ERROR_INTERNAL;
	if (!xmlnested_attribute) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlAttribute* attribute = (TiXmlAttribute*)xmlattribute;
	if (!attribute) return SIMC_ERROR_INTERNAL;

	TiXmlAttribute* new_attribute = attribute->Next();
	*xmlnested_attribute = (SIMC_XML_ATTRIBUTE*)new_attribute;
	return SIMC_OK;
}

int SIMC_XML_GetAttributeText(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ATTRIBUTE* xmlattribute, char** value) {
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlattribute) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlAttribute* attribute = (TiXmlAttribute*)xmlattribute;
	if (!attribute) return SIMC_ERROR_INTERNAL;

	*value = (char*)attribute->Value();
	return SIMC_OK;
}

int SIMC_XML_GetAttributeName(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ATTRIBUTE* xmlattribute, char** value) {
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlattribute) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlAttribute* attribute = (TiXmlAttribute*)xmlattribute;
	if (!attribute) return SIMC_ERROR_INTERNAL;

	*value = (char*)attribute->Name();
	return SIMC_OK;
}


int SIMC_XML_Create(SIMC_XML_DOCUMENT** xmldoc) {
	if (!xmldoc) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = new TiXmlDocument();
	*xmldoc = (SIMC_XML_DOCUMENT*)(doc);
	return SIMC_OK;
}

int SIMC_XML_Save(SIMC_XML_DOCUMENT* xmldoc, const char* filename) {
	if (!filename) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	doc->SaveFile(filename);
	return SIMC_OK;
}

int SIMC_XML_SaveString(SIMC_XML_DOCUMENT* xmldoc, char** description) {
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!description) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlPrinter printer;
	printer.SetIndent("\t");

	doc->Accept(&printer);
	const char* buffer = printer.CStr();
	*description = (char*)malloc(sizeof(char)*(strlen(buffer)+1));
	strcpy((char*)(*description),buffer);
	return SIMC_OK;
}

int SIMC_XML_AddRootElement(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT** xmlelement, const char* name) {
	if (!name) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* root = new TiXmlElement(name);
	doc->LinkEndChild(root);

	*xmlelement = (SIMC_XML_ELEMENT*)root;
	return SIMC_OK;
}

int SIMC_XML_AddElement(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlrootelement, SIMC_XML_ELEMENT** xmlelement, const char* name) {
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;
	if (!xmlrootelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* root = ((TiXmlNode*)xmlrootelement)->ToElement();
	if (!root) return SIMC_ERROR_INTERNAL;
	TiXmlElement* element = new TiXmlElement(name);
	if (!element) return SIMC_ERROR_INTERNAL;
	root->LinkEndChild(element);

	*xmlelement = (SIMC_XML_ELEMENT*)element;
	return SIMC_OK;
}


int SIMC_XML_AddAttribute(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, const char* value) {
	if (!name) return SIMC_ERROR_INTERNAL;
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;
	if (*value == 0) return SIMC_OK; //Do not create empty attributes

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;
	
	element->SetAttribute(name,value);
	return SIMC_OK;
}

int SIMC_XML_AddAttributeDouble(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* name, double value) {
	if (!name) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;
	if (value == 0) return SIMC_OK; //Do not create empty attributes

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;
	
	if ((value < 1e-15) && (value > -1e-15)) value = 0.0;
	
	char buffer[1024] = { 0 };
	snprintf(buffer,1023,"%.15g",value);
	element->SetAttribute(name,buffer);
	//element->SetDoubleAttribute(name,value);
	return SIMC_OK;
}

int SIMC_XML_SetText(SIMC_XML_DOCUMENT* xmldoc, SIMC_XML_ELEMENT* xmlelement, const char* value) {
	if (!value) return SIMC_ERROR_INTERNAL;
	if (!xmldoc) return SIMC_ERROR_INTERNAL;
	if (!xmlelement) return SIMC_ERROR_INTERNAL;

	TiXmlDocument* doc = (TiXmlDocument*)xmldoc;
	TiXmlElement* element = ((TiXmlNode*)xmlelement)->ToElement();
	if (!element) return SIMC_ERROR_INTERNAL;
	//element->SetValue(value);

	TiXmlText* text = new TiXmlText(value);
	element->LinkEndChild(text);
	return SIMC_OK;
}
