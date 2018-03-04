/**
 * @file	mpc8xxflash.h
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

#ifndef __MPC8XXFLASH_H__
#define __MPC8XXFLASH_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MPC8XX_MAX_FLASH_BANKS		4
#define MPC8XX_MAX_FLASH_SECTORS	1024
#define MPC8XX_MAX_FLASH_CYCLES		10	/* using maximum 2*10 ppc registers for flash preload*/
#define MPC8XX_MAX_FLASH_PRG		100
#define MPC8XX_MAX_BUFFER_LEN		(8*1024)

/* FLASH device sector description */

typedef struct mpc8xx_flash_sector_s
{
	unsigned int ID;
	unsigned int Start;
	unsigned int End;
} mpc8xx_flash_sector_t;

/* FlashCycle: an action paired with a adr:data tupel */
typedef struct mpc8xx_flash_cycle_s
{
	char cType; /* cycle action : one of W R C T P */
	unsigned int Adr;
	char cAdrType;
	unsigned int Data;
	char cDataType;
} mpc8xx_flash_cycle_t;

typedef struct mpc8xx_flash_bank_s
{
	char* sName;			/* name of FLASH device*/
	unsigned int ID;		/* ID number */
	mpc8xx_flash_sector_t* Sector;	/* array of sectors*/
	int nSectors;			/* number of sectors*/
	unsigned int bits;		/* number of bits per device */
	unsigned int devices;		/* number of devices parallel in bank */
	unsigned int start;		/* physical start address of bank */
	unsigned int size;		/* size per device */
	unsigned int end;		/* physical end address*/
	unsigned int IDshift;		/* bit offset for sector id*/
	unsigned int width;		/* number of data lines connected to data bus for total bank */
	unsigned int mask;		/* (1<<width)-1, bit mask for elementar access */
	unsigned int EWA;		/* number of elementar accesses needed per word = 32/width */
	unsigned int align;		/* align  = width/8 , one address increase results in 'align' bytes increase */
	unsigned int shift;		/* ld(align) = bit shift left to get physical mapping */
	mpc8xx_flash_cycle_t* cReset;	/* pointer to reset flash cycles */
	mpc8xx_flash_cycle_t* cIdent;	/* pointer to ident flash cycles */
	mpc8xx_flash_cycle_t* cWrite;	/* pointer to write flash cycles */
	mpc8xx_flash_cycle_t* cErase;	/* pointer to erase flash cycles */
	mpc8xx_flash_cycle_t* cClear;	/* pointer to clear chip flash cycles */
	mpc8xx_flash_cycle_t* cFast;	/* pointer to fast flash cycles for register preloading */
	unsigned int* fflash;		/* pointer to fast flash ppc subroutine array */
	int nfflash;			/* number of instructions of fast flash subroutine */

	/* pointer to lowlevel communication routine */
	unsigned int (*getword)(unsigned int addr); 		
	int (*setword)(unsigned int addr, unsigned int val);
} mpc8xx_flash_bank_t;

extern unsigned int mpc8xx_flash_addr( mpc8xx_flash_bank_t* fb, unsigned int addr );
extern unsigned int mpc8xx_flash_data( mpc8xx_flash_bank_t* fb, unsigned int data );

extern mpc8xx_flash_bank_t* mpc8xx_flash_find_bank(unsigned int Addr, int bVerbose);
extern mpc8xx_flash_sector_t* mpc8xx_flash_find_sector( mpc8xx_flash_bank_t* fb, unsigned int Addr, int bVerbose);

extern int mpc8xx_flash_load_sequence( mpc8xx_flash_cycle_t* pCycle,
					unsigned int Addr, unsigned int Data, int bFirstTime);

extern int mpc8xx_flash_execute_sequence( mpc8xx_flash_cycle_t *pCycle,
					unsigned int Addr, unsigned int Data, unsigned int toggle );

extern int mpc8xx_flash_check_zeros_host(unsigned int* Buffer, unsigned int len,
				  unsigned int Destination, unsigned int* Adr);

extern int mpc8xx_flash_check_zeros(unsigned int SourceStart, unsigned int SourceEnd,
				      unsigned int Destination, unsigned int * Adr,
				      int fast_flash, unsigned int prog_address );

extern unsigned int mpc8xx_flash_fast_program(
	unsigned int SourceStart,
	unsigned int SourceEnd,
	unsigned int Destination,
	unsigned int *Adr,
	unsigned int prog_address);

extern int mpc8xx_flash_file_skip_space( FILE *f, char cChar, unsigned int* Start );
extern int mpc8xx_flash_file_find(FILE *f,char * sString,unsigned int * Start);
extern int mpc8xx_flash_file_node(FILE *f,char * sString,unsigned int * Start);

extern int mpc8xx_flash_free_bank( mpc8xx_flash_bank_t* pFB );
extern int mpc8xx_flash_parse_cycle( mpc8xx_flash_bank_t* pFB, mpc8xx_flash_cycle_t** pCycle,
					FILE* f, char** FileBuffer, char** pParse );
extern mpc8xx_flash_bank_t* mpc8xx_flash_find_empty_bank( void );





extern int mpc8xx_flash_configure(unsigned int Addr,unsigned int Num,const char* sFileName, const char* sDeviceName);
extern int mpc8xx_flash_program_file( const char* sFilename, unsigned int destaddr, int fflash, int toggle, int fquery,
				unsigned int target_buffer_address, unsigned int target_buffer_size,
				unsigned int prog_address );

extern int mpc8xx_flash_info( int toggle );
extern int mpc8xx_flash_ident(unsigned int Adr, int toggle );
extern int mpc8xx_flash_bank_reset(unsigned int Adr, int toggle );
extern int mpc8xx_flash_copy(unsigned int SourceStart, unsigned int SourceEnd,
				unsigned int Destination,  int fflash,int toggle, int fquery, unsigned int prog_address );
extern int mpc8xx_flash_erase( unsigned int addr , int toggle , int fquery );
extern int mpc8xx_flash_clear( unsigned int Addr , int toggle , int fquery );
extern int mpc8xx_flash_write_word(unsigned int Addr, unsigned int Word, int bLoadSeq, int toggle );
extern int mpc8xx_flash_write(unsigned int Addr, unsigned int Word, int toggle, int fquery );

extern int mpc8xx_flash_program_file_fast( const char* sFilename, unsigned int destaddr, int toggle, int fquery,
				unsigned int target_buffer_address, unsigned int target_buffer_size,
				unsigned int prog_address );

#ifdef __cplusplus
}
#endif

#endif /* !__MPC8XXFLASH_H__ */

