//unsigned long bur_heap_size=0xFFFFF;

#include "string.h"
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

#define LINE_ENDING "\r\n"
#define LINE_ENDING_LEN (sizeof(LINE_ENDING)-1)
#define min( a, b ) ( ( (a) < (b)) ? (a) : (b) )
#define max( a, b ) ( ( (a) > (b)) ? (a) : (b) )

#define COL_TYP INT
#define COL_INTERP_TYP DINT
#define TYPE_TYP DINT
#define ERR_TYP DINT

// Possible returns: OK, BUFFER_FULL
signed long csvBuildRow(struct CSVCore_typ* pCsv, ...) {
	long status;
	
	va_list args;
	va_start (args, pCsv);
	status = buildRowList(pCsv, &pCsv->predef, args);
	va_end (args);
	
	return status;
}

// Possible returns: OK, BUFFER_FULL
signed long csvBuildRowAs(struct CSVCore_typ* pCsv, struct CSVPredef_typ* pPredef, ...) {
	long status;
	
	va_list args;
	va_start (args, pPredef);
	status = buildRowList(pCsv, pPredef, args);
	va_end (args);
	
	return status;
}

// Possible returns: OK, BUFFER_FULL
signed long buildRowList(struct CSVCore_typ* pCsv, struct CSVPredef_typ* pPredef, va_list args) {
	register int i;
	void* pVar;
	BOOL truncated;
	long size, orgSize;
	long length = 0;
	TYPE_TYP* pTypes = (TYPE_TYP*)pPredef->pTypes;
	char delimString[3] = {pCsv->cfg.delim, ' ', '\0'};
	
	if(pCsv->internal.pRow == 0) pCsv->internal.pRow = pCsv->internal.pData;
	
	pCsv->out.pRow = pCsv->internal.pRow;
	orgSize = size = pCsv->internal.dataSize - (pCsv->internal.pRow - pCsv->internal.pData) - 1;
	
	// Populate args
	for (i = 0; i < pPredef->numColumns && size > 0; i++) {
		pVar = (void*)va_arg(args, unsigned long);
		if(i != 0)
			appendTo((char**)&pCsv->internal.pRow, delimString, &size);
		
		// Add arg
		getVarValue(pVar, (char*)pCsv->internal.pRow, size, pTypes[i], &length, &pCsv->cfg);
		pCsv->internal.pRow += length;
		size -= length; // Size may go negative becuase of stringlcpy populating length
	}
	
	size = max(size, 0); // Prevent size from being 0
	
	truncated = appendTo((char**)&pCsv->internal.pRow, LINE_ENDING, &size);
	
	pCsv->out.rowLen = pCsv->internal.rowLen = orgSize - size;
	
	if(!truncated) { // No truncation has occured
		pCsv->internal.rowNumber++;
	}
	else {
		return CSV_CORE_ERR_BUFFER_FULL;
	}
	
	pCsv->out.rowNumber = pCsv->internal.rowNumber;
	
	return 0;
}

// Possible returns: OK, BUFFER_FULL
signed long csvBuildComment(struct CSVCore_typ* pCsv, unsigned long pString) {
	BOOL truncated;
	long size, orgSize, len;
	
	if(pCsv->internal.pRow == 0) pCsv->internal.pRow = pCsv->internal.pData;
	
	pCsv->out.pRow = pCsv->internal.pRow;
	orgSize = size = pCsv->internal.dataSize - (pCsv->internal.pRow - pCsv->internal.pData) - 1;
	
	if(size < 1) return CSV_CORE_ERR_BUFFER_FULL;
	
	appendTo((char**)&pCsv->internal.pRow, pCsv->cfg.comment, &size);
	appendTo((char**)&pCsv->internal.pRow, " ", &size); // Append space
	len = escapeChar((char*)pCsv->internal.pRow, size, (char*)pString, size, 0);
	size -= len;
	pCsv->internal.pRow += len;
	truncated = appendTo((char**)&pCsv->internal.pRow, LINE_ENDING, &size);
	
	pCsv->out.rowLen = pCsv->internal.rowLen = orgSize - size;
	
	if(!truncated) { // No truncation has occured
		pCsv->internal.rowNumber++;
	}
	else {
		return CSV_CORE_ERR_BUFFER_FULL;
	}
	
	pCsv->out.rowNumber = pCsv->internal.rowNumber;
	
	return 0;
}

signed long csvBuildCell(struct CSVCore_typ* pCsv, unsigned long pVar,  enum CSV_CORE_TYPE_enum varType, plcbit appendEndOfLine) {
	return CSV_CORE_ERR_NOT_IMPLEMENTED;
}

signed long csvAppendRow(struct CSVCore_typ* pCsv, unsigned long pString) {
	return CSV_CORE_ERR_NOT_IMPLEMENTED;
}

