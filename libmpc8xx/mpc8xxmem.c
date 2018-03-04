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

#include <stdio.h>

#include "mpc8xxmem.h"
#include "mpc8xxspr.h"
#include "mpc8xxbdm.h"
#include "mpc8xxmisc.h"

int mpc8xx_mmu_tablewalk( unsigned int vAddr, unsigned int* pAddr,
			  unsigned int pgdir, unsigned int base )
{
	unsigned int EPN;
	unsigned int TWC;
	unsigned int Level1;
	unsigned int Level2;
	unsigned int Level2Desc;

	/* save EPN register */
	EPN = mpc8xx_get_spr( MPC8XX_SPR_MD_EPN );

	/* save TWC register */
	TWC = mpc8xx_get_spr( MPC8XX_SPR_MD_TWC );

	/* set effective addr to virtual addr */
	mpc8xx_set_spr( MPC8XX_SPR_MD_EPN, vAddr );

	/* do table walk level 1*/
	Level1 = mpc8xx_get_spr( MPC8XX_SPR_M_TWB );
	if( Level1 == 0 )
	{
		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_MMU ) )
		{
			mpc8xx_printf("ppc_bdm_tablewalk: invalid level one descriptor for vAddr 0x%08x\n",vAddr);
		}

		return -1;
	}

	/* kernel function? */
	if( ( Level1 & 0x0800 ) && (pgdir != 0) )
	{
		Level1 = ( ( pgdir & 0x3ffff000 ) | ( Level1 & 0x00000fff ) );

		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_MMU ) )
		{
			mpc8xx_printf("ppc_bdm_tablewalk: pgdir -> Level1 = 0x%08x for vAddr 0x%08x\n",Level1,vAddr);
		}
	}

	Level2 = mpc8xx_get_word( Level1 );
	Level2 &= 0xfffff000;
	if( Level2 == 0 )
	{
		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_MMU ) )
		{
			mpc8xx_printf("ppc_bdm_tablewalk: invalid level two base for vAddr 0x%08x and level one = 0x%08x\n",vAddr,Level1);
		}

		return -1;
	}

	Level2 -= base; /* tophys, like table miss does, default=-0xc0000000 */
	Level2 |= 0x000000001; /* set valid bit in physical L2 page */

	mpc8xx_set_spr( MPC8XX_SPR_MD_TWC, Level2 );
	Level2Desc= mpc8xx_get_spr( MPC8XX_SPR_MD_TWC ); /* perform table walk level 2*/
	if( Level2Desc == 0 )
	{
		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_MMU ) )
		{
			mpc8xx_printf("ppc_bdm_tablewalk: invalid level two descriptor for vAddr 0x%08x, level one 0x%08x, level two 0x%08x\n",vAddr,Level1,Level2);
		}

		return -1;
	}

	*pAddr = mpc8xx_get_word( Level2Desc ); /*get physical page addr*/
	*pAddr = ( *pAddr & 0xfffff000 ) | ( vAddr & 0x00000fff ); /*calc physical addr*/

	if( mpc8xx_verbose_level( MPC8XX_VERBOSE_MMU ) )
	{
		mpc8xx_printf("ppc_bdm_tablewalk: MMU translates virtual 0x%08x to physical 0x%08x\n", vAddr, *pAddr );
	}

	mpc8xx_set_spr( MPC8XX_SPR_MD_TWC, TWC ); /* restore TWC register */
	mpc8xx_set_spr( MPC8XX_SPR_MD_EPN, EPN ); /* restore EPN register */

	return 0;
}

