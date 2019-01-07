#ifndef PZMCLASS_H
#define PZMCLASS_H

#include <cpuClass.h>
#include <oledClass.h>  // needed for diagnostics only

enum cmdindex_t         // index of metering commands
{
	VOLTS 		= 0,
	AMPS	 	= 1,
	WATTS 		= 2,
	ENERGY 		= 3,
	SET_ADDRESS	= 4
}; 
#define NONE ((float)0xF0F0)  // used as an ivalid number to stop simulation

enum pzemstate_t             // STATES
{
	PZ_IDLE            = 10,
	PZ_WAITFOR_1ST     = 20,
	PZ_WAITFOR_NEXT    = 30,
	PZ_DONE            = 0	
};

// be aware of DELAYS. The average command is completed in 22ms. But when one command is
// given immediately after the other, there is a delay as much as 600ms.
// Stage retrival of data every second or so.

class PZEM
{
private:
	
	u32 startT;			        // time command was issued. Used to timeout responses
	uint32 tmout;               // timeout waiting for response
    uint32 noerrdly;            // elapsed time to acquire response if no error
    int err;                    // error code; 0=no errors
    bool debugv;                // diagnostic flag
        
    int pribaudrate;            // if 0, do not change
    int secbaudrate;            // if 0, do not change
    int oe_pin;                 // if -1, do not use enable pin. OR with NEGATIVE_LOGIC is needed
    bool safeiov;               // true if primary, false is secondary
    
	byte response[7];	        // accumulating response. 
	int  rcv_count;		        // number of bytes received. Initialized by pzem.start() and updated by pzem.ready()
	
	pzemstate_t state;
      
    char  text[16];             // text string with value returned
    
    bool simonoff[4];                 // simulation, true or false
    float simvalue[4];           // simulated value 0=volts,...3=kWh
    float coilscale;              // coil scaling factor
    
    void sendCmd( cmdindex_t c ); 

protected:
    cmdindex_t    cmdindx;        // index in the command table. Command to be send is 0xB0+scancmd. Expecting 0xA0+scancmd

public:   
	void init(  uint32 ms = 1000L, 
	            int pbaud=0, 
	            int sbaud=0, 
	            int oe=-1 );
    
	void start( cmdindex_t c );	    // flips state to WAITINGFOR_1st
	bool ready( );	                // Call frequently. If true, getValue() and getUnit()

	float getValue();
    char *getValueText();
    
    char *getUnits();
    void simulate( cmdindex_t c, float value=NONE);
    
    int  error() {return err;}
    int  getDelay() {return noerrdly;}                // if no error, this returns the delay
    
    void swapSerialSec();
    void swapSerialPri();
    bool safeio(){ return safeiov; }                    // true if primary, false is secondary
    
    float readMeter( cmdindex_t c );
    bool debug() { return debugv; }    
    bool debug( bool onoff ) { return debugv=onoff; }

    float scaleAmps( float s ) {return coilscale = s;}
    float scaleAmps() {return coilscale;}
};

// class inherented by above. TO BE DEPRECATED. Not really needed

class PZM4 : public PZEM
{
public:

    int next;               // User convenience. Not used in the class
    struct meter_t
    {  
        float value;
        char text[16];
        char units[4];
    
    } meter[4];
    
    void start( int nxt );
    bool readyNext();
};
extern PZEM pzem;
extern PZM4 pzm4;

#endif
