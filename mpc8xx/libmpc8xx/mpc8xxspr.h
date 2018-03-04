/**
 * @file	mpc8xxspr.h
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

#ifndef __MPC8XXSPR_H__
#define __MPC8XXSPR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MPC8XX_GPR_REG_MASK	0x2000

#define MPC8XX_SPR_XER		1
#define MPC8XX_SPR_LR		8
#define MPC8XX_SPR_CTR		9
#define MPC8XX_SPR_TBLR		268
#define MPC8XX_SPR_TBUR		269
#define MPC8XX_SPR_DSISR	18
#define MPC8XX_SPR_DAR		19
#define MPC8XX_SPR_DEC		22
#define MPC8XX_SPR_SRR0		26
#define MPC8XX_SPR_SRR1		27
#define MPC8XX_SPR_SPRG0	272
#define MPC8XX_SPR_SPRG1	273
#define MPC8XX_SPR_SPRG2	274
#define MPC8XX_SPR_SPRG3	275
#define MPC8XX_SPR_TBLW		284
#define MPC8XX_SPR_TBUW		285
#define MPC8XX_SPR_PVR		287
#define MPC8XX_SPR_EIE		80
#define MPC8XX_SPR_EID		81
#define MPC8XX_SPR_NRI		82
#define MPC8XX_SPR_DPIR		631
#define MPC8XX_SPR_IMMR		638
#define MPC8XX_SPR_IC_CST	560
#define MPC8XX_SPR_IC_ADR	561
#define MPC8XX_SPR_IC_DAT	562
#define MPC8XX_SPR_DC_CST	568
#define MPC8XX_SPR_DC_ADR	569
#define MPC8XX_SPR_DC_DAT	570
#define MPC8XX_SPR_MI_CTR	784
#define MPC8XX_SPR_MI_AP	786
#define MPC8XX_SPR_MI_EPN	787
#define MPC8XX_SPR_MI_TWC	789
#define MPC8XX_SPR_MI_RPN	790
#define MPC8XX_SPR_MI_CAM	816
#define MPC8XX_SPR_MI_RAM0	817
#define MPC8XX_SPR_MI_RAM1	818
#define MPC8XX_SPR_MD_CTR	792
#define MPC8XX_SPR_M_CASID	793
#define MPC8XX_SPR_MD_AP	794
#define MPC8XX_SPR_MD_EPN	795
#define MPC8XX_SPR_M_TWB	796
#define MPC8XX_SPR_MD_TWC	797
#define MPC8XX_SPR_MD_RPN	798
#define MPC8XX_SPR_M_TW		799
#define MPC8XX_SPR_MD_CAM	824
#define MPC8XX_SPR_MD_RAM0	825
#define MPC8XX_SPR_MD_RAM1	826
#define MPC8XX_SPR_CMPA		144
#define MPC8XX_SPR_CMPB		145
#define MPC8XX_SPR_CMPC		146
#define MPC8XX_SPR_CMPD		147
#define MPC8XX_SPR_ICR		148
#define MPC8XX_SPR_DER		149
#define MPC8XX_SPR_COUNTA	150
#define MPC8XX_SPR_COUNTB	151
#define MPC8XX_SPR_CMPE		152
#define MPC8XX_SPR_CMPF		153
#define MPC8XX_SPR_CMPG		154
#define MPC8XX_SPR_CMPH		155
#define MPC8XX_SPR_LCTRL1	156
#define MPC8XX_SPR_LCTRL2	157
#define MPC8XX_SPR_ICTRL	158
#define MPC8XX_SPR_BAR		159
#define MPC8XX_SPR_DPDR		630

#define MPC8XX_SPRI_REV_NUM 	0x3cb0

#define MPC8XX_SPRI_MASK	0x10000
#define MPC8XX_SPRI_SIUMCR	(MPC8XX_SPRI_MASK | 0x000)
#define MPC8XX_SPRI_SYPCR	(MPC8XX_SPRI_MASK | 0x004)
#define MPC8XX_SPRI_SWSR	(MPC8XX_SPRI_MASK | 0x00E)  /* 16 bit 556C AA39 */
#define MPC8XX_SPRI_SIPEND	(MPC8XX_SPRI_MASK | 0x010)
#define MPC8XX_SPRI_SIMASK	(MPC8XX_SPRI_MASK | 0x014)
#define MPC8XX_SPRI_SIEL	(MPC8XX_SPRI_MASK | 0x018)
#define MPC8XX_SPRI_SIVEC	(MPC8XX_SPRI_MASK | 0x01c)
#define MPC8XX_SPRI_TESR	(MPC8XX_SPRI_MASK | 0x020)
#define MPC8XX_SPRI_SDCR	(MPC8XX_SPRI_MASK | 0x032)
#define MPC8XX_SPRI_PBR(x)	(MPC8XX_SPRI_MASK | ( 0x080 + (x)*8))
#define MPC8XX_SPRI_POR(x)	(MPC8XX_SPRI_MASK | ( 0x084 + (x)*8))
#define MPC8XX_SPRI_PGCRA	(MPC8XX_SPRI_MASK | 0x0E0)
#define MPC8XX_SPRI_PGCRB	(MPC8XX_SPRI_MASK | 0x0E4)
#define MPC8XX_SPRI_PSCR	(MPC8XX_SPRI_MASK | 0x0E8)
#define MPC8XX_SPRI_PIPR	(MPC8XX_SPRI_MASK | 0x0F0)
#define MPC8XX_SPRI_PER		(MPC8XX_SPRI_MASK | 0x0F8)
#define MPC8XX_SPRI_BR(x)	(MPC8XX_SPRI_MASK | (0x100 + (x)*8))
#define MPC8XX_SPRI_OR(x)	(MPC8XX_SPRI_MASK | (0x104 + (x)*8))
#define MPC8XX_SPRI_MAR		(MPC8XX_SPRI_MASK | 0x164)
#define MPC8XX_SPRI_MCR		(MPC8XX_SPRI_MASK | 0x168)
#define MPC8XX_SPRI_MAMR	(MPC8XX_SPRI_MASK | 0x170)
#define MPC8XX_SPRI_MBMR	(MPC8XX_SPRI_MASK | 0x174)
#define MPC8XX_SPRI_MSTAT	(MPC8XX_SPRI_MASK | 0x178)    /* MSTAT MPTPR */
#define MPC8XX_SPRI_MDR		(MPC8XX_SPRI_MASK | 0x17C)
#define MPC8XX_SPRI_TBSCR	(MPC8XX_SPRI_MASK | 0x200)
#define MPC8XX_SPRI_TBREFA	(MPC8XX_SPRI_MASK | 0x204)
#define MPC8XX_SPRI_TBREFB	(MPC8XX_SPRI_MASK | 0x208)
#define MPC8XX_SPRI_RTCSC	(MPC8XX_SPRI_MASK | 0x220)
#define MPC8XX_SPRI_RTC		(MPC8XX_SPRI_MASK | 0x224)
#define MPC8XX_SPRI_RTSEC	(MPC8XX_SPRI_MASK | 0x228)
#define MPC8XX_SPRI_RTCAL	(MPC8XX_SPRI_MASK | 0x22C)
#define MPC8XX_SPRI_PISCR	(MPC8XX_SPRI_MASK | 0x240)
#define MPC8XX_SPRI_PITC	(MPC8XX_SPRI_MASK | 0x244)
#define MPC8XX_SPRI_PITR	(MPC8XX_SPRI_MASK | 0x248)
#define MPC8XX_SPRI_SCCR	(MPC8XX_SPRI_MASK | 0x280)
#define MPC8XX_SPRI_PLPRCR	(MPC8XX_SPRI_MASK | 0x284)
#define MPC8XX_SPRI_RSR		(MPC8XX_SPRI_MASK | 0x288)
#define MPC8XX_SPRI_TBSCRK	(MPC8XX_SPRI_MASK | 0x300)
#define MPC8XX_SPRI_CIVR	(MPC8XX_SPRI_MASK | 0x930)
#define MPC8XX_SPRI_CICR	(MPC8XX_SPRI_MASK | 0x940)
#define MPC8XX_SPRI_CIPR	(MPC8XX_SPRI_MASK | 0x944)
#define MPC8XX_SPRI_CIMR	(MPC8XX_SPRI_MASK | 0x948)
#define MPC8XX_SPRI_CISR	(MPC8XX_SPRI_MASK | 0x94C)
#define MPC8XX_SPRI_PADIR	(MPC8XX_SPRI_MASK | 0x950)
#define MPC8XX_SPRI_PAODR	(MPC8XX_SPRI_MASK | 0x954)
#define MPC8XX_SPRI_PCDIR	(MPC8XX_SPRI_MASK | 0x960)
#define MPC8XX_SPRI_PCSO	(MPC8XX_SPRI_MASK | 0x964)
#define MPC8XX_SPRI_PCINT	(MPC8XX_SPRI_MASK | 0x968)
#define MPC8XX_SPRI_PDDIR	(MPC8XX_SPRI_MASK | 0x970)
#define MPC8XX_SPRI_PDDAT	(MPC8XX_SPRI_MASK | 0x976)
#define MPC8XX_SPRI_PBDIR	(MPC8XX_SPRI_MASK | 0xAB8)
#define MPC8XX_SPRI_PBPAR	(MPC8XX_SPRI_MASK | 0xABC)
#define MPC8XX_SPRI_PBODR	(MPC8XX_SPRI_MASK | 0xAC0)
#define MPC8XX_SPRI_PBDAT	(MPC8XX_SPRI_MASK | 0xAC4)
#define MPC8XX_SPRI_SIMODE	(MPC8XX_SPRI_MASK | 0xAE0)
#define MPC8XX_SPRI_SIGMR	(MPC8XX_SPRI_MASK | 0xAE4)
#define MPC8XX_SPRI_SICR	(MPC8XX_SPRI_MASK | 0xAEC)
#define MPC8XX_SPRI_SIRP	(MPC8XX_SPRI_MASK | 0xAF0)

