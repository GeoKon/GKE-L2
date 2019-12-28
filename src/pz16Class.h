/* ---------------------------------------------------------------------------------
 *  Copyright (c) George Kontopidis 1990-2019 All Rights Reserved
 *  You may use this code as you like, as long as you attribute credit to the author
 * ---------------------------------------------------------------------------------
 */

#pragma once
#include <cpuClass.h>
#include "oledClass.h"

// -------------------------- UTILITIES ------------------------------------

    uint16_t crc16( uint16_t crc, int a );            // computes CRC
    uint16_t crcBlock( const byte b[], size_t n );    // computes CRC
    bool ishex( const char c, int *ip );            // checks & converts to hex

// -------------------------- MODBUS BASE CLASS ----------------------------

#define PZ_CMDSIZE (2 + 4 + 2)      // in bytes = addr+cmd+basex2+countx2+crcx2
#define PZ_RCVSIZE (3 + 9*2 + 2)
#define PZ_TMOUT1st 500             // 9600 baud = 960 char/sec = 1ms/char
#define PZ_TMOUTnth 10              // must be at least 3.5 char equivalent
#define PZ_FLUSH    10

#define PZ_16BITS(A,I) (A[I]*256+A[I+1])
#define PZ_32BITS(A,I) (A[I]*256+A[I+1])+(A[I+2]*256+A[I+3])*0x10000L
#define PZ_MSB(A) ((A)>>8)&0xFF
#define PZ_LSB(A) (A)&0xFF

#define PZ_DSP if(ole) ole->dsp
class MODBUS
{
private:
    OLED *ole;
    
protected:    
    byte cmdbytes[PZ_CMDSIZE];
    int  cmdcount;    
    byte rcvbytes[PZ_RCVSIZE];
    int  rcvcount;

    int tmerror;
    
public:
    MODBUS();
    void setOLED( OLED *poled = NULL );					// specifies OLED for diagnostics.
    void fillCMD( byte slave, byte PDU[], int count );  // Fills the cmdbyte[] array
    void parseCMD( const char *s );                     // Fills the cmdbyte[] array 
    void printCMD( char *prompt="" );                   // prints cmdbyte[] array
    void sendCMD();                                     // Sends command. Optional OLED
    
    void fillRSP( byte slave, byte PDU[], int count );  // Fills response (used for simulation)
    int receiveRSP( const int expectedcount );          // Receives rspbytes[]. Optional OLED
    void printRSP( char *prompt="" );                   // Prints rspbyte[] array
    int rcvError();
};

// -------------------------- PZM-016 CLASS ----------------------------

class PZ16: public MODBUS
{
private:
    CPU *mpu;
    bool error;
    int trace;
    
public:
	PZ16( CPU &pcpu, OLED &poled );				// Specify CPU and OLED (used for MODBUS)
	int measure( int slave, int nreg=9 );		// Sends cmd and receives response

	int simulate( int slave, 					// Same as above but with simulated data`
					float volts, 
					float amps );	
					
	void setTrace( int value );					// 0=no trace, 1=printf of low 
	bool getError();							// Measure error 0=none, 1=tmout 2=data err

	float getVolts();							// Reports after measure() or simulate()
	float getAmps();
	float getWatts();
	float getkWh();
	float getHertz();
	float getPF();
	void print( char *prompt="" );				// prints all above(diagnostics)

	int flashSlaveAddr( int newaddr );			// Broadcast Slave Address (no response)
	int resetkWh( int slave );							
    
};