int mpc8xx_icache_info( void )
{
	unsigned int CacheAdr; /* IC_ADR value for reading cache content */
	unsigned int CacheDat; /* IC_DAT value for reading cache content */
	unsigned int OldICacheAdr; /* IC_ADR value for restoring */
	unsigned int IC_CST; /* IC_CST value*/
	unsigned int adr; /* cached adr */
	unsigned int LRU; /* Last Recently Used Code: 1 bit for 860, 6 bit for 860P/DP instruction cache */

	int set;
	int way;

	IC_CST = mpc8xx_get_spr( MPC8XX_SPR_IC_CST );

	mpc8xx_printf( "icache:" );

	if( IC_CST & MPC8XX_IC_CST_IEN )
		mpc8xx_printf(" enabled");
	else
		mpc8xx_printf(" disabled");

	mpc8xx_printf("\n");

	OldICacheAdr = mpc8xx_get_spr( MPC8XX_SPR_IC_ADR );

	for( set=0; set < MPC8XX_ICACHE_SETS; set++ )
	{
		mpc8xx_printf( "set0x%02x: ", set );
		for (way=0; way < MPC8XX_ICACHE_WAYS; way++)
		{
			/* read tags */
			CacheAdr = ( way << (31-19) ) | ( set << (31-27) );
			mpc8xx_set_spr( MPC8XX_SPR_IC_ADR, CacheAdr );

			CacheDat = mpc8xx_get_spr( MPC8XX_SPR_IC_DAT );

			adr = ( CacheDat & MPC8XX_IC_DAT_TAG ) | ( set << (31-27) );

			/* first output ?*/
			if( way == 0 )
			{
				LRU = (CacheDat & MPC8XX_IC_DAT_LRU) >> MPC8XX_IC_DAT_LRU_SHIFT;
				mpc8xx_printf("LRU=0x%1x ",LRU);
			}
			else /* not first way, print separator */
			{
				mpc8xx_printf(", ");
			}

			mpc8xx_printf( "way%1x:0x%08x..%1x", way, adr, MPC8XX_ICACHE_SIZE - 1 );

			if( CacheDat & MPC8XX_IC_DAT_VALID )
				mpc8xx_printf(" val");
			else
				mpc8xx_printf("    ");

			if( CacheDat & MPC8XX_IC_DAT_LOCKED )
				mpc8xx_printf(" lck");
			else
				mpc8xx_printf("    ");
		}

		mpc8xx_printf("\n");
	}

	mpc8xx_set_spr( MPC8XX_SPR_IC_ADR, OldICacheAdr );

	return 0;
}

/*****************************************************************/

