/*
 * File: Parse.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of CSVCore, licensed under the MIT License.
 * 
 */


#include <bur/plctypes.h>

//unsigned long bur_heap_size=0xFFFFF;

#include <string.h>
//#include <strings.h> // strcasecmp and strncasecmp
#include <stdarg.h>
#include <stdio.h>
#include "ctype.h"

#ifdef __cplusplus
	extern "C"
	{
#endif
#include "CsvCoreAPI.h"
#include "CSVCore.h"
#ifdef __cplusplus

	};
#endif

#include "Internal.h"
#include "StringExt.h"

#define COL_TYP INT
#define COL_INTERP_TYP DINT
#define TYPE_TYP DINT
#define ERR_TYP DINT

#define min( a, b ) ( ( (a) < (b)) ? (a) : (b) )

// TODO: Move internal functions to seperate .c files?

signed long csvInitPredef(struct CSVPredef_typ* pPredef, unsigned long numColumns, unsigned long pColumns, unsigned long pTypes, unsigned long pStatus) {
	if(pPredef == 0) return CSV_CORE_ERR_INVALID_INPUT;
	if(numColumns == 0 ) return CSV_CORE_ERR_NUM_COLUMNS;
	
	// TODO: If pColumns || pTypes || pStatus is passed 0 but predef already has those, should we keep old values?
	pPredef->numColumns = numColumns;
	pPredef->pColumns = pColumns;
	pPredef->pTypes = pTypes;
	pPredef->pStatus = pStatus;
	
	return allocPredef(pPredef);
}

// Allocates space for predef NULL pointer members
// Possible Errors: INVALID_INPUT, NUM_COLUMNS
signed long allocPredef(struct CSVPredef_typ* pPredef) {
	if(pPredef == 0) return CSV_CORE_ERR_INVALID_INPUT;
	if(pPredef->numColumns == 0 ) return CSV_CORE_ERR_NUM_COLUMNS;
	
	unsigned int status = 0;
	
	if(pPredef->pColumns == 0 && !status) {
		status = TMP_alloc(sizeof(COL_TYP)*pPredef->numColumns, (void*)&pPredef->pColumns);
		if(!status) setPredefColumnsToDefault((COL_TYP*)pPredef->pColumns, pPredef->numColumns);
	}
	if(pPredef->pTypes == 0 && !status)
		status = TMP_alloc(sizeof(TYPE_TYP)*pPredef->numColumns, (void*)&pPredef->pTypes);
	if(pPredef->pStatus == 0 && !status)
		status = TMP_alloc(sizeof(ERR_TYP)*pPredef->numColumns, (void*)&pPredef->pStatus);
	
	return status;
}

// Sets predef column to defualts
// Defualt is each column in order
// Possible Errors: None
signed long setPredefColumnsToDefault(COL_TYP* pColumns, unsigned long numColumns) {
	int i;
	for (i = 0; i < numColumns; i++) {
		pColumns[i] = i;
	}
	
	return 0;
}

