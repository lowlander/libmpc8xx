/**
 * @file	mpc8xxmisc.c
 *
 * Copyright:	(c) 1999,2000,2001 VAS-EntwicklungsgesellschaftmbH
 *		(c) 2003 Erwin Rol Software Engineering
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 */

#include "mpc8xxmisc.h"
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

static void (*mpc8xx_printf_function)(const char*) = 0;
static int (*mpc8xx_query_function)(const char*) = 0;
static unsigned int mpc8xx_verbose_level_mask = 0;

signed long
mpc8xx_extract_signed_integer( const void *addr, int len , int big_endian )
{
	signed long retval;
	const unsigned char *p;
	const unsigned char *startaddr = addr;
	const unsigned char *endaddr = startaddr + len;

	if (len > (int) sizeof (signed long) )
		printf("That operation is not available on integers of more than %d bytes.",
				(int) sizeof (signed long) );


	/*
	 * Start at the most significant end of the integer, and work towards
	 * the least significant.
	 */

	if( big_endian )
	{
		p = startaddr;
		/*
		 * Do the sign extension once at the start.
		 */

		retval = ((signed long) * p ^ 0x80) - 0x80;
		for (++p; p < endaddr; ++p)
			retval = (retval << 8) | *p;
	}
	else
	{
		p = endaddr - 1;
		/*
		 * Do the sign extension once at the start.
		 */

		retval = ((signed long) * p ^ 0x80) - 0x80;
		for (--p; p >= startaddr; --p)
			retval = (retval << 8) | *p;
	}

	return retval;
}

unsigned long
mpc8xx_extract_unsigned_integer( const void *addr, int len , int big_endian)
{
	unsigned long retval;
	const unsigned char *p;
	const unsigned char *startaddr = addr;
	const unsigned char *endaddr = startaddr + len;

	if (len > (int) sizeof (unsigned long))
		printf("That operation is not available on integers of more than %d bytes.",
			(int) sizeof (unsigned long));


	/*
	 * Start at the most significant end of the integer, and work towards
	 * the least significant.
	 */

	retval = 0;
	if( big_endian )
	{
		for (p = startaddr; p < endaddr; ++p)
			retval = (retval << 8) | *p;
	}
	else
	{
		for (p = endaddr - 1; p >= startaddr; --p)
			retval = (retval << 8) | *p;
	}

	return retval;
}

void mpc8xx_set_print_function( void (*fn)(const char*) )
{
	mpc8xx_printf_function = fn;
}

void mpc8xx_printf( const char* format, ... )
{
	va_list args;
	char buffer[ 1024 ];
	size_t size = 1024;

	if( mpc8xx_printf_function != NULL ) 
	{
		va_start(args, format);

		vsnprintf( buffer, size, format, args );

		mpc8xx_printf_function( (const char*)buffer );

		va_end(args);
	}
}

void mpc8xx_set_query_function( int (*fn)(const char*) )
{
	mpc8xx_query_function = fn;
}

int mpc8xx_query( const char* format, ... )
{
	va_list args;
	char buffer[ 1024 ];
	size_t size = 1024;
	int res = 1;

	if( mpc8xx_query_function != NULL )
	{
		va_start(args, format);

		vsnprintf( buffer, size, format, args );

		res = mpc8xx_query_function( (const char*)buffer );

		va_end(args);
	}

	return res;
}

int mpc8xx_set_verbose_level( unsigned int level )
{
	mpc8xx_verbose_level_mask = level;

	return 0;
}

int mpc8xx_verbose_level( unsigned int level ){
	return (mpc8xx_verbose_level_mask & level) ? 1 : 0 ;
}

