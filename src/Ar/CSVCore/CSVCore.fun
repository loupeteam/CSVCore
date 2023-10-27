(*
 * File: CSVCore.fun
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of CSVCore, licensed under the MIT License.
 * 
 *)

(*Predef*)

FUNCTION csvInitPredef : DINT (*Initializes a Predef*)
	VAR_INPUT
		pPredef : CSVPredef_typ; (*Pointer to predef to initialize*)
		numColumns : UDINT; (*Number of columns Predef will return*)
		pColumns : UDINT; (*Pointer to array of INT size numColumns (optional)*)
		pTypes : UDINT; (*Pointer to array of CSV_CORE_TYPE_enum size numColumns (optional)*)
		pStatus : UDINT; (*Pointer to array of CSV_CORE_ERR_enum size numColumns (optional)*)
	END_VAR
END_FUNCTION

FUNCTION csvFilterByHeader : DINT (*Apply a filter by header to Predef*)
	VAR_INPUT
		pCsv : REFERENCE TO CSVCore_typ; (*Pointer to csv object containing data*)
		pPredef : CSVPredef_typ; (*Pointer to predef to apply filter to *)
		varArgs : UDINT; (*Optional number of arguments*)
	END_VAR
END_FUNCTION

FUNCTION csvFilterByColumn : DINT (*Apply a filter by columns to Predef*)
	VAR_INPUT
		pPredef : CSVPredef_typ; (*Pointer to predef to apply filter to*)
		varArgs : UDINT; (*Optional number of arguments*)
	END_VAR
END_FUNCTION

FUNCTION csvCastAs : DINT (*Apply types definitions to columnns in Predef*)
	VAR_INPUT
		pPredef : CSVPredef_typ; (*Pointer to predef to apply filter to *)
		varArgs : UDINT; (*Optional number of arguments*)
	END_VAR
END_FUNCTION

FUNCTION csvCastAllAs : DINT (*Apply type definition to all calomns in Predef*)
	VAR_INPUT
		pPredef : CSVPredef_typ; (*Pointer to predef to apply filter to *)
		dataType : CSV_CORE_TYPE_enum; (*Data type that columns should be formatted as*)
	END_VAR
END_FUNCTION

FUNCTION csvSetPredef : DINT (*Set defualt Predef*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		pPredef : CSVPredef_typ; (*Pointer to predef to copy configuration from*)
	END_VAR
END_FUNCTION
(*Parsing*)

FUNCTION csvInitData : DINT (*Initialize csv object*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		pData : UDINT; (*Pointer to container to contain csv data*)
		dataSize : UDINT; (*Size of data container*)
	END_VAR
END_FUNCTION

FUNCTION csvReadNextRow : DINT (*Read next row using default Predef*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		varargs : UDINT; (*Optional number of arguments*)
	END_VAR
END_FUNCTION

FUNCTION csvReadRowAs : DINT (*ReRead row using Predef*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		pPredef : CSVPredef_typ; (*Pointer to predef to use while parsing row*)
		varargs : UDINT; (*Optional number of arguments*)
	END_VAR
END_FUNCTION

FUNCTION csvReadNextRowAs : DINT
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		pPredef : CSVPredef_typ; (*Pointer to predef to use while parsing row*)
		varargs : UDINT; (*Optional number of arguments*)
	END_VAR
END_FUNCTION
(*Building*)

FUNCTION csvBuildRow : DINT (*Build row and append to data*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		varArgs : UDINT; (*Optional number of arguments*)
	END_VAR
END_FUNCTION

FUNCTION csvBuildRowAs : DINT (*Build row from predef and append to data*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		pPredef : CSVPredef_typ; (*Pointer to predef to use while building row*)
		varArgs : UDINT; (*Optional number of arguments*)
	END_VAR
END_FUNCTION

FUNCTION csvBuildComment : DINT (*Build comment and append to data*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		pComment : UDINT; (*Pointer to string containg comment to be appeneded*)
	END_VAR
END_FUNCTION

FUNCTION csvBuildCell : DINT (*Build cell and append to data*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		pVar : UDINT; (*Pointer to variable containing data to be added row*)
		pType : CSV_CORE_TYPE_enum; (*Type of pVar*)
		appendEndOfLine : BOOL; (*Apply newline after cell*)
	END_VAR
END_FUNCTION

FUNCTION csvAppendRow : DINT (*Append row to data*)
	VAR_INPUT
		pCsv : CSVCore_typ; (*Pointer to csv object*)
		pRow : UDINT; (*Pointer to string containing row data*)
	END_VAR
END_FUNCTION
(*Advanced*)

FUNCTION csvGetLineLenFn : DINT (*get length of line*)
	VAR_INPUT
		pString : UDINT; (*Pointer to string*)
		pLen : UDINT; (*Pointer to UDINT to be populated with length*)
	END_VAR
END_FUNCTION

FUNCTION csvGetNextLineFn : DINT (*get next line*)
	VAR_INPUT
		ppString : UDINT; (*Pointer to Pointer to string. Pointer will be moved to start of next line*)
		pLen : UDINT; (*Pointer to UDINT to be populated with length (optional)*)
		skipEmptyLines : BOOL; (*Skip lines that contain no data*)
	END_VAR
END_FUNCTION

FUNCTION csvGetNLineFn : DINT (*get nth line*)
	VAR_INPUT
		ppString : UDINT; (*Pointer to Pointer to string. Pointer will be moved to start of next line*)
		pLen : UDINT; (*Pointer to UDINT to be populated with length (optional)*)
		lineNum : DINT; (*Zero indexed line number to get*)
		skipEmptyLines : BOOL; (*Skip lines that contain no data*)
	END_VAR
END_FUNCTION

FUNCTION csvGetCellLenFn : DINT (*get length of cell*)
	VAR_INPUT
		pString : UDINT; (*Pointer to string*)
		pLen : UDINT; (*Pointer to UDINT to be populated with length*)
		delim : USINT; (*Deliminator seperating cells*)
	END_VAR
END_FUNCTION

FUNCTION csvGetNextCellFn : DINT (*get next cell*)
	VAR_INPUT
		ppString : UDINT; (*Pointer to Pointer to string. Pointer will be moved to start of next cell*)
		pLen : UDINT; (*Pointer to UDINT to be populated with length (optional)*)
		delim : USINT; (*Deliminator seperating cells*)
	END_VAR
END_FUNCTION

FUNCTION csvGetNCellFn : DINT (*get nth cell*)
	VAR_INPUT
		ppString : UDINT; (*Pointer to Pointer to string. Pointer will be moved to start of next cell*)
		pLen : UDINT; (*Pointer to UDINT to be populated with length (optional)*)
		cellNum : DINT; (*Zero indexed cell number to get*)
		delim : USINT; (*Deliminator seperating cells*)
	END_VAR
END_FUNCTION
