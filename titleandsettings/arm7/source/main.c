/*---------------------------------------------------------------------------------

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>

#include "ResetData_Mario64.h"
#include "RocketLauncher.h"	// RocketLauncher payload

unsigned int * SCFG_ROM=(unsigned int*)0x4004000;
unsigned int * SCFG_CLK=(unsigned int*)0x4004004; 
unsigned int * SCFG_EXT=(unsigned int*)0x4004008;
unsigned int * SCFG_MC=(unsigned int*)0x4004010;
unsigned int * CPUID=(unsigned int*)0x4004D00;
unsigned int * CPUID2=(unsigned int*)0x4004D04;

//---------------------------------------------------------------------------------
void RocketLauncherMode() {
//---------------------------------------------------------------------------------
	memcpy((u32*)0x02000000,ResetData_Mario64,0x560);
	memcpy((u32*)0x02800000,payload_RocketLauncher,payload_RocketLauncher_len);
	i2cWriteRegister(0x4A, 0x70, 0x01);		// Bootflag = Warmboot/SkipHealthSafety
	i2cWriteRegister(0x4A, 0x11, 0x01);		// Soft-reset to Slot-1 card
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	if(fifoGetValue32(FIFO_USER_08) != 0) {
		RocketLauncherMode();
	}
}

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB() {
//---------------------------------------------------------------------------------
	exitflag = true;
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
    nocashMessage("ARM7 main.c main");
	
	// clear sound registers
	dmaFillWords(0, (void*)0x04000400, 0x100);

	REG_SOUNDCNT |= SOUND_ENABLE;
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	powerOn(POWER_SOUND);

	readUserSettings();
	ledBlink(0);

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
	
	fifoInit();
	
	SetYtrigger(80);
	
	installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);

	setPowerButtonCB(powerButtonCB);
	
	fifoSendValue32(FIFO_USER_01, *SCFG_ROM);
	fifoSendValue32(FIFO_USER_02, *SCFG_CLK);
	fifoSendValue32(FIFO_USER_03, *SCFG_EXT);
	fifoSendValue32(FIFO_USER_04, *CPUID2);
	fifoSendValue32(FIFO_USER_05, *CPUID);
	fifoSendValue32(FIFO_USER_06, 1);
	
	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		// fifocheck();
		swiWaitForVBlank();
	}
	return 0;
}

