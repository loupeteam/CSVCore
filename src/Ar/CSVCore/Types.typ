(*
 * File: Types.typ
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of CSVCore, licensed under the MIT License.
 * 
 *)


TYPE
	CSVCoreCfg_typ : 	STRUCT 
		delim : USINT := 44; (*Delimiter seperating cells. Defualt ','*)
		comment : STRING[5] := '//'; (*Characters representing line comments*)
		headerRow : UDINT := 0; (*Row number containing header. 0 means no header*)
		quoteCharacter : USINT := 34; (*Quote character to be used for building csv data. Defualt "*)
		quoteUnescaping : {REDUND_UNREPLICABLE} BOOL := TRUE; (*Unescape quoted strings*)
		forceEscaping : BOOL := FALSE; (*Force strings to be escaped when building csv data*)
		ignoreEmptyRows : BOOL := TRUE; (*Skip Rows that have no values. Not fully implemented*)
	END_STRUCT;
	CSVCoreOut_typ : 	STRUCT 
		headerRow : UDINT;
		rowNumber : UDINT;
		pRow : UDINT;
		rowLen : UDINT;
	END_STRUCT;
	CSVCore_typ : 	STRUCT 
		predef : CSVPredef_typ;
		cfg : CSVCoreCfg_typ;
		out : CSVCoreOut_typ;
		internal : CSVCoreInternal_typ;
	END_STRUCT;
	CSVCoreInternal_typ : 	STRUCT 
		rowNumber : DINT;
		pRow : UDINT;
		rowLen : UDINT;
		pData : UDINT;
		dataSize : UDINT;
	END_STRUCT;
	CSVPredef_typ : 	STRUCT 
		numColumns : USINT;
		pColumns : UDINT;
		pTypes : UDINT;
		pStatus : UDINT;
	END_STRUCT;
	CSV_CORE_ERR_enum : 
		(
		CSV_CORE_ERR_OK := 0,
		CSV_CORE_ERR_NO_NEWLINE := 5000,
		CSV_CORE_ERR_NEWLINE,
		CSV_CORE_ERR_END_OF_LINE,
		CSV_CORE_ERR_END_OF_DATA,
		CSV_CORE_ERR_BUFFER_FULL,
		CSV_CORE_ERR_HEADER_INDEX,
		CSV_CORE_ERR_NUM_COLUMNS,
		CSV_CORE_ERR_INVALID_INPUT,
		CSV_CORE_ERR_INTERNAL,
		CSV_CORE_ERR_COLUMN_NOT_FOUND,
		CSV_CORE_ERR_INVALID_VALUE,
		CSV_CORE_ERR_INVALID_TYPE,
		CSV_CORE_ERR_UNSUPPORTED_TYPE,
		CSV_CORE_ERR_NOT_IMPLEMENTED
		);
	CSV_CORE_TYPE_enum : 
		(
		CSV_CORE_TYPE_STRUCT := 0, (*0*)
		CSV_CORE_TYPE_BOOL,
		CSV_CORE_TYPE_SINT,
		CSV_CORE_TYPE_INT,
		CSV_CORE_TYPE_DINT,
		CSV_CORE_TYPE_USINT, (*5*)
		CSV_CORE_TYPE_UINT,
		CSV_CORE_TYPE_UDINT,
		CSV_CORE_TYPE_REAL,
		CSV_CORE_TYPE_STRING,
		CSV_CORE_TYPE_ULINT, (*10*)
		CSV_CORE_TYPE_DATE_AND_TIME,
		CSV_CORE_TYPE_TIME,
		CSV_CORE_TYPE_DATE,
		CSV_CORE_TYPE_LREAL,
		CSV_CORE_TYPE_ARRAY_OF_STRUCT, (*15*)
		CSV_CORE_TYPE_TIME_OF_DAY,
		CSV_CORE_TYPE_BYTE,
		CSV_CORE_TYPE_WORD,
		CSV_CORE_TYPE_DWORD,
		CSV_CORE_TYPE_LWORD, (*20*)
		CSV_CORE_TYPE_WSTRING, (*21*)
		CSV_CORE_TYPE_LINT := 23, (*23*)
		CSV_CORE_TYPE_STRINGN := 1000 (*1000*)
		);
END_TYPE
