/**
 * @file	mpc8xxspr.c
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

#include "mpc8xxspr.h"
#include "lptbdm.h"
#include "mpc8xxbdm.h"
#include "mpc8xxmisc.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

mpc8xx_spr_name_t mpc8xx_spr_names[] =
{
{"XER",		MPC8XX_SPR_XER,		"SO|OV|CA||25BCNT",0,"UM141"},
{"LR",		MPC8XX_SPR_LR,		0,"Link Register",0},
{"CTR",		MPC8XX_SPR_CTR,		0,"Counter Register",0},
{"DSISR",	MPC8XX_SPR_DSISR,	0,0,0},
{"DAR",		MPC8XX_SPR_DAR,		0,0,0},
{"DEC",		MPC8XX_SPR_DEC,		0,"Decrementer Register","UM297"},
{"SRR0",	MPC8XX_SPR_SRR0,	0,0,0 },
{"SRR1",	MPC8XX_SPR_SRR1,	"||16EE|PR|FP|ME||SE|BE||26IR|DR||30RI|LE","Machine Status Save/Restore Register1","UM145"},
{"EIE",		MPC8XX_SPR_EIE,		0,0,0},
{"EID",		MPC8XX_SPR_EID,		0,0,0},
{"NRI",		MPC8XX_SPR_NRI,		0,0,0},
{"CMPA",	MPC8XX_SPR_CMPA,	0,"Comparator A Value Register (instr)","UM987"},
{"CMPB",	MPC8XX_SPR_CMPB,	0,"Comparator B Value Register (instr)","UM987"},
{"CMPC",	MPC8XX_SPR_CMPC,	0,"Comparator C Value Register (instr)","UM987"},
{"CMPD",	MPC8XX_SPR_CMPD,	0,"Comparator D Value Register (instr)","UM987"},
{"ICR",		MPC8XX_SPR_ICR,		"|RST|CHSTP|MCI||6EXTI|ALI|PRI|FPUVI|DECI||13SYSI|TR||17SEI|ITLBMS|DTBLMS|ITLBER|DTLBER||28LBRK|IBRK|EBRK|DPI","Interrupt Cause Register","UM994"},
{"DER",		MPC8XX_SPR_DER,		"|RSTE|CHSTPE|MCIE||6EXTIE|ALIE|PRIE|FPUVIE|DECIE||13SYSIE|TRE||17SEIE|ITLBMSE|DTLBMSE|ITLBERE|DTBLERE||28LBRKE|IBRKE|EBRKE|DPIE","Debug Enable Register","UM996"},
{"COUNTA",	MPC8XX_SPR_COUNTA,	"CNTCV|16|30CNTC","Breakpoint Counter Value and Control A Register","UM994"},
{"COUNTB",	MPC8XX_SPR_COUNTB,	"CNTCV|16|30CNTC","Breakpoint Counter Value and Control B Register","UM994"},
{"CMPE",	MPC8XX_SPR_CMPE,	0,"Comparator E Value Register (addr)","UM987"},
{"CMPF",	MPC8XX_SPR_CMPF,	0,"Comparator F Value Register (addr)","UM987"},
{"CMPG",	MPC8XX_SPR_CMPG,	0,"Comparator G Value Register (data)","UM987"},
{"CMPH",	MPC8XX_SPR_CMPH,	0,"Comparator H Value Register (data)","UM987"},
{"LCTRL1",	MPC8XX_SPR_LCTRL1,	"CTE|3CTF|6CTG|9CTH|12CRWE|14CRWF|16CSG|18CSH|20SUSG|SUSH|CGBMSK|26CHBMSK|30","Load/Store Support Comparators Control Register","UM990"},
{"LCTRL2",	MPC8XX_SPR_LCTRL2,	"LW0EN|LW0IA|3LW0IADC|LW0LA|6LW0LADC|LW0LD|9LW0LDDC|LW1EN|LW1IA|13LW1IADC|LW1LA|16LW1LADC|LW1LD|19LW1LDDC|BRKNOMSK||28DLW0EN|DLW1EN|SLW0EN|SLW1EN","Load/Store Support AND-OR Control Register","UM991"},
{"ICTRL",	MPC8XX_SPR_ICTRL,	"CTA|3CTB|6CTC|9CTD|12IW0|14IW1|16IW2|18IW3|20SIW0EN|SIW1EN|SIW2EN|SIW3EN|DIW0EN|DIW1EN|DIW2EN|DIW3EN|IFM|ISCT_SER","Instruction Support Control Register","UM989"},
{"BAR",		MPC8XX_SPR_BAR,		0,0,0 },
{"TBLR",	MPC8XX_SPR_TBLR,	0,0,0 },
{"TBUR",	MPC8XX_SPR_TBUR,	0,0,0 },
{"SPRG0",	MPC8XX_SPR_SPRG0,	0,0,0 },
{"SPRG1",	MPC8XX_SPR_SPRG1, 	0,0,0 },
{"SPRG2",	MPC8XX_SPR_SPRG2,	0,0,0 },
{"SPRG3",	MPC8XX_SPR_SPRG3,	0,0,0 },
{"TBLW",	MPC8XX_SPR_TBLW, 	0,0,0 },
{"TBUW",	MPC8XX_SPR_TBUW, 	0,0,0 },
{"PVR",		MPC8XX_SPR_PVR,		0,0,0 },
{"IC_CST",	MPC8XX_SPR_IC_CST,	"IEN||4CMD|7|10CCER1|CCER2|CCER3|","Instruction Cache Control and Status Register","UM200"},
{"IC_ADR",	MPC8XX_SPR_IC_ADR,	"|18Data|Way1||Set|28Word|30","Instruction Cache Address Register","UM202"},
{"IC_DAT",	MPC8XX_SPR_IC_DAT,	"Tag|21|Valid|Locked|LRU|","Instruction Cache Data Port Register","UM202"},
{"DC_CST",	MPC8XX_SPR_DC_CST,	"DEN|DFWT|LES||CMD|8|10CCER1|CCER2|CCER3|","Data Cache Control an Status Register","UM205"},
{"DC_ADR",	MPC8XX_SPR_DC_ADR,	"|18Cbb|Way1||Set|28","Data Cache Address Register","UM208"},
{"DC_DAT",	MPC8XX_SPR_DC_DAT,	"Tag|21|Valid|Locked|LRU|Modified|","Data Cache Data Port Register","UM209"},
{"DPDR",	MPC8XX_SPR_DPDR,	0,0,0 },
{"DPIR",	MPC8XX_SPR_DPIR,	0,0,0 },
{"IMMR",	MPC8XX_SPR_IMMR,	"ISB|16PARTNUM|24MASKNUM","Internal Memory Map Register","UM279"},
{"MI_CTR",	MPC8XX_SPR_MI_CTR,	"GPM|PPM|CIDEF||RSV4I||PPCS||19ITLB_INDX|24","IMMU Control Register","UM242"},
{"MI_AP",	MPC8XX_SPR_MI_AP,	"GP0|2GP1|4GP2|6GP3|8GP4|10GP5|12GP6|14GP7|16GP8|18GP9|20GP10|22GP11|24GP12|26GP13|28GP14|30GP15","IMMU Access Protection Register","UM250"},
{"MI_EPN",	MPC8XX_SPR_MI_EPN,	"EPN|20|22EV||28ASID", "IMMU Effective Page Number Register","UM244"},
{"MI_TWC",	MPC8XX_SPR_MI_TWC,	"|23APG|G|PS|30|V","IMMU Tablewalk Control Register","UM244"},
{"MI_RPN",	MPC8XX_SPR_MI_RPN,	"RPN|20|PP|28SPS|SH|CI|V","IMMU Real Page Number Register","UM246"},
{"MD_CTR",	MPC8XX_SPR_MD_CTR,	"GPM|PPM|CIDEF|WTDEF|RSV4D|TWAM|PPCS||19DTLB_INDX|24","DMMU Control Register","UM243"},
{"M_CASID",	MPC8XX_SPR_M_CASID,	"|28CASID","MMU Current Address Space ID Register","UM249"},
{"MD_AP",	MPC8XX_SPR_MD_AP,	"GP0|2GP1|4GP2|6GP3|8GP4|10GP5|12GP6|14GP7|16GP8|18GP9|20GP10|22GP11|24GP12|26GP13|28GP14|30GP15","DMMU Access Protection Register","UM250"},
{"MD_EPN",	MPC8XX_SPR_MD_EPN,	"EPN|20|22EV||28ASID", "IMMU Effective Page Number Register","UM244"},
{"M_TWB",	MPC8XX_SPR_M_TWB,	"L1TB|20L1INDX|30","MMU Tablewalk Base Register","UM249"},
{"MD_TWC",	MPC8XX_SPR_MD_TWC,	"L2TB|20|23APG|27G|PS|30WT|V","DMMU Tablewalk Control Register","UM245"},
{"MD_RPN",	MPC8XX_SPR_MD_RPN,	"RPN|20PP|28SPS|SH|CI|V","DMMU Real Page Number Register","UM248"},
{"M_TW",	MPC8XX_SPR_M_TW,	0,"MMU Tablewalk Special Register","UM250"},
{"MI_CAM",	MPC8XX_SPR_MI_CAM,	"EPN|20PS|23ASID|27SH|SPV","IMMU CAM Entry Read Register","UM251"},
{"MI_RAM0",	MPC8XX_SPR_MI_RAM0,	"RPN|20PS_B|23CI|APG|28SFP","IMMU RAM Entry Read Register 0","UM252"},
{"MI_RAM1",	MPC8XX_SPR_MI_RAM1,	"|26UFP|30PV|G","IMMU RAM Entry Read Register 1","UM253"},
{"MD_CAM",	MPC8XX_SPR_MD_CAM,	"EPN|20SPVF|24PS|27SH|ASID","DMMU CAM Entry Read Register","UM254"},
{"MD_RAM0",	MPC8XX_SPR_MD_RAM0,	"RPN|20PS|23APGI|27G|WT|CI|","DMMU RAM Entry Read Register 0","UM255"},
{"MD_RAM1",	MPC8XX_SPR_MD_RAM1,	"|16RES|C|EVF|SA|23SAT|URP0|UWP0|URP1|UWP1|URP2|UWP2|URP3|UWP3","DMMU RAM Entry Read Register 1","UM256"},
{"SIUMCR",	MPC8XX_SPRI_SIUMCR,	"EARB|EARP|4|8DSHW|DBGC|11DBPC|13|FRC|DLK|OPAR|PNCS|DPC|MPRE|MLRC|22AEME|SEME|BSC|GB5E|B2DD|B3DD|","SIU Module Configuration Register","UM280"},
{"SYPCR",	MPC8XX_SPRI_SYPCR,	"SWTC|16BMT|24BME||28SWF|SWE|SWRI|SWP","System Protection Control Register","UM283"},
{"SWSR",	MPC8XX_SPRI_SWSR,	0,"Software Service Register","UM296"}, /*16bit556CAA39*/
{"SIPEND",	MPC8XX_SPRI_SIPEND,	"IRQ0|LVL0|IRQ1|LVL1|IRQ2|LVL2|IRQ3|LVL3|IRQ4|LVL4|IRQ5|LVL5|IRQ6|LVL6|IRQ7|LVL7|","SIU Interrupt Pending Register","UM291"},
{"SIMASK",	MPC8XX_SPRI_SIMASK,	"IRM0|LVM0|IRM1|LVM1|IRM2|LVM2|IRM3|LVM3|IRM4|LVM4|IRM5|LVM5|IRM6|LVM6|IRM7|LVM7|","SIU Interrupt Mask Register","UM291"},
{"SIEL",	MPC8XX_SPRI_SIEL,	"ED0|WM0|ED1|WM1|ED2|WM2|ED3|WM3|ED4|WM4|ED5|WM5|ED6|WM6|ED7|WM7|","SIU Interrupt Edge/Level Register","UM292"},
{"SIVEC",	MPC8XX_SPRI_SIVEC,	"INTC|8","SIU Interrupt Vector Register","UM293"},
{"TESR",	MPC8XX_SPRI_TESR,	"|18IEXT|ITMT|IPB0|IPB1|IPB2|IPB3||26DEXT|DTMT|DPB0|DPB1|DPB2|DPB3","Transfer Error Status Register","UM284"},
{"SDCR",	MPC8XX_SPRI_SDCR,	"|17FRZ|19|25AM||30RAID","SDMA Configuration Register","UM570"},
{"PBR0",	MPC8XX_SPRI_PBR(0),	0,"PCMCIA Base Register 0","UM516"},
{"POR0",	MPC8XX_SPRI_POR(0),	"BSIZE|5|12PSHT|16PSST|20PSL|25PPS|PRS|29PSLOT|WP|PV","PCMCIA Option Register 0","UM517"},
{"PBR1",	MPC8XX_SPRI_PBR(1),	0,"PCMCIA Base Register 1","UM516"},
{"POR1",	MPC8XX_SPRI_POR(1),	"BSIZE|5|12PSHT|16PSST|20PSL|25PPS|PRS|29PSLOT|WP|PV","PCMCIA Option Register 1","UM517"},
{"PBR2",	MPC8XX_SPRI_PBR(2),	0,"PCMCIA Base Register 2","UM516"},
{"POR2",	MPC8XX_SPRI_POR(2),	"BSIZE|5|12PSHT|16PSST|20PSL|25PPS|PRS|29PSLOT|WP|PV","PCMCIA Option Register 2","UM517"},
{"PBR3",	MPC8XX_SPRI_PBR(3),	0,"PCMCIA Base Register 3","UM516"},
{"POR3",	MPC8XX_SPRI_POR(3),	"BSIZE|5|12PSHT|16PSST|20PSL|25PPS|PRS|29PSLOT|WP|PV","PCMCIA Option Register 3","UM517"},
{"PBR4",	MPC8XX_SPRI_PBR(4),	0,"PCMCIA Base Register 4","UM516"},
{"POR4",	MPC8XX_SPRI_POR(4),	"BSIZE|5|12PSHT|16PSST|20PSL|25PPS|PRS|29PSLOT|WP|PV","PCMCIA Option Register 4","UM517"},
{"PBR5",	MPC8XX_SPRI_PBR(5),	0,"PCMCIA Base Register 5","UM516"},
{"POR5",	MPC8XX_SPRI_POR(5),	"BSIZE|5|12PSHT|16PSST|20PSL|25PPS|PRS|29PSLOT|WP|PV","PCMCIA Option Register 5","UM517"},
{"PBR6",	MPC8XX_SPRI_PBR(6),	0,"PCMCIA Base Register 6","UM516"},
{"POR6",	MPC8XX_SPRI_POR(6),	"BSIZE|5|12PSHT|16PSST|20PSL|25PPS|PRS|29PSLOT|WP|PV","PCMCIA Option Register 6","UM517"},
{"PBR7",	MPC8XX_SPRI_PBR(7),	0,"PCMCIA Base Register 7","UM516"},
{"POR7",	MPC8XX_SPRI_POR(7),	"BSIZE|5|12PSHT|16PSST|20PSL|25PPS|PRS|29PSLOT|WP|PV","PCMCIA Option Register 7","UM517"},
{"PGCRA",	MPC8XX_SPRI_PGCRA,	"CxIREQLVL|8CxSCHLVL|16CxDREQ|18|24CxOE|CxReset|","PCMCIA Interface General Control Register","UM516"},
{"PGCRB",	MPC8XX_SPRI_PGCRB,	"CxIREQLVL|8CxSCHLVL|16CxDREQ|18|24CxOE|CxReset|","PCMCIA Interface General Control Register","UM516"},
{"PSCR",	MPC8XX_SPRI_PSCR,	"CAVS1_C|CAVS2_C|CAWP_C|CACD2_C|CACD1_C|CABVD2_C|CABVD1_C||CARDY_L|CARDY_H|CARDY_R|CARDY_F||16CBVS1_C|CBVS2_C|CBWP_C|CBCD2_C|CBCD1_C|CBBVD2_C|CBBVD1_C||CBRDY_L|CBRDY_H|CBRDY_R|CBRDY_F|","PCMCIA Interface Status Changed Register","UM513"},
{"PIPR",	MPC8XX_SPRI_PIPR,	"CAVS1|CAVS2|CAWP|CACD2|CACD1|CABVD2|CABVD1|CARDY||16CBVS1|CBVS2|CBWP|CBCD2|CBCD1|CBBVD2|CBBVD1|CBRDY|","PCMCIA Interface Input Pins Register","UM512"},
{"PER",		MPC8XX_SPRI_PER,	"CA_EVS1_C|CA_EVS2_C|CA_EWP_C|CA_ECD2_C|CA_ECD1_C|CA_EBVD2_C|CA_EBVD1_C||CA_ERDY_L|CA_ERDY_H|CA_ERDY_R|CA_ERDY_F||16CB_EVS1_C|CB_EVS2_C|CB_EWP_C|CB_ECD2_C|CB_ECD1_C|CB_EBVD2_C|CB_EBVD1_C||CB_ERDY_L|CB_ERDY_H|CB_ERDY_R|CB_ERDY_F|","PCMCIA Interface Enable Register","UM514"},
{"BR0",		MPC8XX_SPRI_BR(0),	"BA|17AT|20PS|22PARE|WP|MS|26|31V","Base Register 0","UM435"},
{"OR0", 	MPC8XX_SPRI_OR(0),	"AM|17ATM|20CSNT/SAM|ACS/G5LA,G5LS|23BIH|SCY|28SETA|TRLX|EHTR|","Option Register 0","UM437"},
{"BR1",		MPC8XX_SPRI_BR(1),	"BA|17AT|20PS|22PARE|WP|MS|26|31V","Base Register 1","UM435"},
{"OR1", 	MPC8XX_SPRI_OR(1),	"AM|17ATM|20CSNT/SAM|ACS/G5LA,G5LS|23BIH|SCY|28SETA|TRLX|EHTR|","Option Register 1","UM437"},
{"BR2",		MPC8XX_SPRI_BR(2),	"BA|17AT|20PS|22PARE|WP|MS|26|31V","Base Register 2","UM435"},
{"OR2", 	MPC8XX_SPRI_OR(2),	"AM|17ATM|20CSNT/SAM|ACS/G5LA,G5LS|23BIH|SCY|28SETA|TRLX|EHTR|","Option Register 2","UM437"},
{"BR3",		MPC8XX_SPRI_BR(3),	"BA|17AT|20PS|22PARE|WP|MS|26|31V","Base Register 3","UM435"},
{"OR3", 	MPC8XX_SPRI_OR(3),	"AM|17ATM|20CSNT/SAM|ACS/G5LA,G5LS|23BIH|SCY|28SETA|TRLX|EHTR|","Option Register 3","UM437"},
{"BR4",		MPC8XX_SPRI_BR(4),	"BA|17AT|20PS|22PARE|WP|MS|26|31V","Base Register 4","UM435"},
{"OR4", 	MPC8XX_SPRI_OR(4),	"AM|17ATM|20CSNT/SAM|ACS/G5LA,G5LS|23BIH|SCY|28SETA|TRLX|EHTR|","Option Register 4","UM437"},
{"BR5",		MPC8XX_SPRI_BR(5),	"BA|17AT|20PS|22PARE|WP|MS|26|31V","Base Register 5","UM435"},
{"OR5", 	MPC8XX_SPRI_OR(5),	"AM|17ATM|20CSNT/SAM|ACS/G5LA,G5LS|23BIH|SCY|28SETA|TRLX|EHTR|","Option Register 5","UM437"},
{"BR6",		MPC8XX_SPRI_BR(6),	"BA|17AT|20PS|22PARE|WP|MS|26|31V","Base Register 6","UM435"},
{"OR6", 	MPC8XX_SPRI_OR(6),	"AM|17ATM|20CSNT/SAM|ACS/G5LA,G5LS|23BIH|SCY|28SETA|TRLX|EHTR|","Option Register 6","UM437"},
{"BR7",		MPC8XX_SPRI_BR(7),	"BA|17AT|20PS|22PARE|WP|MS|26|31V","Base Register 7","UM435"},
{"OR7", 	MPC8XX_SPRI_OR(7),	"AM|17ATM|20CSNT/SAM|ACS/G5LA,G5LS|23BIH|SCY|28SETA|TRLX|EHTR|","Option Register 7","UM437"},
{"MAR",		MPC8XX_SPRI_MAR,	0, "Memory Address Register","UM443"},
{"MCR",		MPC8XX_SPRI_MCR,	"OP|2|8UM||16MB|19|MCLF|24|26MAD","Memory Command Register","UM441"},
{"MAMR",	MPC8XX_SPRI_MAMR,	"PTA|8PTAE|AMA|12|DSA|15|G0CLA|19GPLA4DIS|RLFA|24WLFA|28TLFA","Machine A Mode Register","UM440"},
{"MBMR",	MPC8XX_SPRI_MBMR,	"PTB|8PTBE|AMB|12|DSB|15|G0CLB|19GPLB4DIS|RLFB|24WLFB|28TLFB","Machine B Mode Register","UM440"},
{"MSTAT",	MPC8XX_SPRI_MSTAT,	"PER0|PER1|PER2|PER3|PER4|PER5|PER6|PER7|WPER||16PTP|24","Memory Status Register & Periodic Timer","UM439"}, /*contains both MSTAT,MPTPR*/
{"MDR",		MPC8XX_SPRI_MDR,	"CST4|CST1|CST2|CST3|BST4|BST1|BST2|BST3|G0L|10G0H|12G1T4|G1T3|G2T4|G2T3|G3T4|G3T3|G4T4/DLT3|G4T3/WAEN|G5T4|G5T3||24LOOP|EXEN|AMX|28NA|UTA|TODT|LAST", "Memory Data Register", "UM442"},
{"TBSCR",	MPC8XX_SPRI_TBSCR,	"TBIRQ|8REFA|REFB||12REFAE|REFBE|TBF|TBE|","Timebase Status and Control Register","UM300"}, /*16bit!*/
{"TBREFA",	MPC8XX_SPRI_TBREFA,	0,"Timebase Reference Register A","UM299"},
{"TBREFB",	MPC8XX_SPRI_TBREFB,	0,"Timebase Reference Register B","UM299"},
{"RTCSC",	MPC8XX_SPRI_RTCSC,	"RTCIRQ|8SEC|ALR||teK|SIE|ALE|RTF|RTE|","Real-Time Clock Status and Control Register","UM301"}, /*16bit!, teK = 38K*/
{"RTC",		MPC8XX_SPRI_RTC,	0,"Real-Time Clock Register","UM302"},
{"RTSEC",	MPC8XX_SPRI_RTSEC,	"COUNTER|14","Real-Time Clock Alarm Seconds Register","UM304"},
{"RTCAL",	MPC8XX_SPRI_RTCAL,	0,"Real-Time Clock Alarm Register","UM303"},
{"PISCR",	MPC8XX_SPRI_PISCR,	"PIRQ|8PS||13PIE|PITF|PTE|","Periodic Interrupt Status and Control Register","UM305"}, /*16bit!*/
{"PITC",	MPC8XX_SPRI_PITC,	"PITC|16","PIT Count Register","UM306"},
{"PITR",	MPC8XX_SPRI_PITR,	"PIT|16","PIT Register","UM307"},
{"SCCR",	MPC8XX_SPRI_SCCR,	"|COM|3|6TBS|RTDIV|RTSEL|CRQEN|PRQEN||13EBDF|15|17DFSYNC|19DFBRG|21DFNL|24DFNH|27","System Clock and Reset Control Register","UM421"},
{"PLPRCR",	MPC8XX_SPRI_PLPRCR,	"MF|12|16SPLSS|TEXPS||TMIST||CSRC|LPM|24CSR|LOLRE|FIOPD|","PLL, Low-Power and Reset Control Register","UM423"},
{"RSR",		MPC8XX_SPRI_RSR,	"EHRS|ESRS|LLRS|SWRS|CSRS|DBHRS|DBSRS|JTRS|","Reset Status Register","UM313"},
{"TBSCRK",	MPC8XX_SPRI_TBSCRK,	0,"Timebase Status and Control Register Key","UM285"},
{"CIVR",	MPC8XX_SPRI_CIVR,	"VN|5|15IACK|","CPM Interrupt Vector Register 16 bit","UM912"}, /*16bit!*/
{"CICR",	MPC8XX_SPRI_CICR,	"|8SCdP|10SCcP|12SCbP|14SCaP|16IRL|19HP|24IEN||31SPS","CPM Interrupt Configuration Register","UM909"},
{"CIPR",	MPC8XX_SPRI_CIPR,	"PC15|SCC1|SCC2|SCC3|SCC4|PC14|TIMER1|PC13|PC12|SDMA|IDMA1|IDMA2||TIMER2|RTT|I2C|PC11|PC10||TIMER3|PC9|PC8|PC7||TIMER4|PC6|SPI|SMC1|SMC2|PC5|PC4|","CPM Interrupt Pending Register","UM910"},
{"CIMR",	MPC8XX_SPRI_CIMR,	"PC15|SCC1|SCC2|SCC3|SCC4|PC14|TIMER1|PC13|PC12|SDMA|IDMA1|IDMA2||TIMER2|RTT|I2C|PC11|PC10||TIMER3|PC9|PC8|PC7||TIMER4|PC6|SPI|SMC1|SMC2|PC5|PC4|","CPM Interrupt Mask Register","UM910"},
{"CISR",	MPC8XX_SPRI_CISR,	"PC15|SCC1|SCC2|SCC3|SCC4|PC14|TIMER1|PC13|PC12|SDMA|IDMA1|IDMA2||TIMER2|RTT|I2C|PC11|PC10||TIMER3|PC9|PC8|PC7||TIMER4|PC6|SPI|SMC1|SMC2|PC5|PC4|","CPM Interrupt In-Service Register","UM910"},
{"PADIR",	MPC8XX_SPRI_PADIR,	"DIR|16PAR","Port A Data Direction Register 16 bit",0}, /*16bit! PAPAR*/
{"PAODR",	MPC8XX_SPRI_PAODR,	"ODR|16DAT","Port A Open-Drain Register 16 bit",0}, /*16bit! PADAT*/
{"PCDIR",	MPC8XX_SPRI_PCDIR,	"DIR|16PAR","Port C Data Direction Register 16 bit",0}, /*16bit! PCPAR*/
{"PCSO",	MPC8XX_SPRI_PCSO,	"SO|16DAT","Port C Special Option Register 16 bit",0}, /*16bit! PCDAT*/
{"PCINT",	MPC8XX_SPRI_PCINT,	"INT|16","Port C Interrupt Control Register 16 bit",0}, /*16bit!*/
{"PDDIR",	MPC8XX_SPRI_PDDIR,	"DIR|16PAR","Port C Data Direction Register 16 bit",0}, /*16bit! PDPAR*/
{"PDDAT",	MPC8XX_SPRI_PDDAT,	"ODR|16","Port C Data Register 16 bit",0}, /*16bit!*/
{"PBDIR",	MPC8XX_SPRI_PBDIR,	0,"Port B Data Direction Register","UM893"},
{"PBPAR",	MPC8XX_SPRI_PBPAR,	0,"Port B Pin Assignment Register","UM893"},
{"PBODR",	MPC8XX_SPRI_PBODR,	0,"Port B Open-Drain Register","UM891"},
{"PBDAT",	MPC8XX_SPRI_PBDAT,	0,"Port B Data Register","UM892"},
{"SIMODE",	MPC8XX_SPRI_SIMODE,	"SMC2|SMC2CS|4SDMb|6RFSDb|8DSCb|CRTb|STZb|CEb|FEb|GMb|TFSDb|16SMC1|SMC1CS|20SDMa|22RFSDa|24DSCa|CRTa|STZa|CEa|FEa|GMa|TFSDa","SI mode register","UM606"},
{"SIGMR",	MPC8XX_SPRI_SIGMR,	"|4ENb|ENa|RDM|8|16CRORa|CROTa|CRORb|CROTb||24CSRRa|CSRTa|CSRRb|CSRTb|","SI global mode register 8 bit","UM605"}, /*8bit! res, SISTR, SICMR*/
{"SICR",	MPC8XX_SPRI_SICR,	"GR4|SC4|R4CS|5T4CS|8GR3|SC3|R3CS|13T3CS|16GR2|SC2|R2CS|21T2CS|24GR1|SC1|R1CS|28T1CS","SI clock route register","UM612"},
{"SIRP",	MPC8XX_SPRI_SIRP,	"|2VTb|TbPTR|8|10VTa|TaPTR|16|18VRb|RbPTR|24|26VRa|RaPTR","SI RAM pointer register","UM614"},
{"MSR",		MPC8XX_SPR_MSR,		"|13POW||ILE|EE|PR|FP|ME||SE|BE||25IP|IR|DR||30RI|LE","Machine State Register","UM145"},
{"CR",		MPC8XX_SPR_CR,		"CR0|4CR1|8CR2|12CR3|16CR4|20CR5|24CR6|28CR7","Condition Register","UM140"},
{0, 		0,			0,0,0	/*last entry for sprnames*/}
};


