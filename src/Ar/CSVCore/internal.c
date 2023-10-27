/*
 * File: internal.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of CSVCore, licensed under the MIT License.
 * 
 */

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
#include <string.h>

// Will copy pSrc into pDest and will escape it with quotes if necessary (contains any characters that would normally need to be escaped or character)
// @param	pDest		String where data should be copied
// @param	destSize	Size of pDest
// @param	pSrc		Original string to be copied
// @param	maxChars	Maxium characters to be copied. Useful if pSrc is not Null terminated
// @param	character	Additional character that would need to be escaped. If none pass 0
// @param	quoteChar	Quote character to be used if escaped: ' or "
// @param	forceEscape	If true, string will be quoted and escaped regardless of content
unsigned long optionalEscape(char* pDest, unsigned long destSize, char* pSrc, unsigned long maxChars, char character, char quoteChar, unsigned short forceEscape) {
	char *ptr = pSrc;
	unsigned short escapeFound = forceEscape;
	unsigned long length;
	
	// If string starts or ends with space we need to escape
	// This is because parser will remove any prefix or suffix char that matches isspace fn
	// Remaining whitespace is handled below becuase we always want to escape those
	if(*ptr == ' ') escapeFound = 1;
	
	while(*ptr && maxChars > (ptr - pSrc) && !escapeFound) {
		if(*ptr == character)
			escapeFound = 1;
			
		switch (*ptr) {
			case '\'':
			case '\"':
			case '\f':
			case '\n':
			case '\r':
			case '\t':
			case '\v':
				escapeFound = 1;
				break;
			default:
				break;
		}
		ptr++;
	}
	
	// If string starts or ends with space we need to escape
	// This is because parser will remove any prefix or suffix char that matches isspace fn
	// Remaining whitespace is handled above becuase we always want to escape those
	if(pSrc != ptr && *(ptr-1) == ' ') escapeFound = 1;
	
	if(escapeFound) {
		if(quoteChar) {
			length = 2;
			pDest[0] = quoteChar;
			length += escapeChar(pDest+1, destSize-1, pSrc, maxChars, quoteChar);
			pDest[length-1] = quoteChar;
			pDest[length] = '\0';
		}
		else {
			// If no quoteChar do not surrond in quotes
			length = escapeChar(pDest, destSize, pSrc, maxChars, quoteChar);
		}
	}
	else {
		length = stringlcpy((UDINT)pDest, (UDINT)pSrc, destSize);
		if(length >= destSize)
			length = !destSize ? destSize - 1 : 0;
	}
	
	return length;
}

// Will copy pSrc into pDest and will escape it without quotes
// @param	pDest		String where data should be copied
// @param	destSize	Size of pDest
// @param	pSrc		Original string to be copied and escaped
// @param	maxChars	Maxium characters to be copied. Useful if pSrc is not Null terminated
// @param	quoteChar	Quote character to be used if escaped: ' or "
unsigned long escapeChar(char* pDest, unsigned long destSize, char* pSrc, unsigned long maxChars, char quoteChar) {
	char *ptr = pSrc;
	char *ptr2 = pDest;
	
	while(*ptr && maxChars > (ptr - pSrc) && destSize-1 > (ptr2 - pDest)) {
		if(quoteChar == *ptr) {
			*ptr2++ = '\\';
			*ptr2++ = quoteChar;
		}
		else {	
			switch (*ptr) {
			
				case '\\': *ptr2++ = '\\'; *ptr2++ = '\\'; break;
				case '\b': *ptr2++ = '\\'; *ptr2++ = 'b'; break;
				case '\f': *ptr2++ = '\\'; *ptr2++ = 'f'; break;
				case '\n': *ptr2++ = '\\'; *ptr2++ = 'n'; break;
				case '\r': *ptr2++ = '\\'; *ptr2++ = 'r'; break;
				case '\t': *ptr2++ = '\\'; *ptr2++ = 't'; break;
				case '\v': *ptr2++ = '\\'; *ptr2++ = 'v'; break;
				default:  *ptr2++ = *ptr; break;
			}
		}
		ptr++;
	}
	
	*ptr2 = 0; // null terminate ptr2 string
	return (ptr2 - pDest);
}

// Will copy pSrc into pDest and will unescape it. Expected pSrc to alread be dequoted
// @param	pDest		String where data should be copied
// @param	destSize	Size of pDest
// @param	pSrc		Original string to be copied and unescaped
// @param	maxChars	Maxium characters to be copied. Useful if pSrc is not Null terminated
// @param	quoteChar	Quote character to be used if escaped: ' or "
unsigned long unescapeChar(char* pDest, unsigned long destSize, char* pSrc, unsigned long maxChars, char quoteChar) {
	char *ptr = pSrc;
	char *ptr2 = pDest;
	
	while(*ptr && maxChars > (ptr - pSrc) && destSize-1 > (ptr2 - pDest)) {
		// Copy characters while checking for escape sequences
		if (*ptr != '\\') {
			
			*ptr2++ = *ptr++; // Not an escape sequence. Copy char and move on.
		
		} else {
		
			ptr++;
			if(*ptr == quoteChar) {
				*ptr2++ = quoteChar;
			}
			else {
				switch (*ptr) {
				
					case '\\': *ptr2++ = '\\';	break;
					case '/': *ptr2++ = '/';	break;
					case 'b': *ptr2++ = '\b';	break;
					case 'f': *ptr2++ = '\f';	break;
					case 'n': *ptr2++ = '\n';	break;
					case 'r': *ptr2++ = '\r';	break;
					case 't': *ptr2++ = '\t';	break;
					case 'v': *ptr2++ = '\v';	break;
				
					default: *ptr2++ = '\\'; *ptr2++ = *ptr; break;
				
				}
			}
		
			ptr++;
		
		} // escape sequence
	}
	
	*ptr2 = 0; // null terminate ptr2 string
	return (ptr2 - pDest);
}

