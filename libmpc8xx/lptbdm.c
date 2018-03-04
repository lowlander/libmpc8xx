/**
 * @file	lptbdm.c
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


#include "lptbdm.h"
#include "mpc8xxbdm.h"
#include "mpc8xxmisc.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/io.h>
#include <time.h>

static int lptbdm_read( void );

static void lptbdm_power_v1( unsigned char data );
static void lptbdm_write_v1( unsigned char data );
static void lptbdm_reset_v1( unsigned char data );
static void lptbdm_status_v1( void );

static void lptbdm_power_v2( unsigned char data );
static void lptbdm_write_v2( unsigned char data );
static void lptbdm_reset_v2( unsigned char data );
static void lptbdm_status_v2( void );

lptbdm_t lptbdm_port;

/***************************************************************
* low level port routines                                      *
****************************************************************/

static short lptbdm_portadr[] =
{
	0x378,						/* lpt0 */
	0x278,						/* lpt1 */
	0x3bc,						/* lpt2 */
};

static uid_t lptbdm_euid = -1;

//#define INB(x,port)	asm volatile("inb %1, %0" : "=a" (x) : "d" (port))
//#define OUTB(x,port)	asm volatile("outb %0, %1" : : "a" (x), "d" (port))

/* in case of asm trouble (red hat 7.0?) try the code below and compile with -O option */
#include <sys/io.h>
#define INB(x,port)		x = inb(port)
#define OUTB(x,port)	outb(x,port)


/* slow down host for not overrunning target */
static inline void lptbdm_delay( int counter )
{
	unsigned char dummy;

	while (counter--)
	{
		/*
		 * since usleep and sisters are to slow (10-20msec minimum), and nop's
		 * depend on CPU speed we try I/O port reads for a delay of round about 1usec
		 */

		INB( dummy, lptbdm_port.STATAddr );
	}
}

static void lptbdm_sleep( unsigned long usec )
{
	struct timespec	ts;
	ts.tv_sec = usec / 1000000;
	ts.tv_nsec = ( usec % 1000000 ) * 1000;

	nanosleep( &ts, NULL );
}

int lptbdm_read( )
{
	INB( lptbdm_port.STAT, lptbdm_port.STATAddr );

	return lptbdm_port.STAT;
}

void lptbdm_power_v1( unsigned char power_on )
{
	if( power_on ) /* turn power on? */
	{
		lptbdm_port.POWER	= 0xff; /* V1 uses D0-D7 */
	}
	else /* turn power off */
	{
		lptbdm_port.POWER	= 0x00;
	}

	OUTB( lptbdm_port.POWER, lptbdm_port.DATAAddr);
}

void lptbdm_write_v1( unsigned char data )
{
	lptbdm_port.CTRL = 	( lptbdm_port.CTRL & lptbdm_port.CTRLMask ) |
				(data & lptbdm_port.DSDI) |
				( ~(data & lptbdm_port.DSCK) & lptbdm_port.DSCK );

	OUTB( lptbdm_port.CTRL, lptbdm_port.CTRLAddr);

   	lptbdm_delay( lptbdm_port.delay_time );
}

void lptbdm_reset_v1( unsigned char data)
{
	lptbdm_port.CTRL = 	( (( lptbdm_port.CTRL & lptbdm_port.DATAMask ) |
				(~(data & lptbdm_port.CTRLMask)) ) ) &
				lptbdm_port.CTRLMask;

	OUTB( lptbdm_port.CTRL, lptbdm_port.CTRLAddr );
}

void lptbdm_status_v1( )
{
	INB( lptbdm_port.DATA, lptbdm_port.DATAAddr );
	INB( lptbdm_port.STAT, lptbdm_port.STATAddr );
	INB( lptbdm_port.CTRL, lptbdm_port.CTRLAddr );

	mpc8xx_printf( "PortStatusV1: DATA=0x%02X STAT=0x%02X CTRL=0x%02X\n",
			lptbdm_port.DATA, lptbdm_port.STAT, lptbdm_port.CTRL);

	mpc8xx_printf( "Port active lines: '%s%s%s%s%s%s%s%s'\n",
			( lptbdm_port.DATA & lptbdm_port.POWER )   ? "POWER "  : "",
			( lptbdm_port.DATA & lptbdm_port.VFLS0 )   ? "VFLS0 "  : "",
			( lptbdm_port.DATA & lptbdm_port.VDD1 )    ? "VDD1 "   : "",
			( lptbdm_port.DATA & lptbdm_port.DSDO )    ? "DSDO "   : "",
			!( lptbdm_port.DATA & lptbdm_port.DSCK )   ? "DSCK "   : "",
			( lptbdm_port.DATA & lptbdm_port.DSDI )    ? "DSDI "   : "",
			!( lptbdm_port.DATA & lptbdm_port.HRESET ) ? "HRESET " : "",
			!( lptbdm_port.DATA & lptbdm_port.SRESET ) ? "SRESET " : "" );
}

