/**
 * @file	mpc8xxflash.c
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
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>

#include "mpc8xxmisc.h"
#include "mpc8xxflash.h"
#include "mpc8xxbdm.h"
#include "mpc8xxtarget.h"
#include "mpc8xxspr.h"
#include "lptbdm.h"
#include "mpc8xxmem.h"

static mpc8xx_flash_bank_t mpc8xx_flash_bank[ MPC8XX_MAX_FLASH_BANKS ];

/*****************************************************************
* FLASH routines                                                 *
*****************************************************************/
unsigned int mpc8xx_flash_addr( mpc8xx_flash_bank_t* fb, unsigned int addr )
{
	unsigned int val;

	val = (addr << fb->shift) + fb->start;

	return val;
}

/*****************************************************************/
unsigned int mpc8xx_flash_data( mpc8xx_flash_bank_t* fb, unsigned int data )
{
	unsigned int val;
	unsigned int i;

	val = fb->mask & data;

	/* copy data for all parallel devices */
	for (i = 1; i < fb->devices; i++) 
	{
		val |= ( val << fb->bits );
	}

	return val;
}

/*****************************************************************/

mpc8xx_flash_bank_t* mpc8xx_flash_find_bank(unsigned int Addr, int bVerbose)
{
	int n;
	mpc8xx_flash_bank_t* fb;

	fb = mpc8xx_flash_bank;
	for( n = 0; n < MPC8XX_MAX_FLASH_BANKS; n++, fb++ )
	{
		if ( (fb->start <= Addr) && ( fb->end >= Addr) && (fb->sName != NULL) )
		{
			/* found bank for addr */
			return fb;	
		}
	}

	if (bVerbose)
	{
		mpc8xx_printf("no FLASH bank found for Addr:0x%08x\n",Addr);
	}

	return 0;
}

mpc8xx_flash_sector_t* mpc8xx_flash_find_sector( mpc8xx_flash_bank_t* fb, unsigned int Addr, int bVerbose)
{
	int n;
	mpc8xx_flash_sector_t* fs;

	if( fb == 0 )
		return 0;

	fs = fb->Sector;
	for ( n = 0; n < fb->nSectors; n++, fs++ )
	{
		if ( ( fs->Start <= Addr ) && ( fs->End >= Addr ) )
		{
			/* found sector for addr */
			return fs;
		}
	}

	if (bVerbose)
	{
		mpc8xx_printf("no FLASH sector found for Addr:0x%08x in bank %d\n",Addr,n);
	}

	return 0;
}


/*****************************************************************/

int mpc8xx_flash_load_sequence( mpc8xx_flash_cycle_t* pCycle,
				unsigned int Addr, unsigned int Data, int bFirstTime )
{
	int bAdr;
	int bData;
	int bID;
	int cyc;
	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;

	pFB = mpc8xx_flash_find_bank( Addr, 0 );
	pFS = mpc8xx_flash_find_sector( pFB, Addr, 0 );

	if( (pCycle == 0 ) || (pFB == 0 ) || (pFS == 0 ) )
		return -1;
	
	cyc = 0;
	bAdr = 0;	/* actual address used at least once, loaded into r31*/
	bData = 0;	/* actual data used at least once, loaded into r0 */
	bID = 0;	/* actual id used at least once, loaded into r30 */

	while( ( cyc < MPC8XX_MAX_FLASH_CYCLES ) && ( pCycle[cyc].cType != 0 ) )
	{
		switch ( pCycle[cyc].cAdrType )
		{
		case 'a':
			if (!bAdr)
			{
				mpc8xx_set_gpr( 31, Addr );
				bAdr = 1;
			}
			pCycle[cyc].Adr = Addr; /* for debugging, insert actual address */
			break;
		case 'i':
			if (!bID)
			{
				mpc8xx_set_gpr( 30, mpc8xx_flash_addr( pFB, pFS->ID << pFB->IDshift ) );
				bID = 1;
			}
			/*
			 * for debugging, insert actual id address
			 */
			pCycle[cyc].Adr = mpc8xx_flash_addr( pFB, pFS->ID << pFB->IDshift ); 
			break;
		default:
			if (bFirstTime) /* load immediate data only once */
			{
				mpc8xx_set_gpr( (cyc << 1 ) + 1, pCycle[cyc].Adr );
			}
			break;
		}

		/*
		 * only for write cycles data has to be placed into registers
		 * the rest is to load actual cycle data for debug purposes
		 */
		switch ( pCycle[cyc].cDataType )
		{
		case 'd':
			if (pCycle[cyc].cType == 'w')
			{
				if (!bData)
				{
					mpc8xx_set_gpr( 0, Data );
					bData = 1;
				}
			}
			pCycle[cyc].Data = Data; /* for debugging, insert actual Data */
			break;
		case 'i':
			if (pCycle[cyc].cType == 'w')
			{
				if (!bID)
				{
					mpc8xx_set_gpr( 30, mpc8xx_flash_data( pFB, pFS->ID << pFB->IDshift ) );
					bID = 1;
				}
			}
			/*
			 * for debugging, insert actual Data
			 */
			pCycle[cyc].Data = mpc8xx_flash_data( pFB, pFS->ID << pFB->IDshift );
			break;
		default:
			if (pCycle[cyc].cType == 'w')
			{
				if (bFirstTime) /* load immediate data only once */
				{
					mpc8xx_set_gpr( ( cyc << 1 ) + 2, pCycle[cyc].Data );
				}
			}
			break;
		}
		cyc++;
	}

	return 0;
}

