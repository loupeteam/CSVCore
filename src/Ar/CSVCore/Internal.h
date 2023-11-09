/*
 * File: Internal.h
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of CSVCore, licensed under the MIT License.
 * 
 */

/********************************************************
*	NAME	: CsvCoreAPI.h
*	FUNCTION: Headerfile for CSV Core
*			  
*********************************************************/
//#define _CSVCORE_

#ifndef __CSVCOREINTERNAL__
#define __CSVCOREINTERNAL__

// include needed .h files
#include <stdarg.h>
#include <stdio.h>


#define COL_TYP INT
#define COL_INTERP_TYP DINT
#define TYPE_TYP DINT

// Internal Prototypes
unsigned long optionalEscape(char* pDest, unsigned long destSize, char* pSrc, unsigned long maxChars, char character, char quoteChar, unsigned short forceEscape);
unsigned long escapeChar(char* pDest, unsigned long destSize, char* pSrc, unsigned long maxChars, char quoteChar);
unsigned long unescapeChar(char* pDest, unsigned long destSize, char* pSrc, unsigned long maxChars, char quoteChar);

// Parse
signed long allocPredef(struct CSVPredef_typ* pPredef);
signed long setPredefColumnsToDefault(COL_TYP* pColumns, unsigned long numColumns);
signed long readRowList(struct CSVCore_typ* pData, struct CSVPredef_typ* pPredef, va_list args);
signed long getNextRow(struct CSVCore_typ* pData);
long countSpaceL(char* string, long max);
long countSpaceR(char* string, long max);
COL_TYP CSVIndexOf(COL_TYP value, COL_TYP* array, UDINT numElements);
signed long setVarValue(void* pVar, char* value, UDINT valueLen, enum CSV_CORE_TYPE_enum dataType, unsigned short unescapeString);

// Build
signed long buildRowList(struct CSVCore_typ* pData, struct CSVPredef_typ* pPredef, va_list args);
signed long appendTo(char** string, char* value, long* pSpace);
signed long getVarValue(void* pVar, char* value, long valueSize, enum CSV_CORE_TYPE_enum dataType, long* pLength, CSVCoreCfg_typ* cfg);


#endif