int mpc8xx_spr_find_num( const char *pch )
{
	int n;
	for( n = 0; mpc8xx_spr_names[n].name; ++n)
	{
		if( !strcasecmp( mpc8xx_spr_names[n].name, pch ) )
			return mpc8xx_spr_names[n].num;
	}

	return -1;
}

static void bprintf( char** buffer, int* size, const char* format, ... )
{
	int n;
	va_list args;

	if( *size <= 0 )
		return;
	
	va_start(args, format);

	n = vsnprintf( *buffer, *size, format, args );

	va_end(args);

	*size -= n;
	*buffer += n;
}

char* mpc8xx_spr_info(	int num, unsigned int val,
			int bName, int bDef, int bVal, int bPretty,
			char* buffer, int buffer_size )
{
	int i;
	int idx;
	int startbit,bit;
	int len;
	int bFirstPrinted;
	int bSPRI;
	unsigned int bitmask;
	unsigned int bitfieldval;
	const char *pFormat;
	const char *pFieldName;
	char FieldName[255];

	char* bp = buffer;
	int bs = buffer_size;	

	memset( buffer, 0, buffer_size );

	
	idx = -1;
	for(i = 0; mpc8xx_spr_names[i].name ; ++i)
	{
		if(mpc8xx_spr_names[i].num == num)
		{
			idx = i;
			break;
		}
	}

	bSPRI = (num & MPC8XX_SPRI_MASK);
	num &= (MPC8XX_GPR_REG_MASK-1); /*remove register markings*/

	if (idx < 0) /*not in sprnames*/
	{
		if (bName)
		{
			if (bSPRI)
			{
				bprintf(&bp, &bs, "SPRI 0x%x", num);
			}
			else
			{
				bprintf(&bp, &bs,  "SPR %d", num);
			}
		}
		if (bVal)
		{
			bprintf(&bp, &bs,  " = 0x%08x", val);
		}

		return buffer;
	}

	pFormat = mpc8xx_spr_names[i].namefield;
	if (!pFormat)	/*no bitfield description available?*/
	{
		if (bDef)
		{
			if (bSPRI)
			{
				bprintf(&bp, &bs, "IMMR + 0x%x",num);
			}
			else
			{
				bprintf(&bp, &bs, "SPR %d",num);
			}
			if ((bName) && (mpc8xx_spr_names[i].name))
				bprintf(&bp, &bs, " : %s", mpc8xx_spr_names[i].name);
		}
		else if ((bName) && (mpc8xx_spr_names[i].name))
			bprintf(&bp, &bs, "%s", mpc8xx_spr_names[i].name);

		if (bVal)
			bprintf(&bp, &bs, " = 0x%08x", val);
		if (bDef)
		{
			if (mpc8xx_spr_names[i].longname)
				bprintf(&bp, &bs, " , %s", mpc8xx_spr_names[i].longname);

			if (mpc8xx_spr_names[i].reference)
				bprintf(&bp, &bs, " , %s", mpc8xx_spr_names[i].reference);

			if ((mpc8xx_spr_names[i].longname) || (mpc8xx_spr_names[i].reference))
				bprintf(&bp, &bs, "\n");
		}
		return buffer; /*for no Format description, we are finished here */
	}

	/*start of pretty print*/
	if (bDef) /* print name and format description field?*/
	{
		bprintf(&bp, &bs, "%s := (%s)\n",mpc8xx_spr_names[idx].name,mpc8xx_spr_names[idx].namefield);

		if (bSPRI)
			bprintf(&bp, &bs, "IMMR + 0x%x:",num);
		else
			bprintf(&bp, &bs, "SPR %d:",num);

		if (mpc8xx_spr_names[i].longname)
		{
			bprintf(&bp, &bs, "%s", mpc8xx_spr_names[i].longname);

			if (mpc8xx_spr_names[i].reference)
				bprintf(&bp, &bs, ", ");
		}

		if (mpc8xx_spr_names[i].reference)
			bprintf(&bp, &bs, "%s", mpc8xx_spr_names[i].reference);

		bprintf(&bp, &bs, "\n");
	}
	if (bName)
	{
	 	if (mpc8xx_spr_names[i].name)
		{
			bprintf(&bp, &bs, "%s", mpc8xx_spr_names[i].name);
		}
		else
		{
			if (bSPRI)
				bprintf(&bp, &bs, "IMMR + 0x%x",num);
			else
				bprintf(&bp, &bs, "SPR %d",num);
		}
	}
	if (bVal)
	{
		bprintf(&bp, &bs, " = 0x%08x", val);
	}
	if (bPretty)
	{
		startbit = 0;
		bFirstPrinted = 0;
		if (isdigit(*pFormat))	/*extra case first field starting not at bit zero*/
		{
			i = sscanf(pFormat,"%d",&startbit);
			if ((startbit<0) || (startbit >PPC_BITS)|| (i!=1))
			{
				bprintf(&bp, &bs, "\nerror in bitfield description string %s\n",mpc8xx_spr_names[idx].namefield);
				bprintf(&bp, &bs, " wrong bit position %d at %s\n",startbit, pFormat);
				return NULL;
			}
			do
			{
				pFormat++;
			} while (isdigit(*pFormat));
		}
		while ((*pFormat))
		{
			pFieldName = pFormat;
			pFormat = strchr(pFormat,'|');
			if (!pFormat) /*last entry*/
			{
				bit = PPC_BITS; /*reached last bit*/
				len = strlen(pFieldName);
			}
			else
			{
				len = pFormat-pFieldName;
				pFormat++; /*skip |*/
				if (isdigit(*pFormat))	/*bitfield or single bit*/
				{
					i = sscanf(pFormat,"%d",&bit);
					if ((bit<0) || (bit >PPC_BITS) ||(bit <=startbit) || (i!=1))
					{
						bprintf(&bp, &bs, "error in bitfield description string %s\n",mpc8xx_spr_names[idx].namefield);
						bprintf(&bp, &bs, " wrong bit position %d at %s\n",bit,pFormat);
						return NULL;
					}
					do
					{
						pFormat++;
					} while (isdigit(*pFormat));
				}
				else bit = startbit + 1;
			}
			strncpy(FieldName,pFieldName,len);
			FieldName[len]= 0;

			bitmask = PPC_BIT(startbit);
			bitfieldval = 0;
			for (i = startbit; i<bit;i++)
			{
				bitfieldval = (bitfieldval << 1);
				if (val & bitmask) bitfieldval |=1;
				bitmask = bitmask >> 1;
			}
			if (len != 0) /*name, no spare bit field?*/
			{
				if (bit-startbit >1) /*multi bitfield*/
				{
					if (!bFirstPrinted)
					{
						bFirstPrinted = 1;
						bprintf(&bp, &bs, " = (");
					}
					else bprintf(&bp, &bs, "|");
					bprintf(&bp, &bs, "%s=0x%x",FieldName,bitfieldval);
				}
				else if (bitfieldval) /*single bit*/
				{
					if (!bFirstPrinted)
					{
						bFirstPrinted = 1;
						bprintf(&bp, &bs, " = (");
					}
					else bprintf(&bp, &bs, "|");
					bprintf(&bp, &bs, "%s",FieldName);
				}
			}
			if (bit >= PPC_BITS)
			{
				break;
			}
			startbit = bit;
		}
		if (bFirstPrinted) bprintf(&bp, &bs, ")");
	}

	return buffer;
}

