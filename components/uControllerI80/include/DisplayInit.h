#pragma once
#include "esp_system.h"

#define TFT_INIT_DELAY 0x80
#define TFT_INIT_CMD_ARRAY_END 0xFF
#define TFT_INIT_CMD_DATABYTES_MASK 0x7F

typedef struct ScreenCmd
{
	uint8_t Cmd;
	uint8_t Data[16];
	uint8_t DataBytes; // Number of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} TScreenCommand;


#ifndef _ILI9163_INIT_
#define _ILI9163_INIT_

#ifndef _ILI9163_DEFINES_
#define _ILI9163_DEFINES_

#define ILI9163_WIDTH 	128
#define ILI9163_HEIGHT 	160

#define ILI9163_NOP     0x00  // No Operation
#define ILI9163_SWRST   0x01  // Software Reset

#define ILI9163_RDIDINF 0x04  // Read Display Identification Information
#define ILI9163_RDSTAT  0x09  // Read Display Status
#define ILI9163_RDPWMO	0x0A  // Read Display Power Mode
#define ILI9163_RDMACTL 0x0B  // Read Display MADCTL
#define ILI9163_RDPXFMT 0x0C  // Read Display Pixel Format
#define ILI9163_RDIMGMO 0x0D  // Read Display Image Mode
#define ILI9163_RDSIGMO 0x0E  // Read Display Signal Mode

#define ILI9163_SLPIN   0x10  // Sleep In
#define ILI9163_SLPOUT  0x11  // Sleep Out
#define ILI9163_PTLON   0x12  // Partial Mode On def: Normal On
#define ILI9163_NORON   0x13  // Normal Mode On def: Normal On

#define ILI9163_INVOFF  0x20  // Inversion Off
#define ILI9163_INVON   0x21  // Inversion On
#define ILI9163_GAMSET  0x26  // Gamma Set def: Gamma Curve 1 (01h)
#define ILI9163_DISOFF  0x28  // Display Off 
#define ILI9163_DISON   0x29  // Display On

#define ILI9163_CASET   0x2A  // Column Address Set
#define ILI9163_PASET   0x2B  // Page Address Set 
#define ILI9163_RAMWR   0x2C  // Memory Write
#define ILI9163_RAMRD   0x2E  // Memory Read

#define ILI9163_PTLAR   0x30  // Partial Area
#define ILI9163_VSCRDEF 0x33  // Vertical Scrolling Definition
#define ILI9163_TEOFF   0x34  // Tearing Line Off
#define ILI9163_TEON    0x35  // Tearing Line On
#define ILI9163_MACTL   0x36  // Memory Access Control
#define ILI9163_VSCRSADD  0x37  // Vertical Scrolling Start Address
#define ILI9163_IDMOFF  0x38  // Idle Mode Off
#define ILI9163_IDMON   0x39  // Idle Mode On
#define ILI9163_IPF     0x3A  // Interface Pixel Format

#define ILI9163_FRCTL1  0xB1  // Frame Rate Control (Normal Mode / Full Colors)
#define ILI9163_FRCTL2  0xB2  // Frame Rate Control (Idle Mode / 8-bit Colors)
#define ILI9163_FRCTL3  0xB3  // Frame Rate Control (Partial Mode/ Full colors)
#define ILI9163_DISINV  0xB4  // Display Inversion

#define ILI9163_PWRCTL1 0xC0  // Power Control 1 [Set the GVDD and voltage]
#define ILI9163_PWRCTL2 0xC1  // Power Control 2 [Set the AVDD, VCL, VGH and VGL supply power level]
#define ILI9163_PWRCTL3 0xC2  // Power Control 3 [Set the amount of current in Operation amplifier in Normal Mode/ Full Colors]
#define ILI9163_PWRCTL4 0xC3  // Power Control 4 [Set the amount of current in Operation amplifier in Idle Mode / 8-bit Color]
#define ILI9163_PWRCTL5 0xC4  // Power Control 5 [Set the amount of current in Operation amplifier in Partial Mode / Full Color]

#define ILI9163_VCOMCTL1 0xC5
#define ILI9163_VCOMOFF  0xC7

#define ILI9163_GAMPOS  0xE0  // Positive Gamma Adjustment
#define ILI9163_GAMNEG  0xE1  // Negative Gamma Adjustment
#define ILI9163_GAMADJ  0xF2  // Gamma Adjustment Enable [E0h and E1h enable control]

#endif

// Initialization commands for ILI9163 screens
DRAM_ATTR static const TScreenCommand ILI9163_InitCommandArray[] = {
	{ILI9163_SWRST, 	{0}, TFT_INIT_DELAY},  	// Software reset
	{ILI9163_SLPOUT, 	{0}, TFT_INIT_DELAY},   // Exit sleep mode

    // Color Settings
	{ILI9163_IPF, 	    {0xE6}, 1}, 			// Set pixel format // P.143 E = 1110 [18-bit (1 Pixel / 3 Transfers)] | 6 = 0110 [18-bit / Pixel]

    // Gamma Settings
	{ILI9163_GAMSET, 	{0x04}, 1}, 			// Set Gamma curve 3
	{ILI9163_GAMADJ,  	{0x01}, 1}, 			// Gamma adjustment enabled
	{ILI9163_GAMPOS, 	{0x3F, 0x25, 0x1C, 0x1E, 0x20, 0x12, 0x2A, 0x90, 0x24, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00}, 15},    // Positive Gamma
	{ILI9163_GAMNEG, 	{0x20, 0x20, 0x20, 0x20, 0x05, 0x00, 0x15, 0xA7, 0x3D, 0x18, 0x25, 0x2A, 0x2B, 0x2B, 0x3A}, 15}, 	// Negative Gamma

	{ILI9163_FRCTL1, 	{0x05, 0x02}, 2},		// Frame Rate Control (Normal Mode / Full Colors) / 0x04, 0x02 = 154.32 FPS
	{ILI9163_PWRCTL1,  	{0x1E, 0x00}, 2}, 		// Power control 1
	{ILI9163_VCOMCTL1, 	{0x1F, 0x5B}, 2}, 		// Vcom control 1
	{ILI9163_CASET, 	{0x00, 0x00, 0x00, ILI9163_WIDTH - 1}, 4},	// Set column address
	{ILI9163_PASET, 	{0x00, 0x00, 0x00, ILI9163_HEIGHT - 1}, 4}, // Set page address
	{ILI9163_TEON, 		{0x00}, 1},				// Tearing Effect On / 0x00 = V-Blanking Only | 0x01 = V- & H-Blanking

	{ILI9163_DISON, 	{0}, TFT_INIT_DELAY},   // Set display on
	{0, 				{0}, TFT_INIT_CMD_ARRAY_END}
};

#endif