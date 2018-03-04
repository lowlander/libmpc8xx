/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Wed Mar  5 15:51:02 CET 2003
    copyright            : (C) 2003 by Erwin Rol
    email                : erwin@muffin.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "lptbdm.h"
#include "mpc8xxbdm.h"
#include "mpc8xxflash.h"
#include "mpc8xxmisc.h"
#include "mpc8xxspr.h"
#include "mpc8xxtarget.h"
#include "mpc8xxmem.h"

#include <stdio.h>
#include <stdlib.h>

static void print( const char* s ){
	printf("%s",s);
}

int main(int argc, char *argv[])
{
	int lpt_port = 0;
	int adapter_version = 1;
	int power_on = 1;

	if( argc > 1 )
		lpt_port = atoi( argv[1] );

	if( argc > 2 )
		adapter_version = atoi( argv[2] );

	if( argc > 3 )
		power_on = atoi( argv[3] );

	mpc8xx_set_print_function( print );
	mpc8xx_bdm_init( lpt_port, adapter_version, power_on );

	mpc8xx_bdm_reset();

	if( mpc8xx_bdm_wait_freeze( 10 ) < 0){
		printf("Traget didn't enter FREEZE state\n");

		return -1;
	}

	mpc8xx_print_cpu_info();

	return 0;
}
