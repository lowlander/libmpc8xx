/**
 * @file	mpc8xxmisc.h
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

#ifndef __MPC8XXMISC_H__
#define __MPC8XXMISC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MPC8XX_VERBOSE_INFO	0x001
#define MPC8XX_VERBOSE_BDM	0x002
#define MPC8XX_VERBOSE_SER	0x004
#define MPC8XX_VERBOSE_BRK	0x008
#define MPC8XX_VERBOSE_MMU	0x010
#define MPC8XX_VERBOSE_TAR	0x020
#define MPC8XX_VERBOSE_DCA	0x040
#define MPC8XX_VERBOSE_FLS	0x080
#define MPC8XX_VERBOSE_SRR1	0x100

/** Number of bits on the target
 */
#define PPC_BITS	32

/** Covert target bit number to bit mask
 * @param x target bit number ( 0 to 31 )
 * @return target bit mask ( 0xf0000000 to 0x00000001 )
 */
#define PPC_BIT(x)	( 1 << (PPC_BITS-1-(x)) )

/** Endian convert a 2 or 4 byte signed int
 * @param addr pointer to the signed int
 * @param len length of the signed int
 * @param big_endian 1 when the target is big endian
 * @return the converted signed int
 */
extern signed long mpc8xx_extract_signed_integer( const void *addr, int len , int big_endian );

/** Endian convert a 2 or 4 byte unsigned int
 * @param addr pointer to the unsigned int
 * @param len length of the unsigned int
 * @param big_endian 1 when the target is big endian
 * @return the converted unsigned int
 */
extern unsigned long mpc8xx_extract_unsigned_integer( const void *addr, int len , int big_endian );

/** Set the print function
 * @param fn function pointer to fucntion taking a const char* and returning void
 */
extern void mpc8xx_set_print_function( void (*fn)(const char*) );

/** Print function
 * This function uses the function set with mpc8xx_set_print_function to do
 * its printing, that way a program can select where the text will end up.
 * @param format printf like format string
 */
extern void mpc8xx_printf( const char* format, ... );

/** Set the query function
 * @param fn function pointer to fucntion taking a const char* and returning an int
 */
extern void mpc8xx_set_query_function( int (*fn)(const char*) );

/** Query function
 * This function uses the function set with mpc8xx_set_query_function to ask
 * the user yes/no questions, that way some commands can be handled interactivly.
 * @param format printf like format string
 * @return 1 if the answer is yes 0 if the answer is no
 */
extern int mpc8xx_query( const char* format, ... );

/** Set the verbose level
 * @param level verbose level
 * @return -1 on error 0 on success
 */
extern int mpc8xx_set_verbose_level( unsigned int level );

/** Check the verbose level
 * @param level verbose level
 * @return 1 if the level is set else 0
 */
extern int mpc8xx_verbose_level( unsigned int level );


#ifdef __cplusplus
}
#endif

#endif /* !__MPC8XXMISC_H__ */