#define MPC8XX_SPR_MASK		0x20000

#define MPC8XX_SPR_MSR		(MPC8XX_SPR_MASK | 0x1)
#define MPC8XX_SPR_CR		(MPC8XX_SPR_MASK | 0x2)

#define MPC8XX_ICR_RST		PPC_BIT( 1 )
#define MPC8XX_ICR_CHSTP	PPC_BIT( 2 )
#define MPC8XX_ICR_MCI		PPC_BIT( 3 )
#define MPC8XX_ICR_EXTI		PPC_BIT( 6 )
#define MPC8XX_ICR_ALI		PPC_BIT( 7 )
#define MPC8XX_ICR_PRI		PPC_BIT( 8 )
#define MPC8XX_ICR_FPUVI	PPC_BIT( 9 )
#define MPC8XX_ICR_DECI		PPC_BIT( 10 )
#define MPC8XX_ICR_SYSI		PPC_BIT( 13 )
#define MPC8XX_ICR_TR		PPC_BIT( 14 )
#define MPC8XX_ICR_SEI		PPC_BIT( 17 )
#define MPC8XX_ICR_ITLBM	PPC_BIT( 18 )
#define MPC8XX_ICR_DTLBM	PPC_BIT( 19 )
#define MPC8XX_ICR_ITLBER	PPC_BIT( 20 )
#define MPC8XX_ICR_DTLBER	PPC_BIT( 21 )
#define MPC8XX_ICR_LBRK		PPC_BIT( 28 )
#define MPC8XX_ICR_IBRK		PPC_BIT( 29 )
#define MPC8XX_ICR_EBRK		PPC_BIT( 30 )
#define MPC8XX_ICR_DPI		PPC_BIT( 31 )

