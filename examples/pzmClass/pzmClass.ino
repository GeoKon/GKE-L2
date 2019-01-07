//#define TEST1
  #define TEST2
//#define TEST3
  
// ------------------------ REQUIRED BY xxxSupport MODULES ------------------------

	#include "pzmClass.h"
	#include "cliClass.h"   
    #include <timeClass.h>
    
extern COMMAND ztable[]; 		// forward ref
#define RESPONSE PF             // convenient response to server or printf

//--------------------------- allocation of base classes --------------------------
    CPU cpu;
    CLI cli;
    OLED oled;
    PZEM pzem;
    PZM4 pzm4;
//------------------------------------- main code ---------------------------------
void setup() 
{
	cpu.init();
    cli.init( ECHO_ON, "cmd: ", true);  // flush enabled
    
	pinMode( 12, OUTPUT );	            // GPIO12 is D4. Used to OE the tranlator
	digitalWrite( 12, LOW);
	
	oled.init( LARGE );
	oled.dsp( 0, 0, "\vSTARTING");

#ifdef TEST2
    pzm4.init( 2000L, 115200, 9600, 12 );
    pzm4.next = 0;   
#else
    pzem.init( 2000L, 115200, 9600, 12 );
#endif
    cli.prompt();
}

#ifdef TEST3    // ------------------------- TESTS THE BASE CLASS UNBLOCKED ---------------------------------
void loop() 
{    
    if( pzem.safeio() && cli.ready() )
    {
        if( !cli.exec( &ztable[0] ) )
            cpu.printf("Cmnd \"%s\" not found\r\n", *cli.args() );
        
        // the above, MIGHT switch port to secondary
        
        if( pzem.safeio() )
            cli.prompt();
    }
    if( pzem.ready() )
    {
         pzem.swapSerialPri();
         PF("Reading:%-8s %s\terr=%d delay=%dms\r\n", 
            pzem.getValueText(), 
            pzem.getUnits(),
            pzem.error(), 
            pzem.getDelay() );   
    }
    delay( 10 );
}
#endif

#ifdef TEST2    // ------------------------- TESTS THE DERIVED CLASS PZM4 ---------------------------------

ModuloTic myTic( 5 );           // issue a pzem.start() command every N-seconds

void loop() 
{    
    if( myTic.ready() )
    {
        PF("Starting next=%d\r\n", pzm4.next );
        pzm4.swapSerialSec();
        pzm4.start( pzm4.next++ );      // trigger next measurement
    }

    if( pzm4.readyNext() )
    {
         pzm4.swapSerialPri();
         for( int i=0; i<4; i++ )
            PF("%5.2f %-8s %s\r\n", pzm4.meter[i].value, pzm4.meter[i].text, pzm4.meter[i].units );    
    }
}
#endif

#ifdef TEST1    // ------------------------- TESTS THE BASE CLASS PZEM ---------------------------------
void loop() 
{	
	if( cli.ready() )
	{
		if( !cli.exec( &ztable[0] ) )
		    PF("Cmnd \"%s\" not found\r\n", *cli.args() );
        cli.prompt();
	}
	delay(10);
}
#endif