// @param	string	Pointer to string for value to be appeneded to. Pointer will be moved to end of appneded data
// @param	value	String content to be appended to string
// @param	pSpace	Pointer to long containing remaining space left in string. This value will be modified by fn
signed long appendTo(char** string, char* value, long* pSpace) {
	if(*pSpace < 0) return 1;
	
	long len, valueLen;
	valueLen = stringlcpy((UDINT)*string, (UDINT)value, *pSpace);
	len = min(valueLen, *pSpace);
	*pSpace -= len;
	*string += len;
	return (valueLen > len);
}

// @param	pVar			Pointer to variable to get value from
// @param	value			Pointer to string to write value to
// @param	valueSize		Size of value to prevent overflow
// @param	dataType		Data type of pVar
// @param	pLength			Pointer to current length of value
// @param	cfg				Configuration to be used to populate string
// Converts pVar value into a string and populates value with string value
// Possible returns: OK, INTERNAL, BUFFER_FULL, UNSUPPORTED_TYPE, INVALID_TYPE
signed long getVarValue(void* pVar, char* value, long valueSize, enum CSV_CORE_TYPE_enum dataType, long* pLength, CSVCoreCfg_typ* cfg) {
	if(pVar == 0 || value == 0) return CSV_CORE_ERR_INTERNAL;
	
	if(dataType >= CSV_CORE_TYPE_STRINGN)
		dataType = CSV_CORE_TYPE_STRINGN;
	
	UDINT valueUdint;
	DINT valueDint;
	REAL valueReal;
	char tempValue[CSV_CORE_MAX_CHAR_REP];
	
	switch(dataType){
	
	
		//*******************************************************************************
		// BOOL																			*
		//*******************************************************************************
		case CSV_CORE_TYPE_BOOL:
		
			if( *(BOOL*)(pVar) == 0 ){
			
				*pLength = stringlcpy( (UDINT)value, (UDINT)"false", valueSize );
			
			}
			else if( *(BOOL*)(pVar) == 1 ){
			
				*pLength = stringlcpy( (UDINT)value, (UDINT)"true", valueSize );
			
			}
			else{
			
				// Invalid BOOL value.
				// Use convention of non-zero == TRUE for now
				*pLength = stringlcpy( (UDINT)value, (UDINT)"true", valueSize );
			
			}
		
			break;
	
	
		//*******************************************************************************
		// Integer types																*
		//*******************************************************************************
		case CSV_CORE_TYPE_SINT:
		
			valueDint=	*(SINT*)(pVar);
		
			brsitoa( valueDint, (UDINT)tempValue );
			*pLength = stringlcpy((UDINT)value, (UDINT)tempValue, valueSize);
		
			break;
	
	
		case CSV_CORE_TYPE_INT:
		
			valueDint=	*(INT*)(pVar);
		
			brsitoa( valueDint, (UDINT)tempValue );
			*pLength = stringlcpy((UDINT)value, (UDINT)tempValue, valueSize);
		
			break;
	
	
		case CSV_CORE_TYPE_DINT:
		case CSV_CORE_TYPE_TIME:
		
			valueDint=	*(DINT*)(pVar);
		
			brsitoa( valueDint, (UDINT)tempValue );
			*pLength = stringlcpy((UDINT)value, (UDINT)tempValue, valueSize);
		
			break;
	
	
		case CSV_CORE_TYPE_USINT:
		
			valueDint=	*(USINT*)(pVar);
		
			brsitoa( valueDint, (UDINT)tempValue );
			*pLength = stringlcpy((UDINT)value, (UDINT)tempValue, valueSize);
		
			break;
	
	
		case CSV_CORE_TYPE_UINT:
		
			valueDint=	*(UINT*)(pVar);
		
			brsitoa( valueDint, (UDINT)tempValue );
			*pLength = stringlcpy((UDINT)value, (UDINT)tempValue, valueSize);
		
			break;
	
	
		case CSV_CORE_TYPE_UDINT:
		case CSV_CORE_TYPE_DATE_AND_TIME:
		
			valueUdint=	*(UDINT*)(pVar);
		
			uitoa( valueUdint, (UDINT)tempValue );
			*pLength = stringlcpy((UDINT)value, (UDINT)tempValue, valueSize);
		
			break;
	
	
		//*******************************************************************************
		// REAL																			*
		//*******************************************************************************
		case CSV_CORE_TYPE_REAL:
		
			valueReal=	*(REAL*)(pVar);
		
			brsftoa( valueReal, (UDINT)tempValue );
			*pLength = stringlcpy((UDINT)value, (UDINT)tempValue, valueSize);
		
			break;
	
		//*******************************************************************************
		// LREAL																			*
		//*******************************************************************************
		case CSV_CORE_TYPE_LREAL:
		
			valueReal=	(REAL)*(LREAL*)(pVar);
		
			brsftoa( valueReal, (UDINT)tempValue );
			*pLength = stringlcpy((UDINT)value, (UDINT)tempValue, valueSize);
		
			break;
	
		//*******************************************************************************
		// STRING																		*
		//*******************************************************************************
		case CSV_CORE_TYPE_STRING:
		case CSV_CORE_TYPE_STRINGN:
			
			*pLength = optionalEscape(value, valueSize, pVar, valueSize, cfg->delim, cfg->quoteCharacter, cfg->forceEscaping);
		
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