/*****************************************************************/
int mpc8xx_flash_execute_sequence( mpc8xx_flash_cycle_t *pCycle, unsigned int Addr, unsigned int Data, unsigned int toggle )
{
	int cyc;
	unsigned int val;
	unsigned int comload;
	unsigned int comstore;
	unsigned int rD; /* number of register holding data for cycle */
	unsigned int rA; /* number of register holding address for cycle */
	unsigned int comp = 0; /* for load related cycles value to compare with */
	unsigned int i;

	bdm_in_t in;
	bdm_out_t out;
	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;

	pFB = mpc8xx_flash_find_bank( Addr, 0 );
	pFS = mpc8xx_flash_find_sector( pFB, Addr, 0 );

	if( (pCycle == 0 ) || (pFB == 0 ) || (pFS == 0 ) )
		return -1;

	switch( pFB->width )
	{
	/*
	 * l/s w/h/b z rD,d(rA)
	 * com = (OP << 26) | (rD << 21) | (rA << 16) | d
	 */
	case 8:
		comload = (34 << 26); /*lbz rD,d(rA)*/
		comstore =(38 << 26); /*stb rS,d(rA)*/
		break;
	case 16:
		comload = (40 << 26); /*lhz rD,d(rA)*/
		comstore =(44 << 26); /*sth rS,d(rA)*/
		break;
	case 32:
		comload = (32 << 26); /*lwz rD,d(rA)*/
		comstore =(36 << 26); /*stw rS,d(rA)*/
		break;
	default:
		mpc8xx_printf("flash_execute_sequence: config error bank %d width %d\n",pFB->ID,pFB->width);
		return -1;
		break;
	}

	cyc = 0;

	while( ( cyc < MPC8XX_MAX_FLASH_CYCLES ) && ( pCycle[cyc].cType != 0 ) )
	{
		switch( pCycle[cyc].cAdrType )
		{
		case 'a':
			rA = 31;
			break;
		case 'i':
			rA = 30;
			break;
		default:
			rA = ( cyc << 1 ) + 1;
			break;
		}

		if( pCycle[cyc].cType == 'w' )
		{
			switch( pCycle[cyc].cDataType )
			{
			case 'd':
				rD = 0;
				break;
			case 'i':
				rD = 30;
				break;
			default:
				rD = ( cyc << 1 ) + 2;
				break;
			}
		}
		else /* for read related cycles store data in cycle data reg */
		{
			rD = ( cyc << 1 ) + 2;

			switch( pCycle[cyc].cDataType )
			{
			case 'd':
				comp = Data;
				break;
			case 'i':
				comp = mpc8xx_flash_data( pFB, pFS->ID << pFB->IDshift );
				break;
			default:
				comp = pCycle[cyc].Data;
				break;
			}
		}


		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
		{
			mpc8xx_printf("flash_execute_sequence: Cyc%d %c%c 0x%x:%c 0x%x\n",
				cyc,pCycle[cyc].cType,pCycle[cyc].cAdrType,
				pCycle[cyc].Adr,pCycle[cyc].cDataType,pCycle[cyc].Data);
		}

		switch( pCycle[cyc].cType )
		{
		case 'w':
			in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
			in.data =  comstore | (rD << 21) | (rA << 16); /*st[width] rS,0(rA)*/

			mpc8xx_bdm_clk_serial( &in, &out );
			break;
		case 'r':
			in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
			in.data =  comload | (rD << 21) | (rA << 16); /*l[width]w rD,0(rA)*/

			mpc8xx_bdm_clk_serial( &in, &out );

			val = mpc8xx_get_gpr( rD );
			if (comp != val)
			{
				mpc8xx_printf("flash_execute_cycle: read 0x%x got 0x%x\n",comp,val);
			}
			break;
		case 'c':
			in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
			in.data =  comload | (rD << 21) | (rA << 16); /*l[width]z rD,0(rA)*/

			mpc8xx_bdm_clk_serial( &in, &out );

			val = mpc8xx_get_gpr( rD );
			if (comp != val)
			{
				mpc8xx_printf("flash_execute_cycle: compare 0x%x got 0x%x\n",comp,val);
				return -1;
			}
			break;
		case 'p':
			in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
			in.data =  comload | (rD << 21) | (rA << 16); /*l[width]z rD,0(rA)*/

			for (i = 0; i < toggle; i++)
			{
				mpc8xx_bdm_clk_serial( &in, &out );

				/* poll only valid with immediate data */
				val = mpc8xx_get_gpr( rD ) & pCycle[cyc].Data;
				if ( !val )
					break;
			}
			if (i == toggle)
			{
				mpc8xx_printf("flash_execute_cycle: poll timed out\n");
				return -1;
			}
			break;
		case 't':
			in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
			in.data =  comload | (rD << 21) | (rA << 16); /*l[width]z rD,0(rA)*/

			mpc8xx_bdm_clk_serial( &in, &out );

			val = mpc8xx_get_gpr( rD ) & pCycle[cyc].Data; /* toggle only valid with immediate data*/
			for (i = 0; i < toggle; i++)
			{
				mpc8xx_bdm_clk_serial( &in, &out );

				comp = mpc8xx_get_gpr( rD ) & pCycle[cyc].Data;
				if( !(val ^ comp) )
					break; /* toggling stopped?*/
				val = comp;
			}
			if (i == toggle)
			{
				mpc8xx_printf("flash_execute_cycle: toggle timed out\n");
				return -1;
			}
			break;
		default:
			break;
		}
		cyc++;
	}

	return 0; /* success */
}

/*****************************************************************/
int mpc8xx_flash_erase( unsigned int addr, int toggle , int fquery )
{
	mpc8xx_target_registers_t regs;
	int nResult,i;

	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;

	pFB = mpc8xx_flash_find_bank( addr, 0 );
	pFS = mpc8xx_flash_find_sector( pFB, addr, 0 );

	if( (pFB == 0 ) || (pFS == 0 ) ){
		mpc8xx_printf( "mpc8xx_flash_erase: Unkown flash address\n");
		return -1;
	}

#if 0
	/*check if sector is protected*/
	Data = FBS.getword(flash_addr((FBS.pDevice->ADDR_VERIFYPROTECT)+((secID)<<FBS.IDshift)));
#endif


	if( fquery )
	{
		i = mpc8xx_query("erase FLASH bank %d sector %d [0x%08x,0x%08x] ?", pFB->ID, pFS->ID, pFS->Start, pFS->End);
		if (i != 1)
			return -1;
	}
	else
	{
		mpc8xx_printf("erasing FLASH bank %d sector %d [0x%08x,0x%08x]\n", pFB->ID, pFS->ID, pFS->Start, pFS->End);
	}

	mpc8xx_target_prepare( &regs ); /*save GPRs*/

	mpc8xx_flash_load_sequence( pFB->cErase, addr, 0, 1 );

	nResult = mpc8xx_flash_execute_sequence( pFB->cErase, addr, 0, toggle );

	mpc8xx_target_restore( &regs ); /*restore GPRs*/

	return nResult;
}
/*****************************************************************/
int mpc8xx_flash_clear( unsigned int Addr , int toggle , int fquery )
{
	int nResult,i;
	mpc8xx_target_registers_t regs;
	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;

	pFB = mpc8xx_flash_find_bank( Addr, 0 );
	pFS = mpc8xx_flash_find_sector( pFB, Addr, 0 );

	if( (pFB == 0 ) || (pFS == 0 ) )
		return -1;

	if( fquery )
	{
		i = mpc8xx_query("clear FLASH bank %d [0x%08x,0x%08x] ?", 1, pFB->start, pFB->end);
		if (i != 1)
			return 0;
	}
	else
	{
		mpc8xx_printf("clearing FLASH bank %d [0x%08x,0x%08x]\n", 1, pFB->start, pFB->end);
	}

	mpc8xx_target_prepare( &regs ); /*save GPRs*/

	mpc8xx_flash_load_sequence(pFB->cClear,Addr,0,1);

	nResult = mpc8xx_flash_execute_sequence(pFB->cClear,Addr,0, toggle);

	mpc8xx_target_restore( &regs); /*restore GPRs*/

	return nResult;
}
/*****************************************************************/
int mpc8xx_flash_write_word(unsigned int Addr, unsigned int Word, int bLoadSeq, int toggle )
{
/* have to be check for zero to ones before, destroys target register content */
	int nResult = -1;
	int i;

	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;

	pFB = mpc8xx_flash_find_bank( Addr, 0 );
	pFS = mpc8xx_flash_find_sector( pFB, Addr, 0 );

	if( (pFB == 0 ) || (pFS == 0 ) )
		return -1;

	for (i = pFB->EWA-1; i >=0; i--) /* loop elementar accesses for one 32 word, turn endianess */
	{
		nResult = mpc8xx_flash_load_sequence( pFB->cWrite, Addr + (i << pFB->shift ),
						      Word & pFB->mask,
						      ( (unsigned int)i == pFB->EWA-1 ) && ( bLoadSeq ) );
		if ( nResult < 0)
			break;

		nResult = mpc8xx_flash_execute_sequence( pFB->cWrite, Addr + (i << pFB->shift ),
							 Word & pFB->mask, toggle );
		if ( nResult < 0)
			break;

		Word = Word >> pFB->width;
	}

	if ( nResult < 0) /* write reset commando, if errors occured */
	{
		mpc8xx_flash_load_sequence( pFB->cReset, Addr, Word, 1 );
		mpc8xx_flash_execute_sequence( pFB->cReset, Addr, Word, toggle );
	}

	return nResult;
}