#define MPC8XX_DER_RST		PPC_BIT( 1 )
#define MPC8XX_DER_CHSTP	PPC_BIT( 2 )
#define MPC8XX_DER_MCI		PPC_BIT( 3 )
#define MPC8XX_DER_EXTI		PPC_BIT( 6 )
#define MPC8XX_DER_ALI		PPC_BIT( 7 )
#define MPC8XX_DER_PRI		PPC_BIT( 8 )
#define MPC8XX_DER_FPUVI	PPC_BIT( 9 )
#define MPC8XX_DER_DECI		PPC_BIT( 10 )
#define MPC8XX_DER_SYSI		PPC_BIT( 13 )
#define MPC8XX_DER_TR		PPC_BIT( 14 )
#define MPC8XX_DER_SEI		PPC_BIT( 17 )
#define MPC8XX_DER_ITLBM	PPC_BIT( 18 )
#define MPC8XX_DER_DTLBM	PPC_BIT( 19 )
#define MPC8XX_DER_ITLBER	PPC_BIT( 20 )
#define MPC8XX_DER_DTLBER	PPC_BIT( 21 )
#define MPC8XX_DER_LBRK		PPC_BIT( 28 )
#define MPC8XX_DER_IBRK		PPC_BIT( 29 )
#define MPC8XX_DER_EBRK		PPC_BIT( 30 )
#define MPC8XX_DER_DPI		PPC_BIT( 31 )

struct mpc8xx_spr_name_s
{
	const char* name;	/* motorola name for spr */
	int num;		/* spr number */
	const char* namefield;	/* (Na|Nb..|Ne) Acronyms of bitfield plus start and end */
	const char* longname;	/* no acronym */
	const char* reference;	/* where to find documentation */
};

typedef struct mpc8xx_spr_name_s mpc8xx_spr_name_t;

extern mpc8xx_spr_name_t mpc8xx_spr_names[];


typedef struct mpc8xx_cpu_info_s
{
	unsigned int pvr;
	unsigned short partmask;
	unsigned short revnum;
	const char * name;
	const char * regfile;
	unsigned int flags;
} mpc8xx_cpu_info_t;

extern mpc8xx_cpu_info_t mpc8xx_cpu_info[];

/** Get the SPR ID nummer by name
 * @param pch the name of the SPR
 * @return the ID number of the SPR or -1
 */
extern int mpc8xx_spr_find_num( const char *pch );


/** Print SPR Info into a buffer
 * @param num the SPR ID number
 * @param val the value of the SPR
 * @param bName
 * @param bDef
 * @param bVal
 * @param bPretty
 * @param buffer The buffer to hold the resulting text
 * @param buffer_size the sife of buffer
 * @return NULL on error or else a pointer to buffer
 */
extern char* mpc8xx_spr_info(	int num, unsigned int val,
				int bName, int bDef, int bVal, int bPretty,
				char* buffer, int buffer_size );


extern void mpc8xx_spr_print_info(int num, unsigned int val,
			   int bName, int bDef, int bVal, int bPretty );


extern int mpc8xx_print_cpu_info();


#ifdef __cplusplus
}
#endif

#endif /* !__MPC8XXSPR_H__ */