// Tested
// Possible Errors: INVALID_INPUT, NUM_COLUMNS, HEADER_INDEX, END_OF_DATA
signed long csvFilterByHeader(struct CSVCore_typ* pCsv, struct CSVPredef_typ* pPredef, ...) {
	// if pCsv == 0
	if(pPredef == 0) return CSV_CORE_ERR_INVALID_INPUT;
	if(pPredef->numColumns == 0) return CSV_CORE_ERR_NUM_COLUMNS;
	if(pCsv->cfg.headerRow == 0) return CSV_CORE_ERR_HEADER_INDEX;
	
	char* header[pPredef->numColumns];
	long headerLen[pPredef->numColumns];
	char *cell;
	long whitespace, columnIndex, i, status;
	unsigned long cellLen;
	va_list args;
	
	va_start (args, pPredef);
	for (i = 0; i < pPredef->numColumns; i++) {
		header[i] = va_arg(args, char*);
		headerLen[i] = strlen(header[i]);
		// Remove whitespace 
		whitespace = countSpaceR(header[i], headerLen[i]);
		header[i] += whitespace;
		headerLen[i] -= whitespace;
		whitespace = countSpaceL(header[i]+(headerLen[i]?headerLen[i]-1:0), headerLen[i]); // We need to -1 becuase the first character we want to look at is shifted by len -1
		headerLen[i] -= whitespace;
	}
	va_end (args);
	
	// Update outputs
	pCsv->out.headerRow = pCsv->cfg.headerRow;
	
	memset(pPredef->pColumns, -1, sizeof(COL_TYP) * pPredef->numColumns); // Reset all pColumns
	
	// Read current row
	cell = (char*)pCsv->internal.pData;
	columnIndex = 0;
	
	status = csvGetNLine(&cell, 0, pCsv->cfg.headerRow - 1, 0); // -1 because getNLine is zero indexed
	status = csvGetCellLen(cell, &cellLen, pCsv->cfg.delim);
	
	if(status == CSV_CORE_ERR_END_OF_DATA) {
		// Header not found
		return CSV_CORE_ERR_END_OF_DATA;
	}
	
	while(status != CSV_CORE_ERR_END_OF_DATA && status != CSV_CORE_ERR_END_OF_LINE) {		
		// Remove whitespace 
		whitespace = countSpaceR(cell, cellLen);
		cell += whitespace;
		cellLen -= whitespace;
		whitespace = countSpaceL(cell+(cellLen?cellLen-1:0), cellLen); // We need to -1 becuase the first character we want to look at is shifted by len -1
		cellLen -= whitespace;
		
		for (i = 0; i < pPredef->numColumns; i++) {
			if(cellLen == headerLen[i] 
			&& strncasecmp(cell, header[i], cellLen) == 0) {
				
				((COL_TYP*)pPredef->pColumns)[i] = columnIndex;
			}
		}
		
		status = csvGetNextCell(&cell, &cellLen, pCsv->cfg.delim);
		columnIndex++; // Keep track of wich column we are looking at
	}
	
	return 0;
}

// Apply a column filter to predef
// Tested
// Possible Errors: invalid input, num columns
signed long csvFilterByColumn(struct CSVPredef_typ* pPredef, ...) {
	if(pPredef == 0) return CSV_CORE_ERR_INVALID_INPUT;
	if(pPredef->numColumns == 0) return CSV_CORE_ERR_NUM_COLUMNS;
	
	va_list args;
	int i = 0;
	long status;
	
	if(pPredef->pColumns == 0) {
		status = allocPredef(pPredef);
		if(status) return status;
	}
	va_start (args, pPredef);
	
	for (i = 0; i < pPredef->numColumns; i++) {
		((COL_TYP*)pPredef->pColumns)[i] = va_arg(args, COL_INTERP_TYP);
	}
	
	va_end (args);
	return 0;
}

// Define types in predef
// Tested
// todo rename
// Possible Errors: invalid input, num columns
signed long csvCastAs(struct CSVPredef_typ* pPredef, ...) {
	if(pPredef == 0) return CSV_CORE_ERR_INVALID_INPUT;
	if(pPredef->numColumns == 0) return CSV_CORE_ERR_NUM_COLUMNS;
	
	va_list args;
	int i = 0;
	long status;
	
	if(pPredef->pTypes == 0) {
		status = allocPredef(pPredef);
		if(status) return status;
	}
	va_start (args, pPredef);
	
	for (i = 0; i < pPredef->numColumns; i++) {
		((TYPE_TYP*)pPredef->pTypes)[i] = va_arg(args, TYPE_TYP);
	}
	
	va_end (args);
	return 0;
}

// Define all types in predef
// Tested
// todo rename
// Possible Errors: invalid input
signed long csvCastAllAs(struct CSVPredef_typ* pPredef, enum CSV_CORE_TYPE_enum dataType) {
	if(pPredef == 0) return CSV_CORE_ERR_INVALID_INPUT;
	
	int i = 0;
	
	if(pPredef->pTypes == 0) {
		allocPredef(pPredef);
	}
	
	for (i = 0; i < pPredef->numColumns; i++) {
		((TYPE_TYP*)pPredef->pTypes)[i] = dataType;
	}
	
	return 0;
}

// TODO: Should this take 2 predef types instead
// TODO: Not sure if the desired behavoir has been fully defined
// Possible Errors: invalid input
signed long csvSetPredef(struct CSVCore_typ* pCsv, struct CSVPredef_typ* pPredef) {
	if(pCsv == 0 || pPredef == 0) return CSV_CORE_ERR_INVALID_INPUT;
	memcpy(&pCsv->predef, pPredef, sizeof(CSVPredef_typ));
	return 0;
}

