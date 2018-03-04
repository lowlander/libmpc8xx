/**
 * @file	mpc8xxbdm.c
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

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "lptbdm.h"
#include "mpc8xxbdm.h"
#include "mpc8xxspr.h"
#include "mpc8xxmisc.h"

unsigned int mpc8xx_get_gpr( int reg_nr )
{
	bdm_in_t in;
	bdm_out_t out;

	reg_nr &= ~MPC8XX_GPR_REG_MASK;

	/*
	 * DPDR = SPR 630 = 10011 10110 
	 * spr field in instruction is split field = SPR[5-9] || SPR[0-4]
	 * -> spr = 10110 10011 = 723
	 */


	/* mtspr DPDR,rreg */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = ( 31 << 26 ) | ( reg_nr << 21 ) | ( 723 << 11 ) | ( 467 << 1 );
	
	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return 0xDEADBEEF;
	}

	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = MPC8XX_COM_NOP;

	/*get data*/
	if( mpc8xx_bdm_clk_serial( &in, &out ) <  0 ){
		return 0xDEADBEEF;
	}

	return out.data;
}

int mpc8xx_set_gpr( int reg_nr, unsigned int value )
{
	bdm_in_t in;
	bdm_out_t out;

	reg_nr &= ~MPC8XX_GPR_REG_MASK;

	/* mtspr DPDR,rreg */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (31 << 26) | (reg_nr << 21) | (723 << 11) | (339 << 1);

	if( mpc8xx_bdm_clk_serial( &in, &out ) <  0){
		return -1;
	}

	in.prefix = MPC8XX_BDM_PREFIX_CORE_DATA;
	in.data = value;

	/* set data */
	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	return 0;
}


unsigned int mpc8xx_get_msr( )
{
	unsigned int r0;
	unsigned int spr;
	bdm_in_t in;
	bdm_out_t out;

	/* save r0 */
	r0 = mpc8xx_get_gpr( 0 ); 

	/* mfmsr r0 */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (31 << 26) | (83 << 1);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return 0xDEADBEEF;
	}

	/* get r0 */
	spr = mpc8xx_get_gpr( 0 );

	/* restore r0 */
	if( mpc8xx_set_gpr( 0, r0 ) < 0 ){
		return 0xDEADBEEF;
	}

	return spr;
}

int mpc8xx_set_msr( unsigned int value )
{
	unsigned int r0;
	bdm_in_t in;
	bdm_out_t out;

	/* save r0 */
	r0 = mpc8xx_get_gpr( 0 );

	/* load r0 */
	if( mpc8xx_set_gpr( 0, value ) < 0 ){
		return -1;
	}
	
	/*mtmsr r0*/
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (31 << 26) | (146 << 1);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	/* restore r0 */
	mpc8xx_set_gpr( 0, r0 );

	return 0;
}

unsigned int mpc8xx_get_cr( )
{
	unsigned int r0;
	unsigned int spr;
	bdm_in_t in;
	bdm_out_t out;

	/* save r0 */
	r0 = mpc8xx_get_gpr(  0 );

	/* mfcr r0 */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data =  (31 << 26) | (19 << 1);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return 0xDEADBEEF;
	}

	/* get r0 */
	spr = mpc8xx_get_gpr(  0 );

	/* restore r0 */
	mpc8xx_set_gpr(  0, r0 );

	return spr;
}

int mpc8xx_set_cr(   unsigned int value )
{
	unsigned int r0;
	bdm_in_t in;
	bdm_out_t out;

	/* save r0 */
	r0 = mpc8xx_get_gpr(  0 );

	/* set r0 */
	mpc8xx_set_gpr(  0, value );


	/* mtcrf 0xff,r0 */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data =  (31 << 26) | (0xFF << 12) | (144 << 1);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	/* restore r0 */
	mpc8xx_set_gpr(  0, r0 );

	return 0;
}