int mpc8xx_dcache_flush( unsigned int Adr, int bAll, int bmmu , unsigned int pgdir, unsigned int base )
{
	unsigned int CacheAdr;		/* DC_ADR value for reading cache content */
	unsigned int CacheDat;		/* DC_DAT value for reading cache content */
	unsigned int OldDCacheAdr;	/* DC_ADR value for restoring */
	unsigned int DC_CST;		/* DC_CST value*/
	unsigned int pAddr;		/* cached physical address */
	unsigned int ValMod;		/* helper variable for valid modified entry test */
	unsigned int msr;

	int set;
	int way;

	if( bAll )
	{
		/*
		 * if verbose on data cache ops, be informative,
		 * but allow flushing in all cases, even if unnecessary
		 */
		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_DCA ) )
		{
			DC_CST = mpc8xx_get_spr( MPC8XX_SPR_DC_CST );

			if( !( DC_CST & MPC8XX_DC_CST_DEN ) ) /* data cache enabled? */
			{
				mpc8xx_printf("mpc8xx_dcache_flush: data cache not enabled\n");
			}
			if( DC_CST & MPC8XX_DC_CST_DFWT ) /* data cache forced write through enable? */
			{
				mpc8xx_printf("mpcbdm_dcache_flush: data cache DFWT enabled\n");
			}
		}

		set = 0;
	}
	else /* specific address */
	{
		msr = mpc8xx_get_spr( MPC8XX_SPR_MSR );

		if( ( msr & MPC8XX_MSR_DR ) && ( bmmu ) ) /* data mmu on and option activated*/
		{
			mpc8xx_mmu_tablewalk( Adr, &Adr, pgdir, base );
		}

		set = ( ( Adr & MPC8XX_DCACHE_SETMASK ) >> MPC8XX_DCACHE_SETSHIFT );
	}

	OldDCacheAdr = mpc8xx_get_spr( MPC8XX_SPR_DC_ADR ); /* save DC_ADR */

	for( ; set < MPC8XX_DCACHE_SETS; set++ )
	{
		for( way=0; way < MPC8XX_DCACHE_WAYS; way++ )
		{
			/*
			 * address format for reading cache flags AND for flushing blocks
			 * flushing blocks need this format, not physical address as stated
			 * in 860UM.pdf
			 */
			CacheAdr = ( (way << (31-19) ) | ( set << (31-27) ) );
			mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, CacheAdr );

			CacheDat = mpc8xx_get_spr( MPC8XX_SPR_DC_DAT );
			pAddr = ( (CacheDat & MPC8XX_DC_DAT_TAG) | (set << (31-27)) );

			/*
			 * If flushing all command
			 * only flush modified valid (the necessary) cache lines  for speed up.
			 */
			ValMod = ( CacheDat & (MPC8XX_DC_DAT_VALID | MPC8XX_DC_DAT_MOD) );

			if( ( ( bAll) && (ValMod == (MPC8XX_DC_DAT_VALID | MPC8XX_DC_DAT_MOD ) ) ) ||
			    ( (!bAll) && (pAddr == (Adr & ~( MPC8XX_DCACHE_SIZE-1 ) ) ) ) )
			{
				/*
				 * replaced call to low level data cache flush block
				 * stuff, for speed up
				 */
				if( CacheDat & MPC8XX_DC_DAT_LOCKED )
				{
					if( mpc8xx_verbose_level( MPC8XX_VERBOSE_DCA ) )
					{
						mpc8xx_printf("mpcbdm_dcache_flush: unlocking set 0x%02x way 0x%1x for physical adr 0x%08x\n",set,way,pAddr);
					}

					mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, pAddr ); /* set physical address of cached line */
					mpc8xx_set_spr( MPC8XX_SPR_DC_CST, MPC8XX_DC_CST_UNLOCK ); /* unlock block */
				}

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_DCA ) )
				{
					mpc8xx_printf("mpcbdm_dcache_flush: flushing set 0x%02x way 0x%1x for physical adr 0x%08x\n",set,way,pAddr);
				}

				/*
				 * FIXME: this took me half an day: ADR has to be in DC_DAT
				 * reading format for CMD=FLUSH?  and not given as physical
				 * address as stated in the manuals!!!
				 */

				/* set address of cache block */
				mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, CacheAdr );

				/* flush block to get modified cache content into memory */
				mpc8xx_set_spr( MPC8XX_SPR_DC_CST, MPC8XX_DC_CST_FLUSH);

				if( CacheDat & MPC8XX_DC_DAT_LOCKED )
				{
					if( mpc8xx_verbose_level( MPC8XX_VERBOSE_DCA ) )
					{
						mpc8xx_printf("mpcbdm_dcache_flush: relocking set 0x%02x way 0x%1x for physical adr 0x%08x\n",set,way,pAddr);
					}

					/* set physical address of cached line */
					mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, pAddr);
					/* load and lock block again for restauration */
					mpc8xx_set_spr( MPC8XX_SPR_DC_CST, MPC8XX_DC_CST_LOCK);
				}

				if( !bAll ){
					break; /* only one way entry can be valid, so leave way loop*/
				}
			}
		}
		if (!bAll ){
			break; /* finish set loop, if only flushing one entry */
		}
	}

	mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, OldDCacheAdr ); /* restore DC_ADR */

	return 0;
}

