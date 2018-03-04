/**
 * @file	mpc8xxmem.h
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

#ifndef __MPC8XXMEM_H__
#define __MPC8XXMEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MPC8XX_MSR_IR	0x00000020		/*IMMU enabled*/
#define MPC8XX_MSR_DR	0x00000010		/*DMMU enabled*/

#define MPC8XX_IMMU_ENTRIES	32		/* number of instruction mmu tlb entries */
#define MPC8XX_MI_CTR_MASK	0xfe000000	/* bits other than index */
#define MPC8XX_MI_CTR_SHIFT	8		/* start bit for index from right (PC# not PPC;-)*/
#define MPC8XX_DMMU_ENTRIES	32		/* number of instruction mmu tlb entries */
#define MPC8XX_MD_CTR_SHIFT	8		/* start bit for index from right (PC# not PPC;-)*/
#define MPC8XX_MD_CTR_MASK	0xfe000000	/* bits other than index */

#define MPC8XX_ICACHE_SETS	128		/* number of sets in one way of the instruction cache */

/* FIXME:  changes needed for 860P/DP , as icache has 4 ways */
/* and 6 bit LRU encoding */
#define MPC8XX_ICACHE_WAYS	2		/* number of associative ways of the instruction cache */
#define MPC8XX_ICACHE_SIZE	16		/* number of bytes in instruction cache */
#define MPC8XX_IC_CST_IEN	0x80000000	/* instruction cache enable in IC_CST */
#define MPC8XX_IC_DAT_TAG	0xfffff800	/* instruction cache tag field in IC_DAT */
#define MPC8XX_IC_DAT_VALID	0x00000200	/* instruction cache valid in IC_DAT */
#define MPC8XX_IC_DAT_LOCKED	0x00000100	/* instruction cache locked in IC_DAT */
#define MPC8XX_IC_DAT_LRU	0x00000080	/* instruction cache LRU in IC_DAT */
#define MPC8XX_IC_DAT_LRU_SHIFT 7		/* data cache LRU shift */
#define MPC8XX_DCACHE_SETS	128		/* number of sets in one way of the data cache */
#define MPC8XX_DCACHE_SETSHIFT	4		/* set adr start bit */
#define MPC8XX_DCACHE_SETMASK	0x000007f0	/* set adr mask = (SETS-1)<<SHIFT */
#define MPC8XX_DCACHE_WAYS	2		/* number of associative ways of the data cache */
#define MPC8XX_DCACHE_SIZE	16		/* number of bytes in data cache */
#define MPC8XX_DC_CST_DEN	0x80000000	/* data cache enable in DC_CST */
#define MPC8XX_DC_CST_DFWT	0x40000000	/* data cache force write through in DC_CST */
#define MPC8XX_DC_CST_LES	0x20000000	/* data cache little endian swap in DC_CST */
#define MPC8XX_DC_CST_CCER1	0x00200000	/* data cache error (dcbf dcbst)in DC_CST */
#define MPC8XX_DC_CST_CCER2	0x00100000	/* data cache error (DC_CST cmd)in DC_CST */
#define MPC8XX_DC_CST_CCER3	0x00080000	/* data cache error (reserved)in DC_CST */
#define MPC8XX_DC_CST_FLUSH	0x0e000000	/* data cache CMD = Flush block with phys. adr in DC_ADR */
#define MPC8XX_DC_CST_UNLOCK	0x08000000	/* data cache CMD = unlock block with phys. adr in DC_ADR */
#define MPC8XX_DC_CST_LOCK	0x06000000	/* data cache CMD = load and lock block with phys. adr in DC_ADR */
#define MPC8XX_DC_DAT_TAG	0xfffff800	/* data cache tag field in DC_DAT */
#define MPC8XX_DC_DAT_VALID	0x00000200	/* data cache valid in DC_DAT */
#define MPC8XX_DC_DAT_LOCKED	0x00000100	/* data cache locked in DC_DAT */
#define MPC8XX_DC_DAT_LRU	0x00000080	/* data cache LRU in DC_DAT */
#define MPC8XX_DC_DAT_LRU_SHIFT	7		/* data cache LRU shift */
#define MPC8XX_DC_DAT_MOD	0x00000040	/* data cache modified in DC_DAT */

/**
 *
 */
extern int mpc8xx_mmu_tablewalk( unsigned int vAddr, unsigned int* pAddr,
			  unsigned int pgdir, unsigned int base );

/**
 *
 */
extern int mpc8xx_icache_info( void );

/**
 *
 */
extern int mpc8xx_dcache_flush( unsigned int Adr, int bAll, int bmmu , unsigned int pgdir, unsigned int base );

/**
 *
 */
extern int mpc8xx_dcache_info( unsigned int Adr, int bAll , int bmmu , unsigned int pgtable, unsigned int base );

/**
 *
 */
extern int mpc8xx_mem_load( const char* sFilename, unsigned int start, unsigned int len);

/**
 *
 */
extern int mpc8xx_mem_save( const char* sFilename, unsigned int start, unsigned int len );


#ifdef __cplusplus
}
#endif

#endif /* !__MPC8XXMEM_H__ */