unsigned int mpc8xx_get_spri(   int reg )
{
	unsigned int immr;
	unsigned int r0;
	unsigned int r1;
	unsigned int spri;
	bdm_in_t in;
	bdm_out_t out;

	immr = mpc8xx_get_spr(  MPC8XX_SPR_IMMR ); /*get offset for memory mapped sprs*/

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, ( immr & 0xFFFF0000 ) + reg ); /*load r1 with adr for spri*/

	/* lwz r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data =  (32 << 26) | (1 << 16);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return 0xDEADBEEF;
	}

	spri = mpc8xx_get_gpr(  0 ); /*get spri value from r0*/

	mpc8xx_set_gpr(  1, r1 ); /*restore r1*/
	mpc8xx_set_gpr(  0, r0 ); /*restore r0*/

	return spri;
}

int mpc8xx_set_spri(   int reg, unsigned int value )
{
	unsigned int immr;
	unsigned int r0;
	unsigned int r1;
	bdm_in_t in;
	bdm_out_t out;

	immr = mpc8xx_get_spr(  MPC8XX_SPR_IMMR ); /*get offset for memory mapped sprs*/

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, ( immr & 0xFFFF0000 ) + reg ); /*load r1 with adr for spr*/
	mpc8xx_set_gpr(  0, value ); /*set r0 with value*/

	/* stw r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data =  (36 << 26) | (1 << 16);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	mpc8xx_set_gpr(  1, r1 ); /*restore r1*/
	mpc8xx_set_gpr(  0, r0 ); /*restore r0*/

	return 0;
}

int mpc8xx_set_spri_hw(   int reg, unsigned int value )
{
	unsigned int immr;
	unsigned int r0;
	unsigned int r1;
	bdm_in_t in;
	bdm_out_t out;

	immr = mpc8xx_get_spr(  MPC8XX_SPR_IMMR); /*get offset for memory mapped sprs*/
	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, ( immr & 0xFFFF0000 ) + reg ); /*load r1 with adr for spr*/
	mpc8xx_set_gpr(  0, value ); /*set r0 with data*/

	/* sth r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data =  (44 << 26) | (1 << 16);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	mpc8xx_set_gpr(  1, r1 ); /* restore r1 */
	mpc8xx_set_gpr(  0, r0 ); /* restore r0 */

	return 0;
}

unsigned int mpc8xx_get_spr(   int reg )
{
	unsigned int r0;
	unsigned int spr;
	bdm_in_t in;
	bdm_out_t out;

	/* special for msr */
	if(reg == MPC8XX_SPR_MSR)
		return mpc8xx_get_msr( ); 

	/* special for cr */
	if(reg == MPC8XX_SPR_CR)
		return mpc8xx_get_cr( );

	/* memory mapped sprs */
	if(reg & MPC8XX_SPRI_MASK)
		return mpc8xx_get_spri(  reg & ~MPC8XX_SPRI_MASK );


	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/

	/* mfspr r0, SPRreg */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (31 << 26) | ((((reg&0x1f) << 5) | ((reg >> 5)&0x1f)) << 11) | (339 << 1);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return 0xDEADBEEF;
	}

	spr = mpc8xx_get_gpr(  0 ); /*read r0*/
	mpc8xx_set_gpr(  0, r0 ); /*restore r0*/

	return spr;
}

int mpc8xx_set_spr(   int reg, unsigned int value )
{
	unsigned int r0;
	bdm_in_t in;
	bdm_out_t out;

	/* special for msr */
	if(reg == MPC8XX_SPR_MSR){
		return mpc8xx_set_msr(  value );
	}

	/* special for cr */
	if(reg == MPC8XX_SPR_CR){
		return mpc8xx_set_cr(  value ); 
	}
	/* memory mapped sprs */
	if(reg & MPC8XX_SPRI_MASK){
		return mpc8xx_set_spri(  reg & ~MPC8XX_SPRI_MASK, value );
	}

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/

	mpc8xx_set_gpr(  0, value );

	/*mtspr SPRreg,r0*/
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (31 << 26) | ((((reg&0x1f) << 5) | ((reg >> 5)&0x1f)) << 11) | (467 << 1);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	mpc8xx_set_gpr(  0, r0 ); /*restore r0*/

	return 0;
}