/*****************************************************************/
int mpc8xx_flash_write(unsigned int Addr, unsigned int Word, int toggle , int fquery )
{
	int nResult,i;
	unsigned int Data;
	unsigned int hData;
	mpc8xx_target_registers_t regs;
	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;

	pFB = mpc8xx_flash_find_bank( Addr, 0 );
	pFS = mpc8xx_flash_find_sector( pFB, Addr, 0 );

	if( (pFB == 0 ) || (pFS == 0 ) )
		return -1;

	if( fquery )
	{
		i = mpc8xx_query("write 0x%08x to FLASH at 0x%08x (bank %d sector %d [0x%08x,0x%08x]) ?", Word, Addr, pFB->ID, pFS->ID, pFS->Start, pFS->End);
		if (i != 1)
			return 0;
	}
	else
	{
		mpc8xx_printf("writing 0x%08x to FLASH at 0x%08x (bank %d sector %d [0x%08x,0x%08x])\n", Word, Addr, pFB->ID, pFS->ID, pFS->Start, pFS->End);
	}

	Data = mpc8xx_get_word( Addr ); /*Word found in FLASH*/

	hData = Word & ~Data; /* find ones to be written into zeros*/
	if (hData)
	{
		mpc8xx_printf("0x%08x: unable to write 0->1 : need to erase sector\n",Addr);
		return -1; /* failure */
	}

	mpc8xx_target_prepare( &regs ); /*save GPRs*/

	nResult = mpc8xx_flash_write_word( Addr, Word, 1, toggle );

	mpc8xx_target_restore( &regs ); /*restore GPRs*/

	if( nResult < 0 )
	{
		mpc8xx_printf("flash_write: failed to write 0x%08x to 0x%08x\n",Word,Addr);
	}

	return nResult;
}
/*****************************************************************/
int mpc8xx_flash_check_zeros_host(unsigned int* Buffer, unsigned int len,
				  unsigned int Destination, unsigned int* Adr)
{
	unsigned int i;
	int nResult;
	unsigned int r0;
	unsigned int r1;
	unsigned int Data;
	unsigned int hData;
	unsigned int Word;
	unsigned int wlen;
	bdm_in_t in;
	bdm_out_t out;

	nResult = 0;

	r0 = mpc8xx_get_gpr( 0 ); /* save scratch registers */
	r1 = mpc8xx_get_gpr( 1 );

	mpc8xx_set_gpr( 1, Destination - 4 );
	wlen = (len + 3) >> 2; /* word len */
	if( ( wlen << 2 ) != len )
	{
		mpc8xx_printf("flash_check_zeros_host: assert warning: len not divisible by 4");
	}

	*Adr = Destination;
	for( i=0; i < wlen ; i++)
	{
		/*
		 * Buffer is in host byte order, so we have to convert...
		 */

		Word = mpc8xx_extract_unsigned_integer( &(Buffer[i]), sizeof(Word), 1 );

		in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
		in.data =  0x84010004; /*lwzu    r0,4(r1)*/

		mpc8xx_bdm_clk_serial( &in, &out );

		Data = mpc8xx_get_gpr( 0 );

		hData = Word & ~Data; /* find ones to be written into zeros*/
		if (hData)
		{
			nResult = -1;
			break; /* failure, Destination Adr in *Adr */
		}
		(*Adr) += 4;
	}

	mpc8xx_set_gpr( 0, r0 ); /* restore registers */
	mpc8xx_set_gpr( 1, r1 );

	return nResult;
}
/*****************************************************************/
int mpc8xx_flash_check_zeros(unsigned int SourceStart, unsigned int SourceEnd,
			      unsigned int Destination, unsigned int * Adr,
			      int fast_flash , unsigned int prog_address )
{
	int i,n;
	int len;
	mpc8xx_target_registers_t regs;
	mpc8xx_target_program_t rem_prog;
	bdm_in_t in;
	bdm_out_t out;
	unsigned int mem_backup[5];
	unsigned int prg[5]=
	{
		0x84010004,	/*lwzu    r0,4(r1)*/
		0x84820004,	/*lwzu    r4,4(r2)*/
		0x7c002079,	/*andc.   r0,r0,r4*/
		0x4102fff4,	/*bdnzt   eq,-16*/
		0x00000000	/*.long 0x0*/
	};

	mpc8xx_target_prepare( &regs ); /* save GPRs*/

	if ( fast_flash ) /* using fast target routines?*/
	{
		/* prepare target execution */
		rem_prog.program_code = prg;
		rem_prog.program_len = 5;
		rem_prog.mem_backup = mem_backup;
		rem_prog.start_address = prog_address;

		if( mpc8xx_target_load( &rem_prog ) < 0 ) {
			*Adr = 0;

			mpc8xx_target_restore( &regs );

			return -1; /* failure */
		}

		/*
		 * parameters for program
		 */
		mpc8xx_set_gpr( 1, SourceStart - 4 );
		mpc8xx_set_gpr( 2, Destination - 4 );

		len = ( SourceEnd - SourceStart + 4 ) / 4;
		mpc8xx_set_spr( MPC8XX_SPR_CTR, len ); /* len */

		/*
		 * check for zeros needed to be programmed to 1
		 */
		if( mpc8xx_target_execute( &rem_prog, 100 ) < 0 ){
			mpc8xx_printf("mpc8xx_target_execute ERROR\n");

		}

		mpc8xx_target_unload( &rem_prog );
	} else {
		/* slow host side routines:*/

		mpc8xx_set_gpr( 1, SourceStart - 4 );
		mpc8xx_set_gpr( 2, Destination - 4 );

		len = ( SourceEnd - SourceStart + 4 ) / 4;

		for (i=len; i>0; i--)
		{
			/*
			 * we only need the first 3 instructions
			 * because we don't let the MPC handle the
			 * loop and SIE exception
			 */
			for( n = 0; n < 3; n++){
				in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
				in.data =  prg[0];
				mpc8xx_bdm_clk_serial( &in, &out );
			}

			if( mpc8xx_get_gpr( 0 ) != 0 )
				break;
		}
	}

	mpc8xx_target_restore( &regs ); /* restore GPRs*/

	*Adr = regs.gpr[ 2 ];

	return ( regs.gpr[ 0 ] == 0 ) ? 0 : -1;
}

/*****************************************************************/
unsigned int mpc8xx_flash_fast_program(
	unsigned int SourceStart,
	unsigned int SourceEnd,
	unsigned int Destination,
	unsigned int *Adr,
	unsigned int prog_address )
{
	int nResult;
	unsigned int len;

	mpc8xx_target_registers_t regs;
	mpc8xx_target_program_t rem_prog;
	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;

	pFB = mpc8xx_flash_find_bank( Destination, 0 );
	pFS = mpc8xx_flash_find_sector( pFB, Destination, 0 );

	*Adr = 0;

	if( (pFB == 0 ) || (pFS == 0 ) )
		return -1;

	if( (!pFB->fflash) || (pFB->nfflash <= 0) )
		return -2; /* no fast routine configured */


	mpc8xx_target_prepare( &regs );

	rem_prog.program_code = pFB->fflash;
	rem_prog.program_len = pFB->nfflash;
	rem_prog.mem_backup = (unsigned int*)malloc( pFB->nfflash * sizeof( unsigned int) );
	rem_prog.start_address = prog_address;

	if( mpc8xx_target_load( &rem_prog ) < 0 )
		return -3;
	
	/*
	 * preload registers for fast cycle
	 */
	nResult = mpc8xx_flash_load_sequence( pFB->cFast, Destination, 0, 1 );

	/*
	 * parameters for program
	 */
	mpc8xx_set_gpr( 28, SourceStart - pFB->align );
	mpc8xx_set_gpr( 29, Destination - pFB->align );

	len = ( SourceEnd - SourceStart + pFB->align ) / pFB->align;
	mpc8xx_set_spr( MPC8XX_SPR_CTR, len ); /*len*/

	mpc8xx_target_execute( &rem_prog, 100 );

	mpc8xx_target_unload( &rem_prog );

	mpc8xx_target_restore( &regs );
	
	(*Adr) = regs.gpr[ 29 ]; /* last written to FLASH adr */

	free( rem_prog.mem_backup );

	/* in case of no error, CTR should be equal to 0 */
	return ( regs.ctr == 0 ) ? 0 : -1;
}

