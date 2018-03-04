/**
 * @file	mpc8xxtarget.c
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

#include "mpc8xxtarget.h"
#include "mpc8xxbdm.h"
#include "mpc8xxspr.h"
#include "lptbdm.h"
#include "mpc8xxmisc.h"

/*****************************************************************
* remote target execution helpers                                *
*****************************************************************/

/*****************************************************************/

int mpc8xx_target_prepare( mpc8xx_target_registers_t* regs )
{
	int i;

	/*
	 * backup the GPR's
	 */
	for (i = 0; i < 32; i++)
	{
		regs->gpr[i] = mpc8xx_get_gpr( i ); /* save GPR0..31 */
	}

	regs->ctr = mpc8xx_get_spr( MPC8XX_SPR_CTR );	/*save CTR*/
	regs->srr0 = mpc8xx_get_spr( MPC8XX_SPR_SRR0 );	/*save srr0*/
	regs->srr1 = mpc8xx_get_spr( MPC8XX_SPR_SRR1 );	/*save srr1*/
	regs->der = mpc8xx_get_spr( MPC8XX_SPR_DER );	/*save DER*/

	return 0;
}

int mpc8xx_target_load( mpc8xx_target_program_t* program )
{
	unsigned int i;

	program->program_valid = 0;

	/*
	 * copy ram content into buffer, and target program into ram, if any
	 */
	for( i = 0; i < program->program_len ; i++ )
	{
		if(program->mem_backup != 0)
			program->mem_backup[ i ] = mpc8xx_get_word( program->start_address + i * 4 );

		mpc8xx_set_word( program->start_address + i * 4, program->program_code[ i ] );

		/* verify */

		if( mpc8xx_get_word( program->start_address + i * 4 ) != program->program_code[ i ] ) {
			/*
			 * Error, restore the memory we did until now
			 */

			for( ; i > 0 ; i-- ) {
				mpc8xx_set_word( program->start_address + (i-1) * 4, program->mem_backup[ (i-1) ] );
			}

			return -1;
		}
	}

	program->program_valid = 1;


	/*
	 * target processor is now ready to execute program with target_execute()
	 */

	return 0;
}

/*****************************************************************/

int mpc8xx_target_execute( mpc8xx_target_program_t* program, int timeout )
{
	if( program->program_valid != 1 ){
		return -1;
	}

	/*
	 * setup processor for program
	 */
	mpc8xx_set_spr( MPC8XX_SPR_SRR0 , program->start_address );	/* start address */
	mpc8xx_set_spr( MPC8XX_SPR_SRR1 , 0x1000 );		/* new MSR:enable only ME */
	mpc8xx_set_spr( MPC8XX_SPR_DER , 0x7002400f );		/* allow SEIE, to return to gdb*/

	mpc8xx_continue();	/* start target target NIP = preloaded SRR0 = mpxbdm_pram */

	/*
	 * wait until target target reaches 0x0 -> SEI
	 */
	if( mpc8xx_bdm_wait_freeze( timeout ) < 0 ) {
		/*
		 * nothing happend after "timeout" seconds
		 * send a forced debug interupt to stop the program
		 */

		if( mpc8xx_interrupt( timeout ) < 0 ) {
			return -1;
		}

		return -1;
	}

	return 0;
}

int mpc8xx_target_unload( mpc8xx_target_program_t* program )
{
	unsigned int i;

	if( program->program_valid != 1 ){
		return -1;
	}

	program->program_valid = 0;

	if( program->mem_backup == 0 )
		return 0;

	/*
	 * restore original memory content, if any program loaded before
	 */
	for (i = 0; i < program->program_len; i++ )
	{
		mpc8xx_set_word( program->start_address + ( i * 4 ), program->mem_backup[ i ] );


		/*
		 * verify if it works, cause the target program could have
		 * messed with the CS registers or caused the memory map
		 * to change, nothing we can do in that case appart from
		 * returning an error
		 */

		if( mpc8xx_get_word( program->start_address + ( i * 4 ) ) != program->mem_backup[ i ] ) {
			/*
			 * Error
			 */
			return -1;
		}

	}
	
	return 0;
}



int mpc8xx_target_restore( mpc8xx_target_registers_t* regs  )
{
	unsigned int temp;
	int i;

	for( i = 0; i < 32; i++)
	{
		temp = mpc8xx_get_gpr( i ); /* save registers */

		mpc8xx_set_gpr( i, regs->gpr[ i ] ); /* restore GPR0..31 */

		regs->gpr[i] = temp; /* store into register buffers for return values */
	}

	/*
	 * restore original control registers content, if any program loaded before
	 */

	temp = mpc8xx_get_spr( MPC8XX_SPR_CTR ); /*save CTR*/
	mpc8xx_set_spr( MPC8XX_SPR_CTR, regs->ctr ); /*restore CTR*/
	regs->ctr = temp;

	temp = mpc8xx_get_spr( MPC8XX_SPR_SRR0 ); /*save srr0*/
	mpc8xx_set_spr(MPC8XX_SPR_SRR0, regs->srr0 ); /*restore srr0*/
	regs->srr0 = temp;

	temp = mpc8xx_get_spr( MPC8XX_SPR_SRR1 ); /*save srr1*/
	mpc8xx_set_spr( MPC8XX_SPR_SRR1, regs->srr1 ); /*restore srr1*/
	regs->srr1 = temp;

	temp = mpc8xx_get_spr( MPC8XX_SPR_DER ); /*save DER*/
	mpc8xx_set_spr( MPC8XX_SPR_DER, regs->der ); /*restore DER*/
	regs->der = temp;

	return 0;
}