void lptbdm_power_v2( unsigned char power_on )
{
	if( power_on ) /* turn power on? */
	{
		lptbdm_port.POWER = 0xf8; /* set mask for power lines */

		OUTB( (lptbdm_port.DATA | lptbdm_port.POWER), lptbdm_port.DATAAddr);
	}
	else /* turn power off */
	{
		lptbdm_port.POWER = 0x00; /* reset mask for power lines */
		lptbdm_port.DATA = lptbdm_port.DATA & (~0xf8);

		OUTB( lptbdm_port.DATA, lptbdm_port.DATAAddr );
	}
}

void lptbdm_write_v2( unsigned char data)
{
	lptbdm_port.DATA = data;

	OUTB( (data | lptbdm_port.POWER), lptbdm_port.DATAAddr);

   	lptbdm_delay( lptbdm_port.delay_time );

	if( mpc8xx_verbose_level( MPC8XX_VERBOSE_BDM )  )
	{
		mpc8xx_printf( "Write: ");
		lptbdm_port.status( );
	}
}

void lptbdm_reset_v2( unsigned char data)
{
	lptbdm_port.CTRL = data;

	OUTB( data, lptbdm_port.CTRLAddr );

	if ( mpc8xx_verbose_level( MPC8XX_VERBOSE_BDM ) )
	{
		mpc8xx_printf( "Reset: " );
		lptbdm_port.status( );
	}
}

void lptbdm_status_v2( )
{
	INB( lptbdm_port.DATA, lptbdm_port.DATAAddr);
	INB( lptbdm_port.STAT, lptbdm_port.STATAddr);
	INB( lptbdm_port.CTRL, lptbdm_port.CTRLAddr);

	if ( ! mpc8xx_verbose_level( MPC8XX_VERBOSE_BDM ) )
	{
		/* called from user via port command */
		mpc8xx_printf( "PortStatusV2: DATA=0x%02X STAT=0x%02X CTRL=0x%02X\n",
			lptbdm_port.DATA, lptbdm_port.STAT, lptbdm_port.CTRL );
	}

	mpc8xx_printf( "Port active lines: out: %s%s%s%s%s%s%s\tin:%s%s%s%s%s\n",
		( lptbdm_port.CTRL & lptbdm_port.HRESET ) ? "HRESET " : "",
		( lptbdm_port.CTRL & lptbdm_port.SRESET ) ? "SRESET " : "",
		( lptbdm_port.CTRL & lptbdm_port.TRST ) ? "TRST " : "",
		( lptbdm_port.DATA & lptbdm_port.DSCK ) ? "DSCK " : "",
		( lptbdm_port.DATA & lptbdm_port.DSDI ) ? "DSDI " : "",
		( lptbdm_port.DATA & lptbdm_port.TMS ) ? "TMS " : "",
		( lptbdm_port.DATA & lptbdm_port.POWER ) ? "POWER " : "",

		( lptbdm_port.STAT & lptbdm_port.VFLS0 ) ? "VFLS0 " : "",
		( lptbdm_port.STAT & lptbdm_port.VFLS1 ) ? "VFLS1 " : "",
		( lptbdm_port.STAT & lptbdm_port.VDD1 ) ? "VDD1 " : "",
		( lptbdm_port.STAT & lptbdm_port.VDD2 ) ? "VDD2 " : "",
		( lptbdm_port.STAT & lptbdm_port.DSDO ) ? "DSDO " : "" );

}