/*****************************************************************/
int mpc8xx_dcache_info( unsigned int Adr, int bAll , int bmmu , unsigned int pgtable, unsigned int base )
{
	unsigned int CacheAdr; /* DC_ADR value for reading cache content */
	unsigned int CacheDat; /* DC_DAT value for reading cache content */
	unsigned int OldDCacheAdr; /* DC_ADR value for restoring */
	unsigned int DC_CST; /* DC_CST value*/
	unsigned int adr; /* cached adr */
	unsigned int LRU; /* Last Recently Used Code: 1 bit for 860 data cache*/
	unsigned int msr;

	int set;
	int way;

	msr = mpc8xx_get_spr( MPC8XX_SPR_MSR );

	OldDCacheAdr = mpc8xx_get_spr( MPC8XX_SPR_DC_ADR ); /* for restauration */

	if( bAll ) /* total dcache info */
	{
		DC_CST = mpc8xx_get_spr( MPC8XX_SPR_DC_CST );

		mpc8xx_printf("dcache: ");

		if( DC_CST & MPC8XX_DC_CST_DEN)
			mpc8xx_printf(" enabled");
		else
			mpc8xx_printf("disabled");


		if( DC_CST & MPC8XX_DC_CST_DFWT)
			mpc8xx_printf(", DFWT");

		if( DC_CST & MPC8XX_DC_CST_LES )
			mpc8xx_printf(", LES");

		if( DC_CST & MPC8XX_DC_CST_CCER1)
			mpc8xx_printf(", CCER1");

		if( DC_CST & MPC8XX_DC_CST_CCER2)
			mpc8xx_printf(", CCER2");

		if( DC_CST & MPC8XX_DC_CST_CCER3 )
			mpc8xx_printf(", CCER3");

		mpc8xx_printf(", data MMU: MSR[DR]");

		if( msr & MPC8XX_MSR_DR ) /* data mmu on*/
			mpc8xx_printf(" enabled");
		else
			mpc8xx_printf(" disabled");

		mpc8xx_printf("\n");

		CacheAdr = ( 1 << (31-18) ) | (4 << (31-27) ); /* read last copy back address */
		mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, CacheAdr );
		CacheDat = mpc8xx_get_spr( MPC8XX_SPR_DC_DAT);

		mpc8xx_printf("last copyback adr=0x%08x:",CacheDat);

		for( set=0; set < (MPC8XX_DCACHE_SIZE / 4) ; set++ )
		{
			CacheAdr = ( 1 << (31-18) ) | ( set << (31-27) ); /* read last copy back buffer */
			mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, CacheAdr );
			CacheDat = mpc8xx_get_spr( MPC8XX_SPR_DC_DAT );

			mpc8xx_printf(" 0x%08x",CacheDat);
		}

		mpc8xx_printf("\n");

		set = 0;
	}
	else /* specific address */
	{
		if( (msr & MPC8XX_MSR_DR ) && ( bmmu ) ) /*mmu on and option activated*/
		{
			mpc8xx_mmu_tablewalk( Adr , &Adr, pgtable, base );
		}

		Adr = ( Adr & (~(MPC8XX_DCACHE_SIZE-1))); /* align to cache block start */
		set = (Adr & MPC8XX_DCACHE_SETMASK) >> MPC8XX_DCACHE_SETSHIFT;
	}


	for( ; set < MPC8XX_DCACHE_SETS; set++ )
	{
		for( way=0; way < MPC8XX_DCACHE_WAYS; way++ )
		{
			/* read tags */
			CacheAdr = (way<<(31-19)) | (set << (31-27));

			mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, CacheAdr );
			CacheDat = mpc8xx_get_spr( MPC8XX_SPR_DC_DAT );
			adr = (CacheDat & MPC8XX_DC_DAT_TAG) | (set << (31-27));

			if( !bAll && (adr != (Adr & ~(MPC8XX_DCACHE_SIZE-1))) )
				continue; /* skip not matching entry */

			if( !bAll || (way == 0)) /* show LRU code only once */
			{
				LRU = ( CacheDat & MPC8XX_DC_DAT_LRU) >> MPC8XX_DC_DAT_LRU_SHIFT;
				mpc8xx_printf("set0x%02x: LRU=0x%1x ",set,LRU);
			}

			if( bAll )
			{
				if( way ) /* need separator ?*/
					mpc8xx_printf(", ");

				mpc8xx_printf("way%1x:0x%08x..%1x",way,adr,MPC8XX_DCACHE_SIZE-1);
			}
			else /* show only specified way */
			{
				mpc8xx_printf("way%1x:0x%08x..%1x",way,Adr,MPC8XX_DCACHE_SIZE-1);
			}
			if( CacheDat & MPC8XX_DC_DAT_VALID )
				mpc8xx_printf(" val");
			else
				mpc8xx_printf("    ");

			if (CacheDat & MPC8XX_DC_DAT_LOCKED)
				mpc8xx_printf(" lck");
			else
				mpc8xx_printf("    ");


			if( CacheDat & MPC8XX_DC_DAT_MOD )
				mpc8xx_printf(" mod");
			else
				mpc8xx_printf("    ");


			if( !bAll )
			{
				mpc8xx_printf("\n");
				break;
			}
		}

		if( !bAll )
			break;

		mpc8xx_printf("\n");
	}

	mpc8xx_set_spr( MPC8XX_SPR_DC_ADR, OldDCacheAdr );

	return 0;
}