/*****************************************************************/
int mpc8xx_flash_copy(unsigned int SourceStart, unsigned int SourceEnd,
			unsigned int Destination, int fflash,int toggle, int fquery, unsigned int prog_address )
{
	int i;
	int K,L;
	unsigned int Source,len;
	int nResult;
	unsigned int Adr;
	unsigned int Pos;
	unsigned int Word; /*for read operations*/
	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;
	bdm_in_t in;
	bdm_out_t out;
	mpc8xx_target_registers_t regs;

	if (SourceStart > SourceEnd)
	{
		mpc8xx_printf("error: source start > source end\n");
		return -1; /* failure */
	}

	len = SourceEnd - SourceStart;


	pFB = mpc8xx_flash_find_bank( Destination, 0 );
	pFS = mpc8xx_flash_find_sector( pFB, Destination, 0 );

	if( (pFB == 0 ) || (pFS == 0 ) )
		return -1;

	if (pFB->end < ( Destination + len ) )
	{
		mpc8xx_printf("error: destination range [0x%08x,0x%08x] exceeds bank range [0x%08x,0x%08x]\n",
				Destination, Destination+len, pFB->start, pFB->end);
		return -1;
	}
	if (((Destination>=SourceStart) && (Destination <=SourceEnd))||
		((Destination+len>=SourceStart) && (Destination+len <=SourceEnd)))
	{
		mpc8xx_printf("error: destination range [0x%08x,0x%08x] within source range [0x%08x,0x%08x]\n",
				Destination, Destination+len, SourceStart, SourceEnd);
		return -1;
	}
	if (SourceStart % pFB->align !=0)
	{
		mpc8xx_printf("error: start address has to be %d bytes aligned\n",pFB->align);
		return -1;
	}
	if (Destination%pFB->align !=0)
	{
		mpc8xx_printf("error: destination address has to be %d bytes aligned\n",pFB->align);
		return -1;
	}

   	mpc8xx_printf(" checking, if data can be written without erasing FLASH...\n");

	for (Pos = 0; Pos <= SourceEnd-SourceStart;)
	{
		/*search for zeros to be written to ones: sector needs erasure*/
		nResult = mpc8xx_flash_check_zeros( SourceStart+Pos, SourceEnd, Destination+Pos, &Adr , fflash, prog_address );
		if (nResult >= 0)
			break; /*ok to write data to flash*/

		if (Adr == 0) /* flash_check_zeros failed to setup target*/
		{
			mpc8xx_printf(" range not copied\n");
			return -1;
		}
		mpc8xx_printf("0x%08x: need to erase sector\n",Adr);
		i = mpc8xx_flash_erase( Adr , toggle , fquery ); /*try to erase sector*/
		if (i < 0)
		{
			mpc8xx_printf(" range not copied\n");
			return -1;
		}

		Pos = Adr - Destination; /* retry checking at last position */

		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
		{
			mpc8xx_printf("flash_copy Pos = 0x%08x\n",Pos);
		}
	}

	if (Pos > SourceEnd-SourceStart)
	{
		mpc8xx_printf("assert warning:  flash_copy Pos > SourceEnd-SourceStart 0x%08x\n",Pos);
	}

   	mpc8xx_printf("FLASH checked successfully!\n");

	if( fquery )
	{
		i = mpc8xx_query("Copy memory [0x%08x,0x%08x] to FLASH at [0x%08x,0x%08x] ?", SourceStart, SourceEnd, Destination, Destination +len);
		if (i!=1)
			return -1;
	}
	else
	{
		mpc8xx_printf("Copying memory [0x%08x,0x%08x] to FLASH at [0x%08x,0x%08x]\n", SourceStart, SourceEnd, Destination, Destination +len);
	}

	/*
	 * use fast flash ppc subroutines?
	 */
	if( fflash && ( pFB->fflash ) && ( pFB->nfflash > 0 ) && ( pFB->cFast ) )
	{
		nResult = mpc8xx_flash_fast_program(SourceStart,SourceEnd,Destination,&Adr,prog_address);
		if (nResult  < 0 )
		{
			mpc8xx_printf("error: FLASH failure at 0x%08x: nResult=0x%08x\n",Adr,nResult);
			return -1; /* failure */
		}

		return 0; /* success*/
	}
	if ( fflash )
	{
		if ( ( !(pFB->fflash) ) || (pFB->nfflash <= 0) )
		{
			mpc8xx_printf("no fast flash routine configured, using slow host routine\n");
		}
		else if( !(pFB->cFast) )
		{
			mpc8xx_printf("no fast flash routine register preload (cfast) configured, using slow host routine\n");
		}
	}

	/*
	 * use host controlled flash routine, very slow!
	 */
	mpc8xx_target_prepare( &regs ); /*save GPRs*/
	mpc8xx_set_gpr( 28, SourceStart - 4 ); /**/
	len = 0;
	K = 0;
	L = 0;
	for( Source = SourceStart; Source <= SourceEnd; Destination+=4,Source+=4)
	{
		in.prefix = MPC8XX_BDM_PREFIX_CORE_INSTRUCTION;
		in.data =  0x877c0004; /*   lwzu r27,4(r28) */
		mpc8xx_bdm_clk_serial( &in, &out );

		Word = mpc8xx_get_gpr( 27 );

		i = mpc8xx_flash_write_word( Destination, Word, ( Source == SourceStart ) , toggle );
		if (!i)
		{
			mpc8xx_target_restore( &regs ); /*restore GPRs*/
			return -1;/*stop if failed writing data to FLASH*/
		}
		len+=4;
		K+=4; /*count bytes*/
		if (K >= 4096) /*4KByte*/
		{
			mpc8xx_printf(".");
			K-=4096;
			L++;
			if (L == 32) /*32*4K = 128KByte*/
			{
				mpc8xx_printf( "%dK\n", len / 1024 );
				L = 0;
			}
		}
	}

	mpc8xx_target_restore( &regs ); /*restore GPRs*/

	mpc8xx_printf("\n copying finished.\n");

	return 0; /* success */
}

/*****************************************************************/
int mpc8xx_flash_info( int toggle )
{
	int bank;
	int nResult;
	mpc8xx_flash_bank_t* pFB;
	mpc8xx_flash_sector_t* pFS;

	for(bank=0; bank < MPC8XX_MAX_FLASH_BANKS; bank++) /*for all configured flash banks*/
	{
		pFB = &mpc8xx_flash_bank[ bank ];

		if( pFB->sName == NULL )
			continue;

		pFS = mpc8xx_flash_find_sector( pFB, mpc8xx_flash_bank[ bank ].start, 0 );

		if( (pFB == 0 ) || (pFS == 0 ) )
			continue;  /* misconfigured, should never happen*/

		mpc8xx_printf("FLASH bank %d %dx %d bit %s: %x-%x, adr align %d\n",
				bank,pFB->devices,pFB->bits,pFB->sName,
				pFB->start, pFB->end, pFB->align);

		nResult = mpc8xx_flash_ident( pFB->start, toggle ); /* test if device is responding */

#if 0
/* 		for (i = 0; i< FBS.pDevice->nSectors; i++)
		{
			Data = FBS.getword(flash_addr((FBS.pDevice->ADDR_VERIFYPROTECT)
								+((FBS.pDevice->Sector[i].ID)<<FBS.IDshift)));
			printf_filtered(" SA%02d Protect:%x",i,Data);
			printf_filtered(" Start:%08x",flash_addr(FBS.pDevice->Sector[i].Start));
			printf_filtered(" End:%08x\n",flash_addr(FBS.pDevice->Sector[i].End)+(FBS.width)/8-1);
		}
*/
#endif
	}

	return 0;
}


/*****************************************************************/
int mpc8xx_flash_file_find(FILE* f,char* sString, unsigned int * Start)
{
	int pos;
	int len;
	int nResult;

	pos = 0;
	len = strlen( sString );
	while( !feof(f) && !ferror(f) )
	{
		nResult = fgetc( f );
		if( nResult != sString[ pos ] )
		{
			pos = 0;
			continue;
		}
		pos++;
		if( pos == len )
		{
			if( Start != 0 )
				*Start = ftell( f );

			return 0;
		}
	}

	mpc8xx_printf("file error while searching for '%s'\n",sString);
	return -1;
}
/*****************************************************************/
int mpc8xx_flash_file_node( FILE* f, char* sString, unsigned int * Start)
{
	while( 1 )
	{
		if( feof(f) || ferror(f) )
			return -1;

		if( mpc8xx_flash_file_find(f, sString, Start ) < 0 )
		{
			mpc8xx_printf("error: '%s' not found.\n",sString );
			return -1;
		}

		if( mpc8xx_flash_file_skip_space(f,'{',Start) >= 0 )
		{
			return 0;
		}
	}

	return -1;
}
/*****************************************************************/
int mpc8xx_flash_free_bank( mpc8xx_flash_bank_t* pFB )
{

	if( pFB != NULL )
	{
		if( pFB->sName )
		{
			free( pFB->sName );
			pFB->sName = NULL;
		}
		if( pFB->Sector )
		{
			free( pFB->Sector );
			pFB->Sector = NULL;
		}
		if( pFB->cReset )
		{
			free( pFB->cReset );
			pFB->cReset = NULL;
		}
		if( pFB->cIdent )
		{
			free( pFB->cIdent );
			pFB->cIdent = NULL;
		}
		if( pFB->cWrite )
		{
			free( pFB->cWrite );
			pFB->cWrite = NULL;
		}
		if( pFB->cErase )
		{
			free( pFB->cErase );
			pFB->cErase = NULL;
		}
		if( pFB->cClear )
		{
			free( pFB->cClear );
			pFB->cClear = NULL;
		}
		if( pFB->cFast )
		{
			free( pFB->cFast );
			pFB->cFast = NULL;
		}
		if( pFB->fflash )
		{
			free( pFB->fflash );
			pFB->fflash = NULL;
			pFB->nfflash = 0;
		}
	}

	return 0;
}