/* setup port parameters, try version 2 for default */
int mpc8xx_bdm_init( int lpt_port, int adapter_version, int power_on )
{
	int nResult;

	memset( &lptbdm_port , 0, sizeof(lptbdm_port) );
	lptbdm_port.port = -1;

	if ( (lpt_port < 0) || (lpt_port > 2) )
	{
		mpc8xx_printf( "invalid printer port %d. Use 0..2\n", lpt_port );

		return -1;
	}

	lptbdm_port.DATAAddr = lptbdm_portadr[ lpt_port ];
	lptbdm_port.STATAddr = lptbdm_port.DATAAddr + 1;
	lptbdm_port.CTRLAddr = lptbdm_port.DATAAddr + 2;

	if( lptbdm_euid == -1 ) {
		lptbdm_euid = geteuid();
	} else {
		if( seteuid( lptbdm_euid ) < 0 )
		{
			mpc8xx_printf( "unable to change process effective user ID\n");

			return -1;
		}
	}

	nResult = ioperm( lptbdm_port.DATAAddr, 3, 1 );

	seteuid( getuid() );

	if (nResult == -1)
	{
		mpc8xx_printf( "unable to get access rights for printer port %d addr 0x%3X..0x%3X\n",
			lpt_port, lptbdm_port.DATAAddr, lptbdm_port.CTRLAddr );
		return -1;
	}

	mpc8xx_printf( "got access rights for printer port %d addr 0x%3X..0x%3X\n",
		lpt_port, lptbdm_port.DATAAddr, lptbdm_port.CTRLAddr);

	lptbdm_port.port = lpt_port; /* remember port number */

	OUTB( 0x00, lptbdm_port.DATAAddr); /* reset all data lines -> disable power from port */
	OUTB( 0xC0, lptbdm_port.CTRLAddr); /* set data lines for output */

	mpc8xx_printf( "disabled power at port %d\n", lptbdm_port.port );

	/*
	 * allow changes on port lines, use a hardcoded value here
	 * since the timeouts aren't configured yet.
	 */
	lptbdm_sleep( 100000 ); /*  */

	/*
	 * Removed the buggy auto detection, default is adapter version 2.
	 * The very rare adapter 1 users have to explicitly 'set mpcbdm_adapter 1'
	 * before initializing the 'target'
	 */

	if( adapter_version == MPC8XX_BDM_ADAPTER_AUTO ) {
		/* use adpater version 2, if no other is specified */
		adapter_version = MPC8XX_BDM_ADAPTER_V2;
	}

	switch (adapter_version) /* initialize version dependencies */
	{
	case MPC8XX_BDM_ADAPTER_V1 :
		lptbdm_port.read 		= lptbdm_read;
		lptbdm_port.status 		= lptbdm_status_v1;
		lptbdm_port.power 		= lptbdm_power_v1;
		lptbdm_port.write 		= lptbdm_write_v1;
		lptbdm_port.reset 		= lptbdm_reset_v1;

		lptbdm_port.delay_time		= 0;
		lptbdm_port.sleep_time		= 100000; /* 100 msec */
		lptbdm_port.relaxed_timing	= 1;

		lptbdm_port.DSCK		= 0x01;
		lptbdm_port.DSDI		= 0x04;
		lptbdm_port.VFLS0		= 0x08;
		lptbdm_port.DSDO		= 0x20;
		lptbdm_port.VDD1		= 0x40;
		lptbdm_port.SRESET		= 0x02;
		lptbdm_port.HRESET		= 0x08;

		lptbdm_port.FREEZE		= lptbdm_port.VFLS0; /* bug of v1 adapter, only VFLS0 connected */

		lptbdm_port.CTRLMask 		= lptbdm_port.HRESET | lptbdm_port.SRESET;
		lptbdm_port.DATAMask 		= lptbdm_port.DSDI | lptbdm_port.DSCK;

		nResult = lptbdm_port.read( );
		/*
		 * plausibility check for adapter v1
		 * STAT[7] is NC on v1 -> always low (because of invert)
		 */
		if( nResult & 0x80 )
		{
			mpc8xx_printf( "adapter v1 specified, but STAT[7] != 0\n");
			lptbdm_port.status( );

			return -1;
		}

		break;

	case MPC8XX_BDM_ADAPTER_V2:
		lptbdm_port.read 		= lptbdm_read;
		lptbdm_port.power 		= lptbdm_power_v2;
		lptbdm_port.write 		= lptbdm_write_v2;
		lptbdm_port.reset 		= lptbdm_reset_v2;
		lptbdm_port.status 		= lptbdm_status_v2;

		lptbdm_port.delay_time		= 0;
		lptbdm_port.sleep_time		= 100000; /* 100 msec */
		lptbdm_port.relaxed_timing	= 0;

		lptbdm_port.DSCK		= 0x01;
		lptbdm_port.DSDI		= 0x02;
		lptbdm_port.TMS			= 0x04;
		lptbdm_port.VFLS0		= 0x08;
		lptbdm_port.VDD2		= 0x10;
		lptbdm_port.DSDO		= 0x20;
		lptbdm_port.VDD1		= 0x40;
		lptbdm_port.VFLS1		= 0x80;
		lptbdm_port.TRST		= 0x01;
		lptbdm_port.SRESET		= 0x02;
		lptbdm_port.HRESET		= 0x08;

		lptbdm_port.FREEZE		= lptbdm_port.VFLS0 | lptbdm_port.VFLS1;
		break;

	default:
		mpc8xx_printf( "invalid adapter version %d specified\n", adapter_version );
		return -1;
		break;
	}

	if( power_on )
	{
		lptbdm_port.power( 1 ); /* enable power if requested */

		mpc8xx_printf( "turned on powering from port %d\n", lptbdm_port.port );
	}

	lptbdm_port.write( 0x00 ); /* DSDI and DSCK low*/

	mpc8xx_printf( "adapter version %d initialized\n", adapter_version );

	return 0;
}