// Tested
// Possible returns: OK, NO NEWLINE, INTERNAL, END OF DATA
signed long csvReadNextRow(struct CSVCore_typ* pCsv, ...){
	va_list args;
	long returnVar = 0;
	long status;
	
	va_start (args, pCsv);
	status = getNextRow(pCsv);
	if(status == 0 || status == CSV_CORE_ERR_NO_NEWLINE)
		returnVar = readRowList(pCsv, &pCsv->predef, args);
	va_end (args);
	return status ? status : returnVar; // TODO: maybe return something else here
}

// Tested
// Possible returns: OK, INTERNAL, END_OF_DATA
signed long csvReadRowAs(struct CSVCore_typ* pCsv, struct CSVPredef_typ* pPredef, ...) {
	va_list args;
	signed long returnVar;
	
	if(pCsv->internal.pRow == 0) 
		getNextRow(pCsv);
	
	va_start (args, pPredef);
	returnVar = readRowList(pCsv, pPredef, args);
	va_end (args);
	return returnVar;
}

// Tested
// Possible returns: OK, NO_NEWLINE, INTERNAL, END_OF_DATA
signed long csvReadNextRowAs(struct CSVCore_typ* pCsv, struct CSVPredef_typ* pPredef, ...) {
	va_list args;
	long returnVar = 0;
	long status;
	
	va_start (args, pPredef);
	status = getNextRow(pCsv);
	if(status == 0 || status == CSV_CORE_ERR_NO_NEWLINE)
		returnVar = readRowList(pCsv, pPredef, args);
	va_end (args);
	return status ? status : returnVar;
}

// Tested
// Possible returns: OK, INTERNAL
signed long readRowList(struct CSVCore_typ* pCsv, struct CSVPredef_typ* pPredef, va_list args) {
	int i, index;
	long whitespace, status = 0;
	UDINT pVar;
	char* cell;
	UDINT cellLen;
	char* filteredCells[pPredef->numColumns];
	UDINT filteredCellLen[pPredef->numColumns];
	TYPE_TYP* pTypes = (TYPE_TYP*)pPredef->pTypes;
	COL_TYP* pColumns = (COL_TYP*)pPredef->pColumns;
	
	if(pCsv->internal.pRow == 0) return CSV_CORE_ERR_INTERNAL; // This is for debug
	
	memset(filteredCells, 0, sizeof(char*)*pPredef->numColumns); // memset instead of initialize because variable-sized object
	
	// Read current row
	cell = (char*)pCsv->internal.pRow;
	i = 0;
	
	csvGetCellLen(cell, &cellLen, pCsv->cfg.delim);
	while(status != CSV_CORE_ERR_END_OF_DATA && status != CSV_CORE_ERR_END_OF_LINE) {
		
		index = CSVIndexOf(i, pColumns, pPredef->numColumns);
		if(index != -1) {
			whitespace = countSpaceR(cell, cellLen);
			cell += whitespace;
			cellLen -= whitespace;
			whitespace = countSpaceL(cell+(cellLen?cellLen-1:0), cellLen); // We need to -1 becuase the first character we want to look at is shifted by len -1
			cellLen -= whitespace;
			
			filteredCells[index] = cell;
			filteredCellLen[index] = cellLen;
		}
		
		status = csvGetNextCell(&cell, &cellLen, pCsv->cfg.delim);
		i++;
	}
	
	// Populate args
	ERR_TYP tempStatus;
	for (i = 0; i < pPredef->numColumns; i++) {
		pVar = va_arg(args, unsigned long);
		tempStatus = setVarValue((void*)pVar, filteredCells[i], filteredCellLen[i], pTypes[i], pCsv->cfg.quoteUnescaping);
		if(pPredef->pStatus != 0)
			((ERR_TYP*)pPredef->pStatus)[i] = tempStatus;
	}
	
	return 0; // Return
}

// Possible returns: OK
signed long csvInitData(struct CSVCore_typ* pCsv, unsigned long pData, unsigned long dataSize) {
	pCsv->internal.pData = pData;
	pCsv->internal.dataSize = dataSize;
	pCsv->internal.pRow = 0;
	pCsv->internal.rowNumber = 0;
	
	return 0;
}

