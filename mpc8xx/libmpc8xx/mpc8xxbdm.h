/**
 * @file	mpc8xxbdm.h
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

#ifndef __MPC8XXBDM_H__
#define __MPC8XXBDM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Commands of Development Port Shift Register */

#define MPC8XX_BDM_PREFIX_CORE_INSTRUCTION	0x0	/* 00b +32 bits */
#define MPC8XX_BDM_PREFIX_CORE_DATA		0x1	/* 01b +32 bits */
#define MPC8XX_BDM_PREFIX_TECR			0x2	/* 10b +7 bits */
#define MPC8XX_BDM_PREFIX_DPC			0x3	/* 11b +7 bits */

#define MPC8XX_BDM_DPC_NOP 			0x00
#define MPC8XX_BDM_DPC_HRESET			0x01
#define MPC8XX_BDM_DPC_SRESET			0x02
#define MPC8XX_BDM_DPC_END_DOWNLOAD		0x43
#define MPC8XX_BDM_DPC_START_DOWNLOAD		0x63
#define MPC8XX_BDM_DPC_NEGATE_MASK_BREAK	0x1f
#define MPC8XX_BDM_DPC_ASSERT_MASK_BREAK	0x3f
#define MPC8XX_BDM_DPC_NEGATE_NMASK_BREAK	0x1f
#define MPC8XX_BDM_DPC_ASSERT_NMASK_BREAK	0x7f

#define MPC8XX_BDM_STAT_CORE_DATA		0x0	/* 00b +32 bits */
#define MPC8XX_BDM_STAT_SEQ_ERROR		0x1	/* 01b +2 bits + xx */
#define MPC8XX_BDM_STAT_CORE_INTERRUPT		0x2	/* 10b +2 bits + xx */
#define MPC8XX_BDM_STAT_NULL			0x3	/* 11b +2 bits + xx */

#define MPC8XX_COM_NOP		0x60000000
#define MPC8XX_COM_RFI		0x4C000064
#define MPC8XX_COM_ICR		0x7C1422A6
#define MPC8XX_COM_DER		0x7C1522A6
#define MPC8XX_COM_MFMSR	0x7C0000A6
#define MPC8XX_COM_MTSPR	0x7C169BA6


#define PPC_I_R0R1   0x90010000	/* stw 0,0(1) # mov r0,@r1 */
#define PPC_I_R0R1hw 0xB0010000	/* stw 0,0(1) # movh r0,@r1 */
#define PPC_I_R1R0   0x80010000	/* lwz 0,0(1) # mov @r1,r0 */

#define MAX_HWBRK	4 /*number of hardware breakpoint registers to use*/

#define BDM_BREAKPOINT {0x0,0x0,0x0,0x0} /* For ppc 8xx */


/**
 *
 */
extern unsigned int mpc8xx_get_gpr( int reg_nr );

/**
 *
 */
extern int mpc8xx_set_gpr( int reg_nr, unsigned int value );

/**
 *
 */
extern unsigned int mpc8xx_get_msr( void );

/**
 *
 */
extern int mpc8xx_set_msr( unsigned int value );

/**
 *
 */
extern unsigned int mpc8xx_get_cr( void );

/**
 *
 */
extern int mpc8xx_set_cr( unsigned int value );

/**
 *
 */
extern unsigned int mpc8xx_get_spri( int reg_nr );

/**
 *
 */
extern int mpc8xx_set_spri( int reg_nr, unsigned int value );

/**
 *
 */
extern int mpc8xx_set_spri_hw( int reg_nr, unsigned int value );

/**
 *
 */
extern unsigned int mpc8xx_get_spr( int reg );

/**
 *
 */
extern int mpc8xx_set_spr( int reg, unsigned int value );

/**
 *
 */
extern int mpc8xx_set_word( unsigned int addr, unsigned int val );

/**
 *
 */
extern unsigned int mpc8xx_get_word( unsigned int addr );

/**
 *
 */
extern int mpc8xx_set_halfword( unsigned int addr, unsigned int val );

/**
 *
 */
extern unsigned int mpc8xx_get_halfword( unsigned int addr );

/**
 *
 */
extern int mpc8xx_set_byte( unsigned int addr, unsigned int val );

/**
 *
 */
extern unsigned int mpc8xx_get_byte( unsigned int addr );

/**
 *
 */
extern int mpc8xx_read_block( unsigned int from_address, unsigned char* to_buffer, unsigned int len );

/**
 *
 */
extern int mpc8xx_write_block( unsigned int to_address, unsigned char *from_buffer, unsigned int len );

/**
 *
 */
extern int mpc8xx_hreset( void );

/**
 *
 */
extern int mpc8xx_sreset( void );

/**
 *
 */
extern int mpc8xx_interrupt( int timeout );

/**
 *
 */
extern int mpc8xx_resume( void );

/**
 *
 */
extern int mpc8xx_continue_single_step( void );

/**
 *
 */
extern int mpc8xx_continue_until_branch( void );

/**
 *
 */
extern int mpc8xx_continue( void );


#ifdef __cplusplus
}
#endif

#endif /* __MPC8XXBDM_H__ */

