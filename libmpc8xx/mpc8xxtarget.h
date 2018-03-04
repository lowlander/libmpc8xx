/**
 * @file	mpc8xxtarget.h
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

#ifndef __MPC8XXTARGET_H__
#define __MPC8XXTARGET_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Target registers
 *
 */
struct mpc8xx_target_registers_s {
	unsigned int gpr[ 32 ];	/**< The 32 General Purpose Registers */
	unsigned int ctr;	/**< The CTR Register */
	unsigned int srr0;	/**< The SRR0 Register */
	unsigned int srr1;	/**< The SRR1 Register */
	unsigned int der;	/**< The DER Register */
};

typedef struct mpc8xx_target_registers_s mpc8xx_target_registers_t;

/** Target program information
 *
 */
struct mpc8xx_target_program_s {
	unsigned int start_address;	/**< Start address on the target of the program */
	unsigned int* program_code;	/**< Local array with the program code */
	unsigned int program_len;	/**< Local array length */
	unsigned int program_valid;	/**< Flag that identicates that the program is loaded */
	unsigned int* mem_backup;	/**< Local array used to backup target memory */
};

typedef struct mpc8xx_target_program_s mpc8xx_target_program_t;

/** Prepare the target
 * @param regs hold a backup of the target registers
 * @return
 */
extern int mpc8xx_target_prepare( mpc8xx_target_registers_t* regs );

/** Load a program in the target
 * @param prog holds the program information
 * @return
 */
extern int mpc8xx_target_load( mpc8xx_target_program_t* prog );

/** Start the target program
 * @prog the target program
 * @timeout time (in seconds) to wait until canceling the target program
 * @return
 */
extern int mpc8xx_target_execute( mpc8xx_target_program_t* prog, int timeout );

/** Unload the target program
 * @param prog the target program
 * @return
 */
extern int mpc8xx_target_unload( mpc8xx_target_program_t* prog );

/** Restore target registers
 * @param regs backup of the registers, on return they will hold the
 * values as changed by the target program
 * @return
 */
extern int mpc8xx_target_restore( mpc8xx_target_registers_t* regs );

#ifdef __cplusplus
}
#endif

#endif /*! __MPC8XXTARGET_H__ */