// Internal get next row in pCsv
// Get next row and increment row number
// Skips rows that start with configured comment
// Possible returns: OK, INVALID_INPUT, END_OF_DATA, NO_NEWLINE
signed long getNextRow(struct CSVCore_typ* pCsv) {
	long status = 0;
	long commentLen = strlen(pCsv->cfg.comment);
	if(pCsv->internal.pRow == 0) {
		pCsv->internal.pRow = pCsv->internal.pData;
		pCsv->internal.rowNumber = 1;
		pCsv->out.headerRow = pCsv->cfg.headerRow;
		if(pCsv->cfg.headerRow == 0) {
			csvGetLineLen((char*)pCsv->internal.pRow, &pCsv->internal.rowLen);
			while(!status 
			&& ((commentLen && strncmp(pCsv->cfg.comment, (char*)pCsv->internal.pRow, commentLen) == 0) || (pCsv->internal.rowLen == 0 && pCsv->cfg.ignoreEmptyRows))) { 
				// Get next row until we do not have a comment
				status = (UDINT)csvGetNextLine((char**)&pCsv->internal.pRow, &pCsv->internal.rowLen, pCsv->cfg.ignoreEmptyRows);
				pCsv->internal.rowNumber++;
			}
		}
		else {
			status = csvGetNLine((char**)&pCsv->internal.pRow, &pCsv->internal.rowLen, pCsv->cfg.headerRow, 0); // getNLine is zero indexed 
			pCsv->internal.rowNumber = pCsv->cfg.headerRow + 1; // +1 because we are getting the line after the header 
		}
	}
	else {		
		do {
			status = (UDINT)csvGetNextLine((char**)&pCsv->internal.pRow, &pCsv->internal.rowLen, pCsv->cfg.ignoreEmptyRows);
			pCsv->internal.rowNumber++;			
		} while(!status && commentLen && strncmp(pCsv->cfg.comment, (char*)pCsv->internal.pRow, commentLen) == 0); // Skip commented rows
	}
	
	// Update outputs
	pCsv->out.pRow = pCsv->internal.pRow;
	pCsv->out.rowLen = pCsv->internal.rowLen;
	pCsv->out.rowNumber = pCsv->internal.rowNumber;
	
	return status;
}

// Count the number of whitespace chars starting from string moving to the right
// Only checks a maxium of max characters
long countSpaceR(char* string, long max) {
	int space = 0;
	while(space <= max && isspace(string[space]))
		space++;
	
	return space;
}

// Count the number of whitespace chars starting from string moving to the left
// Only checks a maxium of max characters
long countSpaceL(char* string, long max) {
	int space = 0;
	while(space <= max && isspace(string[-space]))
		space++;
	
	return space;
}

// Finds index of value in array 
// Returns -1 if not found
COL_TYP CSVIndexOf(COL_TYP value, COL_TYP* array, UDINT numElements) {
	int i;
	for (i = 0; i < numElements; i++) {
		if(value == array[i])
			return i;
	}
	return -1;
}