int mpc8xx_bdm_wait_power( int timeout )
{
	int nResult, nLast;
	time_t end_time = time( NULL ) + timeout;

	nResult = lptbdm_port.read( );

	while ( !(nResult & lptbdm_port.VDD1) ) /* until target powered up */
	{
		nLast = nResult;
		nResult = lptbdm_port.read( );
		if (nResult != nLast)
		{
			lptbdm_port.status( );
		}

		lptbdm_sleep( lptbdm_port.sleep_time );

		if( time( NULL ) > end_time )
			return -1;
	}

	return 0;
}

int mpc8xx_bdm_has_power()
{
	int nResult = lptbdm_port.read();
	
	if( nResult & lptbdm_port.VDD1 )
		return 1;
	else
		return 0;
}


int mpc8xx_bdm_wait_freeze( int timeout )
{
	int nResult;
	
	time_t end_time = time( NULL ) + timeout;

	if( mpc8xx_verbose_level( MPC8XX_VERBOSE_TAR )  )
	{
		mpc8xx_printf( "mpc8xx_bdm_wait_freeze:\n" );
	}

	while(  time( NULL ) <=  end_time )
	{
		/* loop until we have a stable signal */
		do {
			nResult = lptbdm_port.read( );
		} while( nResult != lptbdm_port.read() );
		
		if ( !( nResult & lptbdm_port.VDD1 ) )
		{
			/* no power */
			return -2;
		}
		
		if( ( nResult & lptbdm_port.FREEZE ) == lptbdm_port.FREEZE )
		{
			/* ok went in freeze */
			return 0;
		}
		
		lptbdm_sleep( lptbdm_port.sleep_time );
	}

	/* timeout error */	
	return -1;		
}


/* keep DSDI low and toggle DSCK until DSDO goes low */

int mpc8xx_bdm_wait_ready( int timeout )
{
	time_t end_time = time( NULL ) + timeout;

	if( mpc8xx_verbose_level( MPC8XX_VERBOSE_SER | MPC8XX_VERBOSE_BDM ) )
	{
		mpc8xx_printf("ser_wait_ready: start\n");
	}

	while( lptbdm_port.read( ) & lptbdm_port.DSDO )
	{
		if( lptbdm_port.relaxed_timing )
		{
			lptbdm_port.write( 0x00 ); /* relaxed BDM port timing */
		}
		lptbdm_port.write( lptbdm_port.DSCK );
		lptbdm_port.write( 0x00 );

		if( time( NULL ) > end_time )
			return -1;

		lptbdm_delay( 1 ); /* assure host independent timing */

	}

	return 0;
}