unsigned int mpc8xx_get_word(   unsigned int addr )
{
	unsigned int r0;
	unsigned int r1;
	unsigned int val;
	bdm_in_t in;
	bdm_out_t out;

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, addr ); /*set addr to r1*/

	/* lwz r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (32 << 26) | (1 << 16);;

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return 0xDEADBEEF;
	}

	val = mpc8xx_get_gpr(  0 ); /*get data from r0*/

	mpc8xx_set_gpr( 0, r0 ); /*restore data*/
	mpc8xx_set_gpr( 1, r1 ); /*restore data*/

	return val;
}

int mpc8xx_set_word( unsigned int addr, unsigned int val )
{
	unsigned int r0;
	unsigned int r1;
	bdm_in_t in;
	bdm_out_t out;

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, addr ); /*put addr in r1*/
	mpc8xx_set_gpr(  0, val ); /*put data in r0*/

	/* stw r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (36 << 26) | (1 << 16);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	mpc8xx_set_gpr(  0, r0); /*restore r0*/
	mpc8xx_set_gpr(  1, r1); /*restore r1*/

	return 0;
}

unsigned int mpc8xx_get_halfword( unsigned int addr )
{
	unsigned int r0;
	unsigned int r1;
	unsigned int val;
	bdm_in_t in;
	bdm_out_t out;

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, addr ); /*set addr to r1*/

	/* lwz r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (40 << 26) | (1 << 16);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return 0xDEADBEEF;
	}

	val = mpc8xx_get_gpr(  0 ); /*get data from r0*/

	mpc8xx_set_gpr( 0, r0 ); /*restore data*/
	mpc8xx_set_gpr( 1, r1 ); /*restore data*/

	return val;
}

int mpc8xx_set_halfword( unsigned int addr, unsigned int val )
{
	unsigned int r0;
	unsigned int r1;
	bdm_in_t in;
	bdm_out_t out;

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, addr ); /*put addr in r1*/
	mpc8xx_set_gpr(  0, val ); /*put data in r0*/

	/* stw r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (44 << 26) | (1 << 16);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	mpc8xx_set_gpr(  0, r0); /*restore r0*/
	mpc8xx_set_gpr(  1, r1); /*restore r1*/

	return 0;
}

unsigned int mpc8xx_get_byte( unsigned int addr )
{
	unsigned int r0;
	unsigned int r1;
	unsigned int val;
	bdm_in_t in;
	bdm_out_t out;

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, addr ); /*set addr to r1*/

	/* lbz r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (34 << 26) | (1 << 16);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return 0xDEADBEEF;
	}

	val = mpc8xx_get_gpr(  0 ); /*get data from r0*/

	mpc8xx_set_gpr( 0, r0 ); /*restore data*/
	mpc8xx_set_gpr( 1, r1 ); /*restore data*/

	return val;
}

int mpc8xx_set_byte( unsigned int addr, unsigned int val )
{
	unsigned int r0;
	unsigned int r1;
	bdm_in_t in;
	bdm_out_t out;

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	mpc8xx_set_gpr(  1, addr ); /*put addr in r1*/
	mpc8xx_set_gpr(  0, val ); /*put data in r0*/

	/* stb r0,0(r1) */
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = (38 << 26) | (1 << 16);

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	mpc8xx_set_gpr( 0, r0); /*restore r0*/
	mpc8xx_set_gpr( 1, r1); /*restore r1*/

	return 0;
}