// @param	pVar			Pointer to variable to be populated
// @param	value			Pointer to string value to be written to pVar, not required to be null terminated
// @param	valueLen		Length of string value
// @param	dataType		Data type of pVar
// @param	unescapeString	TRUE indicates quoted strings should be unescaped and the quotes removed
// Set pVar with string value converted to type dataType
// Possible returns: OK, INVALID_INPUT, COLUMN_NOT_FOUND, INVALID_TYPE, BUFFER_FULL, UNSUPPORTED_TYPE, INVALID_VALUE
signed long setVarValue(void* pVar, char* value, UDINT valueLen, enum CSV_CORE_TYPE_enum dataType, unsigned short unescapeString) {
	if(pVar == 0) return CSV_CORE_ERR_INVALID_INPUT;
	if(value == 0 || valueLen == 0) return CSV_CORE_ERR_COLUMN_NOT_FOUND;
	
	UINT varLen;
	if(dataType >= CSV_CORE_TYPE_STRINGN) { // STRINGN can have length encoded into it
		varLen = dataType - CSV_CORE_TYPE_STRINGN;
		varLen += !varLen ? 0 : -1; // Remove 1 from size to get max len
		dataType = CSV_CORE_TYPE_STRINGN;
	}
	
	int len = dataType == CSV_CORE_TYPE_STRING || dataType == CSV_CORE_TYPE_STRINGN ? 0 : valueLen; // We dont want to declare a temp string for types that dont need casting
	char tempString[len+1];
	UDINT ValueUdint;
	DINT ValueDint;
	REAL ValueReal;
	
	// Copy value into tempString
	// value may not be Null terminated
	// and to parse properly we need a NULL terminated string
	// so we handle that by doing a stringlcpy which will always append a NULL
	// dataType of Strings will not do this because they can be large and are not required to be NULL terminated to handle them
	stringlcpy((UDINT)tempString, (UDINT)value, len+1);
	
	switch(dataType){
	
		//*******************************************************************************
		// BOOL																			*
		//*******************************************************************************
		case CSV_CORE_TYPE_BOOL:
		
			if(	strcmp( tempString, "0" ) == 0 ){
		
				*(BOOL*)(pVar)=	0;
		
			} // "0" //
		
			else if( strcmp( tempString, "1" ) == 0 ){
		
				*(BOOL*)(pVar)=	1;
		
			} // "1" //
		
			else{
			
			
				// Convert String to lower case
			
				//ValueStringLength=	strlen( value );
				
				ToLower( (UDINT)tempString ); // convert to lower case
			
			
				// Check string
			
				if( strcasecmp( tempString, "false" ) == 0 ){
		
					*(BOOL*)(pVar)=	0;
		
				}
				else if( strcasecmp( tempString, "true" ) == 0 ){
		
					*(BOOL*)(pVar)=	1;
		
				}
				else{
		
					// Invalid BOOL value
					// Use convention of non-zero == TRUE for now		
					*(BOOL*)(pVar)=	1;
			
				}
			
			
			} // Other string //

			break;
	
	
		//*******************************************************************************
		// Integer types																*
		//*******************************************************************************
		case CSV_CORE_TYPE_SINT:
		
			ValueDint=	brsatoi( (UDINT)tempString );
		
			if( 	(ValueDint > CSV_CORE_MAX_SINT)
				||	(ValueDint < CSV_CORE_MIN_SINT)
			){
			
				return CSV_CORE_ERR_INVALID_VALUE;
		
			}
			else{
			
				*(SINT*)(pVar)=	(SINT)ValueDint;
				
			}
			
			break;
	

		case CSV_CORE_TYPE_INT:
		
			ValueDint=	brsatoi( (UDINT)tempString );
		
			if( 	(ValueDint > CSV_CORE_MAX_INT)
				||	(ValueDint < CSV_CORE_MIN_INT)
			){
			
				return CSV_CORE_ERR_INVALID_VALUE;
		
			}
			else{
			
				*(INT*)(pVar)=	(INT)ValueDint;
				
			}
			
			break;
	
	
		case CSV_CORE_TYPE_DINT:
		case CSV_CORE_TYPE_TIME:
		
			ValueDint=	brsatoi( (UDINT)tempString );
		
			if( 	(ValueDint > CSV_CORE_MAX_DINT)
			||	(ValueDint < CSV_CORE_MIN_DINT)
			){
			
			return CSV_CORE_ERR_INVALID_VALUE;
		
			}
			else{
			
			*(DINT*)(pVar)=	(DINT)ValueDint;
				
			}
			
			break;
	
	
		case CSV_CORE_TYPE_USINT:
		
			ValueDint=	brsatoi( (UDINT)tempString );
		
			if( 	(ValueDint > CSV_CORE_MAX_USINT)
				||	(ValueDint < CSV_CORE_MIN_USINT)
			){
			
				return CSV_CORE_ERR_INVALID_VALUE;
		
			}
			else{
			
				*(USINT*)(pVar)=	(USINT)ValueDint;
				
			}
			
			break;
	
	
		case CSV_CORE_TYPE_UINT:
		
			ValueDint=	brsatoi( (UDINT)tempString );
		
			if( 	(ValueDint > CSV_CORE_MAX_UINT)
				||	(ValueDint < CSV_CORE_MIN_UINT)
			){
			
				return CSV_CORE_ERR_INVALID_VALUE;
		
			}
			else{
			
				*(UINT*)(pVar)=	(UINT)ValueDint;
				
			}
			
			break;
	
	
		case CSV_CORE_TYPE_UDINT:
		case CSV_CORE_TYPE_DATE_AND_TIME:
		
			ValueUdint=	atoui( (UDINT)tempString );
		
			if( 	(ValueUdint > CSV_CORE_MAX_UDINT)
			||	(ValueUdint < CSV_CORE_MIN_UDINT)
			){
			
			return CSV_CORE_ERR_INVALID_VALUE;
		
			}
			else{
			
			*(UDINT*)(pVar)=	(UDINT)ValueUdint;
				
			}
			
			break;
	
	
		//*******************************************************************************
		// REAL																			*
		//*******************************************************************************
		case CSV_CORE_TYPE_REAL:
		
			ValueReal=	brsatof( (UDINT)tempString );
		
			if( 	(ValueReal > CSV_CORE_MAX_REAL)
				||	(ValueReal < CSV_CORE_MIN_REAL)
			){
			
				return CSV_CORE_ERR_INVALID_VALUE;
		
			}
			else{
			
				*(REAL*)(pVar)=	ValueReal;
				
			}
			
			break;
		
		//*******************************************************************************
		// LREAL																			*
		//*******************************************************************************
		case CSV_CORE_TYPE_LREAL: // Lets only support up to a real //
			ValueReal=	brsatof( (UDINT)tempString );
		
			if( 	(ValueReal > CSV_CORE_MAX_REAL)
				||	(ValueReal < CSV_CORE_MIN_REAL)
			){
			
				return CSV_CORE_ERR_INVALID_VALUE;
		
			}
			else{
			
				*(LREAL*)(pVar)=	ValueReal;
				
			}
			
			break;
		
		//*******************************************************************************
		// STRING																		*
		//*******************************************************************************
		case CSV_CORE_TYPE_STRING:
		
			//memset( (char*)pVar, 0, ipVariable->length );
			// Note we do not use tempString here
			// Instead we directly copy from value so we do not add a large var to stack
			// value may not be Null terminated
			// stringlcpy((char*)pVar, value, valueLen+1);
			if(unescapeString
				&& valueLen > 1
				&& value[0] == value[valueLen-1] 
			&& (value[0] == '\'' || value[0] == '\"')) {
				unescapeChar((char*)pVar, valueLen-1, value+1, valueLen-2, value[0]);
			}
			else {
				memcpy( (char*)pVar, value, valueLen); // Do memcpy for performance
				((char*)pVar)[valueLen] = '\0'; // Append null
			}
			break;
		
		
		case CSV_CORE_TYPE_STRINGN:
			// Note we do not use tempString here
			// Instead we directly copy from value so we do not add a large var to stack
			// value may not be Null terminated
			if(unescapeString
				&& valueLen > 1
				&& value[0] == value[valueLen-1] 
			&& (value[0] == '\'' || value[0] == '\"')) {
				unescapeChar((char*)pVar, varLen+1, value+1, valueLen-2, value[0]);
			}
			else {
				memcpy( (char*)pVar, value, min(valueLen, varLen)); // Do memcpy for performance
				((char*)pVar)[min(valueLen, varLen)] = '\0'; // Append null
			}
			
			if(varLen < valueLen)
				return CSV_CORE_ERR_BUFFER_FULL;
		
			break;
		

		//*******************************************************************************
		// Unsupported and Invalid types												*
		//*******************************************************************************
		case CSV_CORE_TYPE_STRUCT:
		case CSV_CORE_TYPE_ULINT:
		case CSV_CORE_TYPE_DATE:
		case CSV_CORE_TYPE_ARRAY_OF_STRUCT:
		case CSV_CORE_TYPE_TIME_OF_DAY:
		case CSV_CORE_TYPE_BYTE:
		case CSV_CORE_TYPE_WORD:
		case CSV_CORE_TYPE_DWORD:
		case CSV_CORE_TYPE_LWORD:
		case CSV_CORE_TYPE_WSTRING: // maybe do something here //
		case CSV_CORE_TYPE_LINT: return CSV_CORE_ERR_UNSUPPORTED_TYPE; break;

		default: return CSV_CORE_ERR_INVALID_TYPE; break;	
	
	
	} // switch(DataType) //
	
	return 0;
}