void mpc8xx_bdm_reset( )
{
	/*
	 * set DSCK and /DSDI, implicite wait in Write
	 * assert /HRESET and /SRESET, hold DSCK and /DSDI
	 * Debug Mode enable
	 */
	lptbdm_port.write( lptbdm_port.DSCK );

	/* make sure the DSCK is set 3 clocks before SRESET */
	lptbdm_sleep( lptbdm_port.sleep_time );

	lptbdm_port.reset( lptbdm_port.HRESET | lptbdm_port.SRESET );

	/* hold reset for some while, since here occured some problems with fast CPUs*/
	lptbdm_sleep( lptbdm_port.sleep_time );

	 /* release HRESET and SRESET  with DSCK, /DSDI: immediately debug, asynchr. clocked*/
	lptbdm_port.reset( 0x00 );

	/* make sure there is enough time after reset */
	lptbdm_sleep( lptbdm_port.sleep_time );
}


static int lptbdm_clk_bit( int b )
{
	int res = 0;
	int bit;

	if( lptbdm_port.read( ) & lptbdm_port.DSDO )
		res = 1;

	bit = b ? lptbdm_port.DSDI : 0;

	if( lptbdm_port.relaxed_timing )
	{
		lptbdm_port.write( bit ); /* relaxed BDM port timing */
	}

	lptbdm_port.write( bit | lptbdm_port.DSCK ); /* clock DSCK High after setting data bit on DSDI*/
	lptbdm_port.write( bit); /*clock DSCK low for reading DSDO next */

	return res;
}

int mpc8xx_bdm_clk_serial( const bdm_in_t* in, bdm_out_t* out )
{
	int res = 0;
	int n;
	int reslen = 0;
	int len, nResult;
	unsigned int mask,word;

	/* clear out */
	memset( out, 0 , sizeof(bdm_out_t) );


	nResult = lptbdm_port.read( );
	
	if ( !( nResult & lptbdm_port.VDD1 ) )
	{
		if( mpc8xx_bdm_wait_power( 10 ) < 0 )
		{
			/* no power */
			return -2;
		}
		
		/* reread to get the freeze state 
		 * we put that here instead of outside the if
		 * to have a optimal path when everything goes ok 
		 */
		nResult = lptbdm_port.read( );	
	}

	if( ( nResult & lptbdm_port.FREEZE ) == lptbdm_port.FREEZE )
	{
		if( mpc8xx_bdm_wait_ready( 2 ) < 0 )
		{
			/* target is not ready in 2 secs */
			return -1;
		}
	} 

	if( in->prefix & 2  )
		len = 7;
	else
		len = 32;

	/* clock start bit get ready bit */
	out->ready = lptbdm_clk_bit( 1 );

	/* clock mode get first status bit */
	out->status = lptbdm_clk_bit( in->prefix & 2 );

	out->status <<= 1;
	/* clock control bit get second status bit */
	out->status |= lptbdm_clk_bit( in->prefix & 1 );


	switch( out->status ){
		case MPC8XX_BDM_STAT_CORE_DATA:
			reslen = 32;
			break;

		/* debug command while target not in debug mode */
		case MPC8XX_BDM_STAT_SEQ_ERROR:
			res = -3;
			reslen = 7;
			break;

		/* interrupt occurred, have to read ICR to clear */
		case MPC8XX_BDM_STAT_CORE_INTERRUPT:
			res = -4;
			reslen = 7;
			break;

		case MPC8XX_BDM_STAT_NULL:
			reslen = 7;
			break;

		default:
			return -5;
			break;
	}


	mask = 1 << ( len - 1 );

	word = in->data;
	out->data = 0;
	for(n = 0; n < len; n++ )
	{
		out->data <<= 1;
		out->data |= lptbdm_clk_bit( word & mask );
		word <<= 1;
	}

	for(;n < reslen; n++ ) /* clock rest of data in with writing zeros*/
	{
		out->data <<= 1;
		out->data |= lptbdm_clk_bit( 0 );
	}

	lptbdm_port.write( 0x00); /* reset data bit on DSDI */	

	return res;
}


int mpc8xx_set_timing( unsigned int delay_time, unsigned int sleep_time, int relaxed_timing )
{
	lptbdm_port.delay_time = delay_time;
	lptbdm_port.sleep_time = sleep_time;
	lptbdm_port.relaxed_timing = relaxed_timing;

	return 0;
}