int mpc8xx_mem_load( const char* sFilename, unsigned int start, unsigned int len)
{
	FILE * f;
	unsigned char buffer[ 1024 ];

	unsigned int	nTotal;
	unsigned int	nBuffer;
	unsigned int	nLeft;
	int flen;

	mpc8xx_printf("mem load \"%s\" to 0x%08x len %d\n", sFilename, start, len);

	if( !sFilename )
	{
		mpc8xx_printf("parameter syntax error.");
		return -1;
	}

	f = fopen(sFilename, "rb");

	if( f == NULL )
	{
		mpc8xx_printf("failed to open input file.");
		return -1;
	}

	if( fseek(f, 0, SEEK_END)  < 0 )
	{
		mpc8xx_printf("fseek error.");
		fclose(f);
		return -1;
	}

	flen = ftell(f);

	if( len == 0 ) {
		len = flen;
	} else if( flen  < len ) {
		len = flen;
	}

	mpc8xx_printf(" len = 0x%08x, total file length\n", len);
	rewind(f);


	for( nTotal = 0, nLeft = len; nLeft > 0; nTotal += nBuffer, nLeft -= nBuffer )
	{
		if( nLeft > 1024 )
			nBuffer = 1024;
		else
			nBuffer = nLeft;

		if( fread( buffer, 1, nBuffer, f ) != nBuffer )
		{
			mpc8xx_printf("read error.");
			fclose( f );
			return -1;
		}

		if( mpc8xx_write_block( start + nTotal, buffer, nBuffer ) < 0 ){
			fclose( f );
			return -1;
		}
	}

	fclose(f);

	return len;
}

/*****************************************************************/

int mpc8xx_mem_save( const char* sFilename, unsigned int start, unsigned int len )
{
	FILE * f;
	unsigned char buffer[ 1024 ];

	unsigned int	nTotal;
	unsigned int	nBuffer;
	unsigned int	nLeft;

	mpc8xx_printf("mem save \"%s\" from 0x%08x len %d\n", sFilename, start, len);

	if( (sFilename == NULL ) || (len == 0 ) )
	{
		mpc8xx_printf("parameter syntax error.");
		return -1;
	}

	f = fopen(sFilename, "wb");

	if(f == NULL )
	{
		mpc8xx_printf("failed to open output file.");
		return -1;
	}

	for (nTotal = 0, nLeft = len; nLeft > 0; nTotal += nBuffer, nLeft -= nBuffer)
	{
		if(nLeft > 1024)
			nBuffer = 1024;
		else
			nBuffer = nLeft;


		if( mpc8xx_read_block(start + nTotal, buffer, nBuffer) < 0 ){
			fclose( f );
			return -1;
		}

		if( fwrite(buffer, 1, nBuffer, f) != nBuffer )
		{
			mpc8xx_printf("write error.");
			return -1;
		}
	}

	fclose(f);

	return len;
}


