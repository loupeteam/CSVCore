/********************************************************
*	NAME	: CsvCoreAPI.h
*	FUNCTION: Headerfile for CSV Core
*			  
*********************************************************/
//#define _CSVCORE_

#ifndef __CSVCOREAPI__
#define __CSVCOREAPI__

#define csvFilterByHeader(a, b, c) csvFilterByHeader(struct CSVCore_typ* pData, struct CSVPredef_typ* pPredef, ...);
#define csvFilterByColumn(a, b) csvFilterByColumn(struct CSVPredef_typ* pPredef, ...);
#define csvCastAs(a, b) csvCastAs(struct CSVPredef_typ* pPredef, ...);
#define csvReadNextRow(a, b) csvReadNextRow(struct CSVCore_typ* pData, ...);
#define csvReadRowAs(a, b, c) csvReadRowAs(struct CSVCore_typ* pData, struct CSVPredef_typ* pPredef, ...);
#define csvReadNextRowAs(a, b, c) csvReadNextRowAs(struct CSVCore_typ* pData, struct CSVPredef_typ* pPredef, ...);
#define csvBuildRow(a, b) csvBuildRow(struct CSVCore_typ* pData, ...);
#define csvBuildRowAs(a, b, c) csvBuildRowAs(struct CSVCore_typ* pData, struct CSVPredef_typ* pPredef, ...);
#include "CSVCore.h"
#undef csvFilterByHeader
#undef csvFilterByColumn
#undef csvCastAs
#undef csvReadNextRow
#undef csvReadRowAs
#undef csvReadNextRowAs
#undef csvBuildRow
#undef csvBuildRowAs

// include needed .h files
#include <stdarg.h>
#include <stdio.h>


#define COL_TYP INT
#define COL_INTERP_TYP DINT
#define TYPE_TYP DINT

// User facing prototypes
signed long csvGetLineLen(char* pString, unsigned long* pLen);
signed long csvGetNextLine(char** ppString, unsigned long* pLen, plcbit skipEmptyLines);
signed long csvGetNLine(char** ppString, unsigned long* pLen,  long lineNum, plcbit skipEmptyLines);
signed long csvGetCellLen(char* pString, unsigned long* pLen, char delim);
signed long csvGetNextCell(char** ppString, unsigned long* pLen, char delim);
signed long csvGetNCell(char** ppString, unsigned long* pLen, long cellNum, char delim);

#endif