void mpc8xx_spr_print_info(int num, unsigned int val,
			   int bName, int bDef, int bVal, int bPretty )
{
	char buffer[ 1024 ];

	mpc8xx_spr_info( num, val, bName, bDef, bVal, bPretty, buffer, 1024 );

	mpc8xx_printf( "%s", buffer );
}

mpc8xx_cpu_info_t mpc8xx_cpu_info[]=
{
	{ 0x00500000, 0x0502, 0x0000, "XPC860 Rev. D.4", 0, 0 },
	{ 0x00500000, 0x0501, 0x0000, "XPC860 Rev. D.3", 0, 0 },
	{ 0x00500000, 0x0031, 0x0065, "XPC860SR Rev. C.1", 0, 0 },
	{ 0x00500000, 0x0031, 0x0004, "XPC860 Rev. C.1", 0, 0 },
	{ 0x00500000, 0x0030, 0x0065, "XPC860SR Rev. C.0", 0, 0 },
	{ 0x00500000, 0x0030, 0x0004, "XPC860 Rev. C.0", 0, 0 },
	{ 0x00500000, 0x0020, 0x0064, "XPC860SR Rev. B.1", 0, 0 },
	{ 0x00500000, 0x0020, 0x0004, "XPC860 Rev. B", 0, 0 },
	{ 0x00500000, 0x0013, 0x0003, "XPC860 Rev. A.3", 0, 0 },
	{ 0x00500000, 0x0012, 0x0003, "XPC860 Rev. A.2", 0, 0 },
	{ 0x00500000, 0x0010, 0x0003, "XPC860 Rev. A.1", 0, 0 },
	{ 0x00500000, 0x0002, 0x0001, "XPC860 Rev. 0", 0, 0 },
	{ 0x00500000, 0x2101, 0x0067, "XPC850 Rev. B", "XPC850.reg", 0 },
	{ 0x00500000, 0x2100, 0x0065, "XPC850 Rev. A", "XPC850.reg", 0 },
	{ 0x00500000, 0x2002, 0x0001, "XPC850 Rev. 0", "XPC850.reg", 0 },
	{ 0x00000000, 0x0000, 0x0000, 0, 0, 0 } /* last entry for processor*/
};

