/**
 * @file	lptbdm.h
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

#ifndef __LPTBDM_H__
#define __LPTBDM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*

i86 parallel port registers and pin connections:

|Funct.| Addr.|Access|Bit 7 |Bit 6 |Bit 5 |Bit 4 |Bit 3 |Bit 2 |Bit 1 |Bit 0 |
+------+------+------+------+------+------+------+------+------+------+------+
| DATA |0x378 | R/W  |D7    |D6    |D5    |D4    |D3    |D2    |D1    |D0    |
|      |      |      |Pin 9 |Pin 8 |Pin 7 |Pin 6 |Pin 5 |Pin 4 |Pin 3 |Pin 2 |
|      |      |      |      |      |      |      |      |      |      |      |
+------+------+------+------+------+------+------+------+------+------+------+
| STAT |0x379 |  R   |~Busy | Ack  |PapErr|Select|Error |  1   |  1   |Time- |
|      |      |      |Pin 11|Pin 10|Pin 12| -In  |Pin 15|      |      | out  |
|      |      |      |      |      |      |Pin 13|      |      |      |      |
+------+------+------+------+------+------+------+------+------+------+------+
| CTRL |0x37A |  W   |  1   |   1  |DATDIR|IRQEna|~Se­  | Init |~Auto-|~Stro-|
|      |      |      |      |      | 1=I  |      | lect |Pin 16| feed |  be  |
|      |      |      |      |      | 0=O  |      |Pin 17|      |Pin 14|Pin 1 |
+------+------+------+------+------+------+------+------+------+------+------+

mapping of adapter V1 and V2:

DATA	7		6		5		4		3		2		1		0
V1		V5V		V5V		V5V		V5V		V5V		V5V		V5V		V5V
V2		V5V		V5V		V5V		V5V		V5V		TMS		DSDI	DSCK

STAT	7		6		5		4		3		2		1		0
V1		-		VDD		DSDO	-		FRZ		-		-		-
V2		VFLS1	VDD1	DSDO	VDD2	VFLS0	-		-		-

CTRL	7		6		5		4		3		2		1		0
V1		-		-		DATDIR	IRQEna	HRESET#	DSDI	SRESET#	DSCK
V2		-		-		DATDIR	IRQEna	HRESET#	-		sRESET#	TRST#

JTAG/COP (I/O direction as seen from host):

+------------------+-------------------+
|1  TDO         (I)|2  QACK         (I)|
+------------------+-------------------+
|3  TDI         (O)|4  TRST#        (O)|
+------------------+-------------------+
|5  QREQ        (I)|6  VDD          (I)|
+------------------+-------------------+
|7  TCLK        (O)|8  NC              |
+------------------+-------------------+
|9  TMS         (O)|10 NC              |
+------------------+-------------------+
|11 SRESET#     (O)|12 GND             |
+------------------+-------------------+
|13 HRESET#     (O)|14 NC              |
+------------------+-------------------+
|15 CHKSTOP     (I)|16 GND             |
+------------------+-------------------+

BDM Connector (I/O direction as seen from host):

+------------------+-------------------+
|1 VFLS0/FREEZE (I)| 2 SRESET#      (O)|
+------------------+-------------------+
|3 GND             | 4 DSCK         (O)|
+------------------+-------------------+
|5 GND             | 6 VFLS1/FREEZE (I)|
+------------------+-------------------+
|7 HRESET#      (O)| 8 DSDI         (O)|
+------------------+-------------------+
|9 VDD          (I)| 10 DSDO        (I)|
+------------------+-------------------+

(HRESET#, SRESET#, and TRST# are implemented as open collector signals)

*/

/** abstraction of parallel port layer
 */
typedef struct lptbdm_s
{
	/** pointer to function to read input lines : DSDO,VFLS0..1,VDD1..2*/
	int (*read)( );
	/** pointer to function to enable/disable powering adapter from port */
	void (*power)( unsigned char data);
	/** pointer to function to output DSCK,DSDI*/
	void (*write)( unsigned char data);
	/** pointer to function to output HRESET,SRESET,TRST*/
	void (*reset)( unsigned char data);
	/** uses Print to print the LPT port status */
	void (*status)( );

	int delay_time;
	int sleep_time;
	int relaxed_timing;

	short DATAAddr;
	short STATAddr;
	short CTRLAddr;
	unsigned char DATA;
	unsigned char DATAMask;
	unsigned char STAT;
	unsigned char CTRL;
	unsigned char CTRLMask;
	unsigned char DSCK;
	unsigned char DSDI;
	unsigned char TMS;
	unsigned char POWER;
	unsigned char VFLS0;
	unsigned char VDD2;
	unsigned char DSDO;
	unsigned char VDD1;
	unsigned char VFLS1;
	unsigned char TRST;
	unsigned char SRESET;
	unsigned char HRESET;
	unsigned char FREEZE;
	int port;
} lptbdm_t;


extern lptbdm_t lptbdm_port;

#define MPC8XX_BDM_ADAPTER_AUTO	0
#define MPC8XX_BDM_ADAPTER_V1	1
#define MPC8XX_BDM_ADAPTER_V2	2

#define MPC8XX_BDM_POWER_ON	1
#define MPC8XX_BDM_POWER_OFF	0

#define MPC8XX_BDM_LPT_0	0
#define MPC8XX_BDM_LPT_1	1
#define MPC8XX_BDM_LPT_2	2

/** Initialize the BDM adapter
 * @param lpt_port the printer port nummer
 * @param adapter_version the adapter version
 * @param power_on 1 if the adapter should be powered by the printer port
 * @return 0 on success -1 on failure
 */
extern int mpc8xx_bdm_init( 	int lpt_port,
				int adapter_version,
				int power_on );

/** Hard reset the target
 *
 */
extern void mpc8xx_bdm_reset( void );

/** Wait for the target to have power
 * @param timeout time in secondes to keep waiting
 * @return -1 on error or timeout else 0
 */
extern int mpc8xx_bdm_wait_power( int timeout );

/** Wait for the target to freeze
 * @param timeout time in secondes to keep waiting
 * @return -1 on error or timeout else 0
 */
extern int mpc8xx_bdm_wait_freeze( int timeout );

/** Wait for the target to become ready for the next BDM command
 * @param timeout time in secondes to keep waiting
 * @return -1 on error or timeout else 0
 */
extern int mpc8xx_bdm_wait_ready( int timeout );

/** Data to be clocked into the target
 *
 */
struct bdm_in_s {
	unsigned int prefix	: 2;	/**< 2 bit prefix */
	unsigned int data;		/**< 32 bit data */
};

typedef struct bdm_in_s bdm_in_t;


/** Data that get shifted out of the target
 *
 */
struct bdm_out_s {
	unsigned int ready 	: 1;	/**< target ready status */
	unsigned int status 	: 2;	/**< target status */
	unsigned int data;		/**< target data */
};

typedef struct bdm_out_s bdm_out_t;

/** Clock data or command into the target
 * @param in the data to be shifted in the target
 * @param out the data shifted out the target
 * @return -1 on error else 0
 */
extern int mpc8xx_bdm_clk_serial( const bdm_in_t* in, bdm_out_t* out );


extern int mpc8xx_set_timing( unsigned int delay_time, unsigned int sleep_time, int relaxed_timing );


#ifdef __cplusplus
}
#endif

#endif /* !__LPTBDM_H__ */
