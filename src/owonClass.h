#ifndef OWONCLASS_H
#define OWONCLASS_H

#include <cpuClass.h>
#include <oledClass.h>  // for diagnostics

enum owon_t             // STATES
{
	PZ_IDLE            = 1,
	PZ_WAITFOR_NEXT    = 2,
	PZ_TIMEOUT         = 3,
	PZ_DONE            = 0	
};
#define DEGUG_ON    true
#define DEBUG_OFF   false

// be aware of DELAYS. The average command is completed in 22ms. But when one command is
// given immediately after the other, there is a delay as much as 600ms.
// Stage retrival of data every second or so.

#define MAX_RESP 11

class OWON
{
private:
	u32 startT;			        // time command was issued. Used to timeout responses
	uint32 tmout;               // timeout waiting for response
   
	int  rcv_count;		        // number of bytes received. Initialized by pzem.request() and updated by pzem.ready()
	owon_t state;
    int err;

	byte response[MAX_RESP];          // accumulating response. 
    uint16 resp16[3]; 
    int function, scale, decimal;     // defined by extract() using the response[]

    int pribaudrate;            // if 0, do not change
    int secbaudrate;            // if 0, do not change
    int oe_pin;                 // if -1, do not use enable pin. OR with NEGATIVE_LOGIC is needed

    bool simul;                 // simulation, true or false
    
    bool extract();
    
public:
	void init( uint32 ms = 200L, int pbaud=0, int sbaud=0, int oe=-1 );
    
	void start();	                // flips state to WAITINGFOR_1st
	
	bool ready( bool debug=DEBUG_OFF );	// Call frequently. If true, getValue() and getUnit()

    char *getReading();

    void simulate( bool onoff, float value=0.0 ); 
    
    float getValue();
    char *getValueText();
    char *getUnits( bool ascii=false );
    char *getACDC();
    char *getType( bool ascii=false );
    int   error();
    
    void swapSerialSec();
    void swapSerialPri(); 

    int readMeter();
};

extern OWON owon;
    
#endif