int mpc8xx_read_block( unsigned int from_address, unsigned char* to_buffer, unsigned int len )
{
	unsigned int val;
	unsigned int r0;
	unsigned int r1;
	bdm_in_t in;
	bdm_out_t out;

	r0 = mpc8xx_get_gpr(  0 ); /*save r0*/
	r1 = mpc8xx_get_gpr(  1 ); /*save r1*/

	/* no word aligned start adr? */
	while( ( (from_address & 3) || len < 4 ) && len > 0 ) 
	{
		mpc8xx_set_gpr(  1, from_address );
		from_address += 1;

		/* lbz r0,0(r1) */
		in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
		in.data = 0x88010000;

		if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
			return -1;
		}

		val = mpc8xx_get_gpr(  0 );	/* read byte*/

		*to_buffer = val & 0xFF;		/* store byte*/

		to_buffer += 1;

		len--;
	}

	/* lwzu will increment and then load */
	if( len > 3 )
		mpc8xx_set_gpr(  1, from_address - 4 );

	/* loop for aligned words */
	for( ; len > 3; len -= 4, to_buffer += 4, from_address += 4 ) 
	{
		/* lwzu r0,4(r1) */
		in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
		in.data = 0x84010004;

		if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
			return -1;
		}

		val = mpc8xx_get_gpr(  0 );	/* read word*/

		val = mpc8xx_extract_unsigned_integer( &val, sizeof(val), 1 );

		memcpy(to_buffer, &val, 4);			/* store word */
	}

	/* misaligned bytes to read at end? */
	while( len > 0 && len < 4 )	
	{
		mpc8xx_set_gpr( 1, from_address );
		from_address += 1;

		/* lbz r0,0(r1) */
		in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
		in.data = 0x88010000;

		if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
			return -1;
		}

		val = mpc8xx_get_gpr( 0 );		/*read byte*/

		*to_buffer = val & 0xFF;	/*store byte*/

		to_buffer += 1;

		len--;
	}

	mpc8xx_set_gpr( 0, r0); /*restore r0*/
	mpc8xx_set_gpr( 1, r1); /*restore r1*/

	return 0;
}

int mpc8xx_write_block( unsigned int to_address, unsigned char *from_buffer, unsigned int len )
{
	unsigned int val;
	unsigned int r30;
 	unsigned int r31;
	bdm_in_t in;
	bdm_out_t out;

	r30 = mpc8xx_get_gpr(  30 );
	r31 = mpc8xx_get_gpr(  31 );

	while( ((to_address & 3) || ( len < 4 )) && ( len > 0 ) )
	{
		mpc8xx_set_gpr(  30, to_address );
		to_address += 1;

		val = *from_buffer;
		from_buffer += 1;

		mpc8xx_set_gpr(  31, val );

		/* stb r31,0(r30) */
		in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
		in.data = 0x9BFE0000;

		if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
			return -1;
		}

		len--;
	}

	if( len > 3 )		/*for aligned words use BDM download feature*/
	{
		mpc8xx_set_gpr(  30, to_address - 4 );

		in.prefix = MPC8XX_BDM_PREFIX_DPC;
		in.data = MPC8XX_BDM_DPC_START_DOWNLOAD;

		mpc8xx_bdm_clk_serial( &in, &out );

		for(; len > 3; len -= 4, to_address += 4, from_buffer += 4 )
		{
			memcpy( &val, from_buffer, 4 );
			val = mpc8xx_extract_unsigned_integer( &val, sizeof(val), 1);

			in.prefix = MPC8XX_BDM_PREFIX_CORE_DATA;
			in.data = val;

			if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
				return -1;
			}
		}

		in.prefix = MPC8XX_BDM_PREFIX_DPC;
		in.data = MPC8XX_BDM_DPC_END_DOWNLOAD;

		if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
			return -1;
		}

		in.prefix = MPC8XX_BDM_PREFIX_CORE_DATA;
		in.data = 0;

		if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
			return -1;
		}
	}

	while( len > 0 )
	{
		mpc8xx_set_gpr( 30, to_address );
		to_address += 1;

		val = *from_buffer;
		from_buffer += 1;

		mpc8xx_set_gpr( 31, val);

		/* stb r31,0(r30) */
		in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
		in.data = 0x9BFE0000;

		if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
			return -1;
		}

		len--;
	}

	mpc8xx_set_gpr(  30, r30 );
	mpc8xx_set_gpr(  31, r31 );

	return 0;
}

int mpc8xx_hreset( )
{
	bdm_in_t in;
	bdm_out_t out;

	in.prefix =MPC8XX_BDM_PREFIX_DPC;
	in.data = MPC8XX_BDM_DPC_HRESET;

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	return 0;
}