int mpc8xx_print_cpu_info()
{
	unsigned int pvr;
	unsigned int partmask;
	unsigned int partnum;
	unsigned int masknum;
	unsigned int word;
	unsigned int revnum;
	mpc8xx_cpu_info_t* ci;
	int i;

	/* try to identify processor */
	pvr = mpc8xx_get_spr( MPC8XX_SPR_PVR ); /* get processor version register */

	partmask = 0x0000ffff & mpc8xx_get_spr( MPC8XX_SPR_IMMR); /* get IMMR */

	partnum = (partmask & 0x0000ff00) >> 8;
	masknum = partmask & 0x000000ff;

	word = mpc8xx_get_spri( MPC8XX_SPRI_REV_NUM ); /* get REV_NUM */
	revnum = ( word & 0xffff0000 ) >> 16;

	mpc8xx_printf("Target cpu PVR=0x%08X PARTNUM=0x%02X MASKNUM=0x%02X REV_NUM=0x%04X\n",
				pvr,partnum,masknum,revnum);



	ci = mpc8xx_cpu_info;
	for( i = 0; ci->name; i++, ci++ )
	{
		if( (pvr == ci->pvr) && (partmask == ci->partmask) && (revnum == ci->revnum) )
		{
			mpc8xx_printf("Target cpu is a '%s'\n",ci->name);
			if( ci->regfile )
			{
				mpc8xx_printf("Reading CPU register description file '%s'\n",ci->regfile);

				/* What should this do ?? */
			}

			return 0;
		}
	}

	mpc8xx_printf("warning: unknown CPU. Using default register description\n");

	return 0;
}

