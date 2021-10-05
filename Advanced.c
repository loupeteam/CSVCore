
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

#include "StringExt.h"

#define COL_TYP INT
#define COL_INTERP_TYP DINT
#define TYPE_TYP DINT
#define ERR_TYP DINT

#define min( a, b ) ( ( (a) < (b)) ? (a) : (b) )

signed long csvGetLineLenFn(unsigned long pString, unsigned long pLen) {
	return csvGetLineLen((char*)pString, (unsigned long*)pLen);
}
signed long csvGetNextLineFn(unsigned long ppString, unsigned long pLen, plcbit skipEmptyLines) {
	return csvGetNextLine((char**)ppString, (unsigned long*)pLen, skipEmptyLines);
}
signed long csvGetNLineFn(unsigned long ppString, unsigned long pLen, signed long lineNum, plcbit skipEmptyLines) {
	return csvGetNLine((char**)ppString, (unsigned long*)pLen, lineNum, skipEmptyLines);
}
signed long csvGetCellLenFn(unsigned long pString, unsigned long pLen, unsigned char delim) {
	return csvGetCellLen((char*)pString, (unsigned long*)pLen, delim);
}
signed long csvGetNextCellFn(unsigned long ppString, unsigned long pLen, unsigned char delim) {
	return csvGetNextCell((char**)ppString, (unsigned long*)pLen, delim);
}
signed long csvGetNCellFn(unsigned long ppString, unsigned long pLen, signed long cellNum, unsigned char delim) {
	return csvGetNCell((char**)ppString, (unsigned long*)pLen, cellNum, delim);
}

// Gets length of line starting at pString
// returns err: INVALID_INPUT, NO_NEWLINE
// todo check to make sure this does not include new line
signed long csvGetLineLen(char* pString, unsigned long* pLen) {
	if(!pString || !pLen) return CSV_CORE_ERR_INVALID_INPUT;
	
	unsigned long len = 0;
	short newLineFound = 0;
	short inQuote = 0;
	char quote = 0;
	
	while(*pString) {
		if(*pString == '\n' || (*pString == '\r' && *(pString+1) == '\n')) {
			newLineFound = 1;
			break;
		}
		else if (*pString == '"' || *pString == '\'') {
			if(inQuote) {
				inQuote = !(quote == *pString);
			}
			else {
				inQuote = 1;
				quote = *pString;
			}
		}
		else if(*pString == '\\' && inQuote) {
			// TODO: we should talk about this
			// Skip past escaped chars in quotes
			len++;
			pString++;
		}
		len++;
		pString++;
	}
	
	*pLen = len;
	
	return newLineFound ? 0 : CSV_CORE_ERR_NO_NEWLINE;
}

// Get next line starting at *pString
// pString will be moved by this function
// returns err: INVALID_INPUT, END_OF_DATA, NO_NEWLINE
signed long csvGetNextLine(char** pString, unsigned long* pLen, plcbit skipEmptyLines) {
	if(!pString) return CSV_CORE_ERR_INVALID_INPUT;
	
	char * nextLine = *pString;
	unsigned long len;
	long status;
	status = csvGetLineLen(nextLine, &len);
	
	nextLine += len; // Move past current lines content
	if(nextLine[0] == '\r' && nextLine[1] == '\n') nextLine++; // \r\n counts as a single new line 
	
	while(skipEmptyLines && *nextLine && (nextLine[1] == '\r' || nextLine[1] == '\n')) nextLine++; // Walk past repeating new line chars
	
	if(*nextLine) nextLine++; // Get past new line char
	
	if(*nextLine) {
		*pString = nextLine;
		status = csvGetLineLen(nextLine, &len);
		if(pLen) *pLen = len;
		return status;
	}
	
	*pString = nextLine;
	if(pLen) *pLen = 0;
	return CSV_CORE_ERR_END_OF_DATA;
}

// returns err: INVALID_INPUT, END_OF_DATA, NO_NEWLINE
signed long csvGetNLine(char** pString, unsigned long* pLen,  long lineNum, plcbit skipEmptyLines) {
	if(!pString || !*pString) return CSV_CORE_ERR_INVALID_INPUT;
	
	char *line = *pString;
	unsigned long len;
	long status;
	
	if(lineNum < 1) {
		lineNum = 0;
		status = csvGetLineLen(*pString, &len);
	}
	
	while(status != CSV_CORE_ERR_END_OF_DATA && lineNum) {
		status = csvGetNextLine(&line, &len, skipEmptyLines);
		lineNum--;
	}
	
	*pString = line;
	if(pLen) *pLen = len;
	
	return status;
}

// returns err: INVALID_INPUT, NO_NEWLINE, NEWLINE
signed long csvGetCellLen(char* pString, unsigned long* pLen, char delim) {
	if(!pString || !pLen) return CSV_CORE_ERR_INVALID_INPUT;
	
	unsigned long len = 0;
	short delimFound = 0;
	short newLineFound = 0;
	short inQuote = 0;
	char quote = 0;
	
	while(*pString) {
		if(*pString == '\n' || *pString == '\r') {
			newLineFound = 1;
			break;
		}
		else if(*pString == delim && !inQuote) {
			delimFound = 1;
			break;
		}
		else if (*pString == '"' || *pString == '\'') {
			if(inQuote) {
				inQuote = !(quote == *pString);
			}
			else {
				inQuote = 1;
				quote = *pString;
			}
		}
		else if(*pString == '\\' && inQuote) {
			// Skip past escaped chars in quotes
			len++;
			pString++;
		}
		
		len++;
		pString++;
	}
	
	*pLen = len;
	
	if(newLineFound) return CSV_CORE_ERR_NEWLINE;
	else if(delimFound) return 0;
	else return CSV_CORE_ERR_NO_NEWLINE;
}

// returns err: INVALID_INPUT, NO_NEWLINE, NEWLINE, END_OF_DATA, END_OF_LINE
signed long csvGetNextCell(char** pString, unsigned long* pLen, char delim) {
	if(!pString) return CSV_CORE_ERR_INVALID_INPUT;
	
	unsigned long len;
	long status = csvGetCellLen(*pString, &len, delim);
	
	*pString += len;
	if(pLen) *pLen = 0;
	
	if(status == CSV_CORE_ERR_NO_NEWLINE) {
		return CSV_CORE_ERR_END_OF_DATA; // No next cell
	}
	else if(status == CSV_CORE_ERR_NEWLINE) {
		return CSV_CORE_ERR_END_OF_LINE; // End of line. No next cell in this row
	}
	
	if(**pString) (*pString)++; // Move past delim if we are not on a null char 
	
	status = csvGetCellLen(*pString, &len, delim);
	if(pLen) *pLen = len;
	return status;
}

// Not Tested
// returns err: INVALID_INPUT, NO_NEWLINE, NEWLINE, END_OF_DATA, END_OF_LINE
signed long csvGetNCell(char** pString, unsigned long* pLen, long cellNum, char delim) {
	if(!pString || !*pString) return CSV_CORE_ERR_INVALID_INPUT;
	
	char* cell = *pString;
	unsigned long len;
	long status = 0;
	
	if(cellNum < 1) {
		cellNum = 0;
		status = csvGetCellLen(*pString, &len, delim);
	}
	
	while(status != CSV_CORE_ERR_END_OF_DATA && status != CSV_CORE_ERR_END_OF_LINE && cellNum) {
		status = csvGetNextCell(&cell, &len, delim);
		cellNum--;
	}
	
	*pString = cell;
	if(pLen) *pLen = len;
	
	return status;
}