int mpc8xx_sreset( )
{
	bdm_in_t in;
	bdm_out_t out;

	in.prefix =MPC8XX_BDM_PREFIX_DPC;
	in.data = MPC8XX_BDM_DPC_SRESET;

	if ( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		return -1;
	}

	return 0;
}

int mpc8xx_interrupt( int timeout )
{
	bdm_in_t in;
	bdm_out_t out;

	in.prefix = MPC8XX_BDM_PREFIX_DPC;
	in.data = MPC8XX_BDM_DPC_ASSERT_NMASK_BREAK;

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 )
	{
		return -1;
	}

	if( mpc8xx_bdm_wait_freeze( timeout ) < 0 )
	{
		return -1;
	}

	in.prefix = MPC8XX_BDM_PREFIX_DPC;
	in.data = MPC8XX_BDM_DPC_NEGATE_NMASK_BREAK;

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 )
	{
		return -1;
	}

	return 0;
}

int mpc8xx_resume( )
{
	unsigned int icr;
	bdm_in_t in;
	bdm_out_t out;

	mpc8xx_set_spr( MPC8XX_SPR_IC_CST, 0x0C000000); /*invalidate instruction cache*/

	icr = mpc8xx_get_spr( MPC8XX_SPR_ICR); /*read ICR to clear pending interrupts*/

	if( mpc8xx_verbose_level( MPC8XX_VERBOSE_BDM )  )
	{
		mpc8xx_printf("ICR = %08x\n", icr );
	}

	/*tell PPC to exit from Debug Port Interrupt*/
	in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
	in.data = MPC8XX_COM_RFI;

	if( mpc8xx_bdm_clk_serial( &in, &out ) < 0 ){
		mpc8xx_printf("RFI error\n");

		return -1;
	}

	return 0;
}

int mpc8xx_continue_single_step( )
{
	unsigned int der;
	unsigned int srr1;
	der = mpc8xx_get_spr( MPC8XX_SPR_DER ); /*save previous Debug Enable Register value*/
	mpc8xx_set_spr( MPC8XX_SPR_DER, der | PPC_BIT(14) ); /* force TRE for single step*/

	/* set single step trace enable in next MSR */
	srr1 = mpc8xx_get_spr( MPC8XX_SPR_SRR1 );
	srr1 &= ~ ( PPC_BIT(21) | PPC_BIT(22) );
	srr1 |= PPC_BIT(21);
	mpc8xx_set_spr( MPC8XX_SPR_SRR1, srr1 );
	
	return mpc8xx_resume( );
}

int mpc8xx_continue_until_branch( )
{
	unsigned int der;
	unsigned int srr1;

	der = mpc8xx_get_spr( MPC8XX_SPR_DER ); /*save previous Debug Enable Register value*/
	mpc8xx_set_spr( MPC8XX_SPR_DER, der | PPC_BIT(14) ); /* force TRE for single step*/

	/* set single step trace enable in next MSR */
	srr1 = mpc8xx_get_spr(  MPC8XX_SPR_SRR1 );
	srr1 &= ~ ( PPC_BIT(21) | PPC_BIT(22) );
	srr1 |= PPC_BIT(22);
	mpc8xx_set_spr( MPC8XX_SPR_SRR1, srr1 );

	return mpc8xx_resume( );
}

int mpc8xx_continue(  )
{
	unsigned int der;
	unsigned int srr1;

	der = mpc8xx_get_spr(  MPC8XX_SPR_DER ); /*save previous Debug Enable Register value*/
	der &= ~PPC_BIT(14);
	mpc8xx_set_spr(  MPC8XX_SPR_DER, der );

	/* clear single step trace enable in next MSR */
	srr1 = mpc8xx_get_spr(  MPC8XX_SPR_SRR1 );
	srr1 &= ~ ( PPC_BIT(21) | PPC_BIT(22) );

	mpc8xx_set_spr(  MPC8XX_SPR_SRR1, srr1 );

	return mpc8xx_resume( );
}