mpc8xx_flash_bank_t* mpc8xx_flash_find_empty_bank()
{
	int i;
	mpc8xx_flash_bank_t* pFB = mpc8xx_flash_bank;

	for( i = 0; i < MPC8XX_MAX_FLASH_BANKS;i++, pFB++){
		if( pFB->sName == NULL ){
			pFB->ID = i;
			return pFB;
		}
	}

	return NULL;
}



/*****************************************************************/
int mpc8xx_flash_parse_cycle( 	mpc8xx_flash_bank_t* pFB, mpc8xx_flash_cycle_t** pCycle,
				FILE* f, char** FileBuffer, char** pParse )
{
	int len;
	int i;
	unsigned int j;
	int p;
	int cyc;
	int data;
	unsigned int val;
	char cType;

	*pCycle = malloc( sizeof( mpc8xx_flash_cycle_t ) * MPC8XX_MAX_FLASH_CYCLES );

	if( *pCycle == NULL )
	{
		mpc8xx_printf("flash_parse_cycle:unable to allocate mem\n");
		return -1;
	}

	for(cyc = 0; cyc < MPC8XX_MAX_FLASH_CYCLES; cyc++)
	{
		len = strlen( *pParse );

		for( i = 0; (i < len) && isspace( (*pParse)[i] ) ;i++)
			/* nothing */;

		if( i >= len )
			return -1;

		cType = tolower( (*pParse)[ i ] );
		switch(cType)
		{
		case 'w':
		case 'r':
		case 'c':
		case 't':
		case 'p':
			break;
		default:
			mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d '%s'\n",cyc,*pParse);
			return -1;
			break;
		}

		(*pCycle)[cyc].cType = cType;

		if ( (i+1 < len) && ( (*pParse)[i+1]=='%') ) /* special ADR? */
		{
			if( i+2 >= len )
			{
				mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, missing adr '%s'\n",cyc,*pParse);
				return -1;
			}

			cType = tolower( (*pParse)[i+2] );
			switch(cType)
			{
			case 'a':
				break;
			case 'i':
				break;
			default:
				mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, special adr '%s'\n",cyc,*pParse);
				return -1;
				break;
			}

			(*pCycle)[cyc].cAdrType = cType;
			i += 3;
		}
		else
		{
			(*pCycle)[cyc].cAdrType = ' ';
			sscanf( (*pParse)+i+1, "%i%n", &data, &p );
			if (p==0)
			{
				mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, adr '%s'\n",cyc,*pParse);
				return -1;
			}
			(*pCycle)[cyc].Adr = (((unsigned int)data) << pFB->shift) + pFB->start;
			i+=p+1;
		}
		if (i >= len)
		{
			mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, missing ':' '%s'\n",cyc,*pParse);
			return -1;
		}
		if ((*pParse)[i] != ':')
		{
			mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, missing ':' '%s'\n",cyc,*pParse);
			return -1;
		}
		i++;
		if (i >= len)
		{
			mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, missing data '%s'\n",cyc,*pParse);
			return -1;
		}
		if ((*pParse)[i]=='%') /* special DATA? */
		{
			if (i+1>=len)
			{
				mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, missing data '%s'\n",cyc,*pParse);
				return -1;
			}
			cType = tolower( (*pParse)[i+1] );
			switch(cType)
			{
			case 'd':
				break;
			case 'i':
				break;
			default:
				mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, special data '%s'\n",cyc,*pParse);
				return -1;
				break;
			}
			(*pCycle)[cyc].cDataType = cType;
			i+=2;
		}
		else
		{
			(*pCycle)[cyc].cDataType = ' ';
			sscanf( (*pParse)+i,"%i%n",&data,&p );
			if (p==0)
			{
				mpc8xx_printf("flash_parse_cycle: incorrect cycle #%d, data '%s'\n",cyc,*pParse);
				return -1;
			}
			val = pFB->mask & (unsigned int)data;
			for (j = 1; j < pFB->devices; j++) /* copy data for all parallel devices */
			{
				val |= ( val << pFB->bits );
			}
			(*pCycle)[cyc].Data = val;
			i+=p;
		}

		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
		{
			mpc8xx_printf("flash_parse_cycle: Cyc %d %c%c0x%x:%c0x%x\n",cyc,(*pCycle)[cyc].cType,(*pCycle)[cyc].cAdrType,(*pCycle)[cyc].Adr,(*pCycle)[cyc].cDataType,(*pCycle)[cyc].Data);
		}

		(*pParse) += i;
		len = strlen( *pParse );
		for(i = 0;(i<len) && ( isspace( (*pParse)[i] ) ); i++ )
			/* nothing */;

		if (i >= len) /* end of line, no ',' -> end of cycles */
		{
			cyc++;
			break;
		}
		if ((*pParse)[i] == ',')
		{
			(*pParse)++;
		}
		len = strlen(*pParse);

		for(i = 0;( i < len ) && ( isspace( (*pParse)[i] ) ); i++)
			;

		if (i >= len) /* ',' + eol -> read next line */
		{
			fgets( (*FileBuffer), MPC8XX_MAX_BUFFER_LEN, f );
			(*pParse) = (*FileBuffer);
		}
	}
	if (cyc < MPC8XX_MAX_FLASH_CYCLES )
	{
		(*pCycle)[cyc].cType = 0; /* mark end of cycles */
	}

	return 0; /* successfully parsed cycles */
}




/*****************************************************************/
int mpc8xx_flash_file_skip_space( FILE *f, char cChar, unsigned int* Start )
{
	int nResult;


	while( !feof(f) && !ferror(f) )
	{
		nResult = fgetc(f);

		if( nResult == cChar )
		{
			if (Start!=0) *Start = ftell(f);
			return 0;
		}
		if( ! isspace(nResult)  )
		{
			ungetc(nResult,f);
			return -1;
		}
	}

	if( ferror(f) )
	{
		mpc8xx_printf("file error while skipping whitespace for '%c'\n",cChar);
	}

	return -1;
}


/*****************************************************************/

#define ERROR_RETURN( res ) 		\
	mpc8xx_flash_free_bank( pFB );	\
	if( f != NULL )			\
		fclose(f);		\
	return (res)			

#define OK_RETURN			\
	if( f != NULL )			\
		fclose(f);		\
	return 0