static void cliGetDebug( int n, char **arg)
{
    int q;

    pzem.debug( true );
    for( int i=1; i<n; i++ )
    {
        q =  atoi( arg[i] );
    
        pzem.swapSerialSec();
        pzem.start( (cmdindex_t) q );         // debug OLED enabled
        while( !pzem.ready() )                  // long timeout
            yield();
        pzem.swapSerialPri();
    
        PF("\r\nReading:%-8s %s\terr=%d delay=%dms", 
            pzem.getValueText(), 
            pzem.getUnits(),
            pzem.error(), 
            pzem.getDelay() );
    }
    PF("\r\n");
}
static void cliGetMeter( int n, char **arg)
{
    int q;

    pzem.debug( false );
    for( int i=1; i<n; i++ )
    {
        q =  atoi( arg[i] );
        pzem.readMeter(  (cmdindex_t) q );   
    
        PF("\r\nReading:%-8s %s\terr=%d delay=%dms", 
            pzem.getValueText(), 
            pzem.getUnits(),
            pzem.error(), 
            pzem.getDelay() );
    }
    PF("\r\n");
}
static void cliStart( int n, char **arg)
{
    if( n<=1 )
    {
        PF("Arg required\r\n");
        return;
    }
    PF("Starting...\r\n");
    delay(10);
        
    pzem.debug( false );
    pzem.swapSerialSec();
    pzem.start( (cmdindex_t) atoi( arg[1] ) );   
}
static void cliSimul( int n, char **arg)
{
    float f;
    if( n>2 )
    {
        pzem.simulate( (cmdindex_t)atoi(arg[1]), f=atof( arg[2] ) );
        PF("Simulation ON of %6.3f\r\n", f );
    }        
    else if( n>1) 
    {
        pzem.simulate( (cmdindex_t)atoi(arg[1]), NONE );
        PF("Simulation OFF\r\n");
    }
    else
        PF("Must specify 0...3\r\n");
}

static void help1( int n, char **arg)
{
  COMMAND *row = &ztable[0];
  for( int i=0; row->cmd ; i++, row++ )
  {
    RESPONSE( row->cmd );
    RESPONSE( "\t" );
    RESPONSE( "%s\r\n", row->help );
  }
}
/*
 * Serial uses UART0, which is mapped to pins (
 * 
 * 	UART0 (Tx, Rx) (GPIO1, GPIO3) (.) 
 * 	SWAP0 (Tx Rx)  (GPIO-15, GPIO-13) (D8, D7)
* 	UART1 (Tx Rx)  (GPIO-2, GPIO-?) (D4, NC)
 * 	
 * Serial may be remapped to GPIO15 (TX) and GPIO13 (RX) by calling Serial.swap() after Serial.begin. 
 * Calling swap again maps UART0 back to GPIO1 and GPIO3.

	Serial1 uses UART1, TX pin is GPIO2 (D4). 
	UART1 can not be used to receive data because normally it's RX pin is occupied for 
	flash chip connection. To use Serial1, call Serial1.begin(baudrate).

 	D4  GPIO2  TXD1
 	D7  GPIO13 RXD2
 	D8  GPIO15 TXD2
 
 */
static void cliHealth( int n, char **arg)
{
	RESPONSE( "Heap used=%d, max=%d\r\n", cpu.heapUsedNow(), cpu.heapUsedMax() );
}
static void cliSwapSec( int n, char **arg)
{
	Serial.printf("SWAPPING PORT to SEC (D8, D7)\r\n");
    delay( 200 );  // to empty the transmit buffer
    
    pzem.swapSerialSec(); 
	Serial.printf("\r\nSEC ACTIVE\r\n");
}
static void cliSwapPri( int n, char **arg)
{
	Serial.printf("SWAPPING PORTS to PRI (USB)\r\n");
    delay( 200 );  // to empty the transmit buffer
    pzem.swapSerialPri();
	Serial.printf("\r\nPRI ACTIVE\r\n");
}

COMMAND ztable[] // must be external to be able to used by the cliSupport
{
    {"h",     	"help", 				            help1    },
  	{"health",	"Shows health stats",	            cliHealth},
  	
    {"pri",		"swap port to Primary",	            cliSwapPri},
    {"sec",		"swap port to Secondary",			cliSwapSec},
    {"dbg",     "(0=Volts 1=Amps 2=kW 3=kwh)* (debug)",cliGetDebug},
    {"mter",    "(0=Volts 1=Amps 2=kW 3=kwh)*",        cliGetMeter},
    {"simul",   "0=V...3=kWh [value]. Activates simulation",    cliSimul},
    {"start",   "0=Volts 1=Amps 2=kW 3=kwh.. USE ONLY FOR TEST3", cliStart},
    {NULL, NULL, NULL}
};