int mpc8xx_flash_configure( unsigned int Addr, unsigned int Num, const char* sFileName, const char* sDeviceName )
{
	FILE* f = NULL;
	char Buffer[ MPC8XX_MAX_BUFFER_LEN ];
	char sSector[ MPC8XX_MAX_BUFFER_LEN ];
	char sAlgo[ MPC8XX_MAX_BUFFER_LEN ];
	char sFast[ MPC8XX_MAX_BUFFER_LEN ];
	char FileBuffer[ MPC8XX_MAX_BUFFER_LEN ];
	int len;
	int i;
	int nResult;
	int nComma;
	unsigned int nStart;
	unsigned int adr;
	unsigned int prg;
	mpc8xx_flash_bank_t* pFB;
	char* pParse;
	char* pFileBuffer = FileBuffer;

	sFast[ 0 ] = 0;

	/*
	 * Check if we already have a FLASH at the given localtion
	 * if so simply delete it and reconfigure it with the new info
	 */
	pFB = mpc8xx_flash_find_bank( Addr, 0 );
	if( pFB != NULL )
		mpc8xx_flash_free_bank( pFB );

	/*
	 * Find a new empty flash bank entry
	 */
	pFB = mpc8xx_flash_find_empty_bank();
	if( pFB == NULL ){
		mpc8xx_printf("Could not find and empty bank\n");
		return -1;
	}

	/*
	 * Skip white space before the device name
	 */
	while( isspace( *sDeviceName ) )
		sDeviceName++;

	len = strlen( sDeviceName );

	if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
	{
		mpc8xx_printf("configuring bank 0x%08x as %u * %s\n",Addr,Num,sDeviceName);
	}

	pFB->sName = malloc( len + 1 );
	if( pFB->sName == NULL )
	{
		mpc8xx_printf("flash_configure:unable to allocate mem for Name\n");

		ERROR_RETURN( -1 );
	}


	strncpy( pFB->sName, sDeviceName , len + 1 );
	pFB->start = Addr;
	pFB->devices = Num;

	f = fopen( sFileName , "rb" );

	if( f == NULL )
	{
		mpc8xx_printf("failed to open flash configuration file.");

		ERROR_RETURN( -1 );
	}

	sprintf( Buffer, ".device %s", sDeviceName );

	if( mpc8xx_flash_file_node( f, Buffer, &nStart ) < 0 )
	{
		mpc8xx_printf("mpc8xx_flash_file_node failed.");

		ERROR_RETURN( -1 );
	}

	while( 1 ){
		if( feof(f) || ferror(f) )
		{
			mpc8xx_printf("file error while parsing .device %s", sFileName);
			ERROR_RETURN( -1 );
		}

		nResult = mpc8xx_flash_file_skip_space( f, '}' , NULL );
		if( nResult >= 0 )
		{
			/* found the end '}'  exit the while(1) loop */
			break;
		}

		nResult = mpc8xx_flash_file_skip_space( f, '.', NULL );
		if( nResult >= 0 )
		{
			/* found '.' */
			fgets( Buffer, sizeof( Buffer ), f );
			pParse = Buffer;
			if( strncmp( pParse, "size", 4 ) == 0 )
			{
				pParse += 4;
				pFB->size = strtoul( pParse, &pParse, 0 );
				if( pParse[0] == 'M' )
				{
					pFB->size *= 1048576;
				}
				else if( pParse[0] == 'K' )
				{
					pFB->size *= 1024;
				}

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("%s.size %u\n",pFB->sName,pFB->size);
				}

			}
			else if( strncmp( pParse, "width", 5 ) == 0 )
			{
				pParse += 5;
				pFB->bits = strtoul( pParse, &pParse, 0 );

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("%s.width %u\n",pFB->sName,pFB->bits);
				}

			}
			else if( strncmp( pParse, "sector", 6 ) == 0 )
			{
				pParse += 6;
				sscanf( pParse, "%s", sSector );

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("%s.sector %s\n",pFB->sName,sSector);
				}

			}
			else if( strncmp( pParse, "idoffset", 8 ) == 0 )
			{
				pParse += 8;
				pFB->IDshift = strtoul( pParse, &pParse, 0 );

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("%s.idoffset %u\n",pFB->sName,pFB->IDshift);
				}

			}
			else if( strncmp( pParse, "algorithm", 9 ) == 0)
			{
				pParse += 9;
				sscanf( pParse, "%s", sAlgo );

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("%s.algorithm %s\n",pFB->sName,sAlgo);
				}

			}
			else
			{
				mpc8xx_printf("wrong syntax: %s",Buffer);
				ERROR_RETURN( -1 );
			}
		}
	}

	pFB->width = pFB->devices * pFB->bits;
	pFB->EWA = 32 / pFB->width; /* number of elementar accesses needed for a 32 bit word */
	pFB->fflash = NULL;
	pFB->nfflash = 0;

	switch ( pFB->width)
	{
	case 8:
		pFB->mask = 0xff;
		pFB->align = 1;
		pFB->shift = 0;
		pFB->getword = mpc8xx_get_byte;
		pFB->setword = mpc8xx_set_byte;
		break;
	case 16:
		pFB->mask = 0xffff;
		pFB->align = 2;
		pFB->shift = 1;
		pFB->getword = mpc8xx_get_halfword;
		pFB->setword = mpc8xx_set_halfword;
		break;
	case 32:
		pFB->mask = 0xffffffff;
		pFB->align = 4;
		pFB->shift = 2;
		pFB->getword = mpc8xx_get_word; /* set default routines to 32 bit */
		pFB->setword = mpc8xx_set_word; /* set default routines to 32 bit */
		break;
	default:
		/* mpc8xx_printf("FLASH bank %d misconfigured: found width %d for Addr:0x%08x\n",mpcbdm_nBank,pFB->width,Addr); */

		ERROR_RETURN( -1 );
		break;
	}

	pFB->end = pFB->start + pFB->devices * pFB->size -1;

	if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
	{
		mpc8xx_printf("%s.end 0x%08x\n",pFB->sName, pFB->end );
	}

	sprintf(Buffer,".sector %s",sSector);
	if( mpc8xx_flash_file_node( f, Buffer, &nStart ) < 0 )
	{
		ERROR_RETURN( -1 );
	}

	pFB->Sector = malloc( sizeof(mpc8xx_flash_sector_t) * MPC8XX_MAX_FLASH_SECTORS);
	if( pFB->Sector == NULL )
	{
		mpc8xx_printf("flash_configure:unable to allocate mem for sectors\n");
		ERROR_RETURN( -1 );
	}

	pFB->nSectors = 0;
	nComma = 0;
	for (i = 0; i < MPC8XX_MAX_FLASH_SECTORS; i++)
	{
		if( feof(f) || ferror(f) )
		{
			mpc8xx_printf("file error while parsing .device %s", sFileName);
			ERROR_RETURN( -1 );
		}

		if( mpc8xx_flash_file_skip_space(f,'}',NULL) >= 0 )
		{
			/*
			 * found } token
			 */
			if( nComma == 1 )
			{

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("flash_configure: unnecessary ',' in .sector %s\n",sSector);
				}
			}
			break;
		}
		if( (i != 0 ) && ( nComma == 0 ) )
		{

			if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
			{
				mpc8xx_printf("flash_configure: missing ',' in .sector %s entry %d\n",sSector,i);
			}
			break;
		}

		if( fscanf(f,"%i",&adr) != 1 ){
			ERROR_RETURN( -1 );
		}

		pFB->Sector[i].ID = adr;
		if( mpc8xx_flash_file_skip_space(f,',',NULL) < 0 ){
			ERROR_RETURN( -1 );
		}

		if( fscanf(f,"%i",&adr) != 1 ){
			ERROR_RETURN( -1 );
		}

		pFB->Sector[i].Start = ( adr << pFB->shift) + pFB->start;
		if( mpc8xx_flash_file_skip_space(f,',',NULL) < 0 )
		{
			ERROR_RETURN( -1 );
		}

		if( fscanf(f,"%i",&adr) != 1 ){
			ERROR_RETURN( -1 );
		}

		pFB->Sector[i].End = ( adr << pFB->shift) + pFB->start +(pFB->width) / 8-1;

 	 	if( mpc8xx_flash_file_skip_space(f,',',NULL) < 0 )
		{
			if( mpc8xx_flash_file_skip_space(f,'}',NULL) < 0 ){
				ERROR_RETURN( -1 );
			} else {
				break;
			}
		} else {
			nComma = 1;
		}
	}

	pFB->nSectors = i;
	if ( ( i > 0) && ( pFB->Sector[ i - 1 ].End != pFB->end ) )
	{
		if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
		{
			mpc8xx_printf("end of last sector not equal to end of bank?\n");
		}
	}

	sprintf( Buffer,".algorithm %s", sAlgo );
	if( mpc8xx_flash_file_node( f, Buffer, &nStart ) < 0 )
	{
		ERROR_RETURN( -1 );
	}

	while( 1 )
	{
		if( feof(f) || ferror(f) )
		{
			mpc8xx_printf("file error while parsing .device %s", sFileName);
			ERROR_RETURN( -1 );
		}

		if( mpc8xx_flash_file_skip_space(f,'/',NULL) >= 0 )
		{
			/* comment, skip end of row */
			fgets( FileBuffer, sizeof(FileBuffer), f );

			continue;
		}

		if( mpc8xx_flash_file_skip_space( f,'}',NULL) >= 0 )
		{
			/* found end } exit while loop */
			break;
		}


		if( mpc8xx_flash_file_skip_space( f, '.', NULL) >= 0 )
		{
			fgets( FileBuffer, sizeof(FileBuffer), f );
			pParse = FileBuffer;
			if( strncmp( pParse, "reset", 5) == 0)
			{
				pParse += 5;

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("flash_configure: %s.reset\n",pFB->sName);
				}

				if( mpc8xx_flash_parse_cycle( pFB, &pFB->cReset, f, &pFileBuffer, &pParse ) < 0 )
				{
					ERROR_RETURN( -1 );
				}
			}
			else if( strncmp( pParse, "ident", 5 ) == 0 )
			{
				pParse += 5;

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("flash_configure: %s.ident\n",pFB->sName);
				}

				if( mpc8xx_flash_parse_cycle( pFB, &pFB->cIdent,f, &pFileBuffer, &pParse) < 0 )
				{
					mpc8xx_printf("error in .algorithm %s .ident\n",sAlgo);
					ERROR_RETURN( -1 );
				}
			}
			else if( strncmp( pParse, "write", 5) == 0 )
			{
				pParse += 5;

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("flash_configure: %s.write\n",pFB->sName);
				}

				if( mpc8xx_flash_parse_cycle( pFB, &pFB->cWrite,f,&pFileBuffer, &pParse) < 0 )
				{
					ERROR_RETURN( -1 );
				}
			}
			else if( strncmp( pParse, "erase", 5 ) == 0 )
			{
				pParse += 5;

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("flash_configure: %s.erase\n",pFB->sName);
				}

				if( mpc8xx_flash_parse_cycle( pFB, &pFB->cErase ,f, &pFileBuffer, &pParse) < 0 )
				{
					mpc8xx_printf("error in .algorithm %s .erase\n",sAlgo);
					ERROR_RETURN( -1 );
				}
			}
			else if( strncmp( pParse, "clear", 5 ) == 0 )
			{
				pParse += 5;

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("flash_configure: %s.clear\n",pFB->sName);
				}

				if( mpc8xx_flash_parse_cycle( pFB, &pFB->cClear,f,&pFileBuffer, &pParse) < 0 )
				{
					mpc8xx_printf("error in .algorithm %s .clear\n",sAlgo);
					ERROR_RETURN( -1 );
				}
			}
			else if( strncmp( pParse, "cfast", 5 ) == 0)
			{
				pParse += 5;

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("flash_configure: %s.cfast\n",pFB->sName);
				}

				if( mpc8xx_flash_parse_cycle( pFB, &pFB->cFast,f,&pFileBuffer, &pParse ) < 0 )
				{
					mpc8xx_printf("error in .algorithm %s .cfast\n",sAlgo);
					return ERROR_RETURN( -1 );
				}
			}
			else if (strncmp( pParse, "fast", 4) == 0 )
			{
				pParse += 4;
				sscanf( pParse, "%s", sFast );

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("%s.fast%d %s\n",pFB->sName,pFB->width,sFast);
				}
			}
			else
			{
				mpc8xx_printf("wrong syntax: %s",FileBuffer);
				ERROR_RETURN( -1 );
			}
		}
	}

	if( sFast[0] != 0 )
	{
		sprintf( Buffer, ".fast%d %s", pFB->width, sFast );

		if( mpc8xx_flash_file_node( f, Buffer, &nStart ) < 0 )
		{
			ERROR_RETURN( -1 );
		}

		pFB->fflash = malloc( sizeof(unsigned int) * MPC8XX_MAX_FLASH_PRG );
		if( !pFB->fflash )
		{
			mpc8xx_printf("flash_configure:unable to allocate mem for fast flash routine\n");
			ERROR_RETURN( -1 );
		}

		nComma = 0;
		i = 0;
		while( 1 )
		{
			if( feof(f) || ferror(f) )
			{
				mpc8xx_printf("file error while parsing .device %s", sFileName);
				ERROR_RETURN( -1 );
			}

			if( mpc8xx_flash_file_skip_space(f,'}',NULL) >= 0 )
			{
				if( nComma == 1 )
				{

					if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
					{
						mpc8xx_printf("flash_configure: unnecessary comma...\n");
					}
				}
				break;
			}

			if( mpc8xx_flash_file_skip_space(f,'/',NULL) < 0 )
			{
				/* not a / comment char */

				if(fscanf(f,"%x",&prg) != 1 )
				{
					mpc8xx_printf("flash_configure: %s expected instruction\n",Buffer);
					ERROR_RETURN( -1 );
				}

				if( i >= MPC8XX_MAX_FLASH_PRG )
				{
					mpc8xx_printf("flash_configure: %s too many instructions\n",Buffer);
					ERROR_RETURN( -1 );
				}

				pFB->fflash[i] = prg;

				if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
				{
					mpc8xx_printf("flash_configure: fflash[%d] =0x%08x\n",i,pFB->fflash[i]);
				}

				i++;
				if( mpc8xx_flash_file_skip_space(f,',',NULL) >= 0 )
					nComma = 1;
			}
			else 
			{
				/* comment, skip end of row */
				fgets( FileBuffer, sizeof(FileBuffer), f );
			}
		}

		pFB->nfflash = i;
	}

	if( mpc8xx_verbose_level( MPC8XX_VERBOSE_FLS ) )
	{
		mpc8xx_printf("flash bank configured.\n");
	}


	OK_RETURN;
}
/*****************************************************************/
int mpc8xx_flash_bank_reset(unsigned int Adr, int toggle )
{
	mpc8xx_target_registers_t regs;
	mpc8xx_flash_bank_t* pFB;

	pFB = mpc8xx_flash_find_bank( Adr, 1 );
	if( !pFB )
		return -1; /* no valid bank configuration found */

	mpc8xx_target_prepare( &regs );

	mpc8xx_flash_load_sequence( pFB->cReset, Adr, 0, 1 );
	mpc8xx_flash_execute_sequence( pFB->cReset, Adr, 0, toggle );

	mpc8xx_target_restore( &regs );

	return 0;
}

/*****************************************************************/
int mpc8xx_flash_ident(unsigned int Adr, int toggle )
{
	int nResult;
	mpc8xx_target_registers_t regs;
	mpc8xx_flash_bank_t* pFB;

	pFB = mpc8xx_flash_find_bank( Adr, 1 );
	if (!pFB)
		return -1; /* no valid bank configuration found */

	mpc8xx_target_prepare( &regs );

	mpc8xx_flash_load_sequence( pFB->cIdent, Adr, 0, 1 );
	nResult = mpc8xx_flash_execute_sequence( pFB->cIdent, Adr, 0, toggle );

	mpc8xx_target_restore( &regs );
	if( nResult < 0 )
	{
		mpc8xx_printf("Ident Sequence: failure?\n");
	}
	else
	{
		mpc8xx_printf("Ident Sequence: Ok!\n");
	}

	return nResult;
}


/*****************************************************************/
int mpc8xx_flash_program_file( const char* sFilename, unsigned int destaddr, int fflash, int toggle, int fquery,
				unsigned int target_buffer_address, unsigned int target_buffer_size,
				unsigned int prog_address )
{
	unsigned int i;
	unsigned int len;
	FILE * pf;
	int nResult = -1;
	int bUseFast;
	unsigned int Adr;
	unsigned int Data;

	unsigned int	nTotal;
	unsigned int	nBuffer;
	unsigned int	nLeft;
	mpc8xx_flash_bank_t* pFB;
	mpc8xx_target_registers_t regs;

	unsigned char * buffer = NULL;
	unsigned char * rbuffer = NULL;

	pFB = mpc8xx_flash_find_bank( destaddr, 0 );
	if( pFB == NULL )
		return -1;

	if( target_buffer_address % 4 )
	{
		mpc8xx_printf("mpcbdm_fbuf must be divisible by 4.\n");
		return -1;
	}
	if( target_buffer_size % 4 )
	{
		mpc8xx_printf("mpcbdm_fsizebuf must be divisible by 4.\n");
		return -1;
	}

	if( destaddr % pFB->align != 0 )
	{
		mpc8xx_printf("error: destination address has to be %d bytes aligned\n",pFB->align);
		return -1; /* failure */
	}

	buffer = (unsigned char*)malloc( target_buffer_size );
	if( buffer == NULL )
	{
		mpc8xx_printf("flash_program_file: unable to alloc host mem for buffer\n");
		return -1;
	}

	/*
	 * can we use fast flash ppc subroutines?
	 */
	bUseFast = 0;
	if( fflash && pFB->fflash && ( pFB->nfflash > 0 ) && pFB->cFast )
	{
		/*
		 * so it's faster to copy bufferwise into target and execute target
		 * flashing per buffer this mode seems only usefull, if using DPRAM and
		 * no other properly configured RAM is available else using 'mem load' and
		 * then 'flash copy' is more efficient
		 */
		rbuffer = (unsigned char*)malloc( target_buffer_size );
		if( rbuffer == NULL ){
			mpc8xx_printf("flash_program_file: unable to alloc host mem for rbuffer\n");
			return -1;
		}

		/*
		 * save target buffer content
		 */
		if( mpc8xx_read_block( target_buffer_address, rbuffer, target_buffer_size ) < 0 ){
			mpc8xx_printf("flash_program_file: unable to read traget mem for rbuffer\n");
			return -1;
		}

		bUseFast = 1;

		mpc8xx_printf("Copying to flash with target routines using buffer[0x%08x,0x%08x]...\n",
				target_buffer_address, target_buffer_address + target_buffer_size - 1 );
	}
	else if( fflash )
	{
		if( ( !(pFB->fflash) ) || ( pFB->nfflash <= 0 ) )
		{
			mpc8xx_printf("no fast flash routine configured\n");
		}
		else if( !( pFB->cFast ) )
		{
			mpc8xx_printf("no fflash register preload (cfast) configured\n");
		}
	}


	do /* for freeing resources on error, break */
	{
		pf = fopen( sFilename, "rb" );
		if( pf == NULL ) /* could not open the file */
		{
			mpc8xx_printf("Could not open the file\"%s\"\n",sFilename);
			break;
		}

		if( fseek(pf, 0, SEEK_END) < 0 )
		{
			mpc8xx_printf("fseek error.");
			break;
		}

		len = (unsigned int)ftell( pf );
		rewind( pf );

		if( destaddr + len > pFB->end)
		{
			mpc8xx_printf("error: destination end 0x%08x exceeds bank border 0x%08x\n",destaddr+len,pFB->end);
			break; /* failure */
		}


		i = mpc8xx_query("Copy file \"%s\" len 0x%08x to FLASH at [0x%08x,0x%08x] ?",sFilename, len, destaddr, destaddr+len-1);
	    	if( i != 1 )
			break;

		/*
		 * checking if we have to erase
		 */
		for( nTotal = 0, nLeft = len; nLeft > 0; nTotal += nBuffer, nLeft -= nBuffer )
		{
			if( nLeft > target_buffer_size )
			{
				nBuffer = target_buffer_size;
			}
			else
			{
				nBuffer = nLeft;
			}

			if( fread( buffer, 1, nBuffer, pf ) != nBuffer )
			{
				mpc8xx_printf("read error.");
				nResult = -1;
				break;
			}

			/*
			 * check buffer for zero writing to ones
			 */
			mpc8xx_printf("checking FLASH [0x%08x,0x%08x]...\n",destaddr+nTotal,destaddr+nTotal+nBuffer-1);

			if( bUseFast )
			{
				mpc8xx_write_block( target_buffer_address, buffer, nBuffer );

				nResult = mpc8xx_flash_check_zeros( target_buffer_address,
							target_buffer_address + nBuffer - 1,
							destaddr + nTotal, &Adr , fflash , prog_address );
			}
			else
			{
				nResult = mpc8xx_flash_check_zeros_host(
					(unsigned int *)buffer, nBuffer, destaddr + nTotal, &Adr );
			}

			if( nResult < 0 )
			{
				mpc8xx_printf("need to erase sector for 0x%08x\n", Adr );

				/* try to erase sector */
				nResult = mpc8xx_flash_erase( Adr , toggle , fquery );
				if( nResult < 0 )
				{
					mpc8xx_printf(" range not erased\n");
					break; /* failure, exit for loop */
				}
			}
		}

		if( nResult < 0 )
			break; /* for loop failure */

		rewind( pf );

		if( bUseFast )
		{
			mpc8xx_printf("Copying to Flash using target routines...\n");
		}
		else
		{
			mpc8xx_printf("Copying to Flash using host routines...\n");

			mpc8xx_target_prepare( &regs ); /*save GPRs*/
		}

		for (nTotal = 0, nLeft = len; nLeft > 0; nTotal += nBuffer, nLeft -= nBuffer)
		{
			if(nLeft > target_buffer_size )
				nBuffer = target_buffer_size;
			else
				nBuffer = nLeft;

			if( fread(buffer, 1, nBuffer, pf) != nBuffer )
			{
				mpc8xx_printf("read error.");
				nResult = -1;
				break;
			}

			mpc8xx_printf("writing FLASH [0x%08x,0x%08x]...\n",
				destaddr + nTotal, destaddr + nTotal + nBuffer - 1 );

			if( bUseFast )
			{
				nResult = mpc8xx_write_block( target_buffer_address, buffer, nBuffer );

				if( nResult < 0 ){
					mpc8xx_printf("error: failed to write target memory\n");
					break;
				}

				nResult = mpc8xx_flash_fast_program( target_buffer_address,
						target_buffer_address+nBuffer-1, destaddr+nTotal, &Adr,prog_address);

				if (nResult < 0)
				{
					mpc8xx_printf("error: FLASH failure at 0x%08x: nResult=0x%08x\n",Adr,nResult);
					break;
				}
			}
			else
			{
				for (i = 0; i < nBuffer; i+=4)
				{
					if (i+3 >= nBuffer)
					{
						mpc8xx_printf("buffer len not divisible by 4, ommiting last subword\n");
						nResult = -1;
						break;
					}

					/*
					 * we have to correct endianess here:
					 */
					Data = mpc8xx_extract_unsigned_integer( buffer+i, sizeof( Data ) , 1 );
					/* Data = *(unsigned int *)&(buffer[i]); */
					nResult = mpc8xx_flash_write_word( destaddr + nTotal + i, Data,
						( (nTotal == 0) && (i == 0) ), toggle );
					if( nResult < 0)
					{
						mpc8xx_printf("error: FLASH failure at 0x%08x: nResult=0x%08x\n",Adr,nResult);
						break;
					}
				}
			}

			if ( nResult < 0 ) break;
		}
		if( !bUseFast )
		{
			mpc8xx_target_restore( &regs ); /*restore GPRs*/
		}
		break;
	} while (1); /* for freeing resources on error, break */

	/*
	 * restore target buffer content, if any
	 */
	mpc8xx_write_block( target_buffer_address, rbuffer, target_buffer_size );
	free( rbuffer );
	free(buffer);

	if( ferror(pf) )
	{
		mpc8xx_printf(" file error while reading \"%s\" \n", sFilename );
	}

	fclose( pf );

	return 0;
}

int mpc8xx_flash_program_file_fast( const char* sFilename, unsigned int destaddr, int toggle, int fquery,
				unsigned int target_buffer_address, unsigned int target_buffer_size,
				unsigned int prog_address )
{
	int res;
	int len;

	len = mpc8xx_mem_load( sFilename, target_buffer_address, target_buffer_size );

	if( len < 0 ){
		mpc8xx_printf("mpc8xx_flash_program_file_fast: load error\n");
		return -1;
	}
	res = mpc8xx_flash_copy( target_buffer_address, target_buffer_address + len,
				destaddr, 1, toggle, fquery, prog_address );

	if( res < 0 ){
		mpc8xx_printf("mpc8xx_flash_program_file_fast: copy error\n");
		return -1;
	}

	return len;
}
