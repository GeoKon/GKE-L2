/* ---------------------------------------------------------------------------------
 *          TEST APPLICATION: Exercises the PZ16CLASS
 *  
 *  Optionally MEGUNO for GUI interface.
 *  See \asset directory with support documents
 *  Author: GK. Original Nov 2019
 *  
 *  Copyright (c) George Kontopidis 1990-2019 All Rights Reserved
 *  You may use this code as you like, as long as you attribute credit to the author
 * ---------------------------------------------------------------------------------
 */
// ------------------------ REQUIRED BY xxxSupport MODULES ------------------------

	#include "pz16Class.h"
	#include "cliClass.h"   
    #include <ticClass.h>

#define USE_MEGUNO
#ifdef USE_MEGUNO
    #include <mgnClass.h>
    MGN mgn;
    void mgn_update();              // forward reference
#endif    

extern CMDTABLE ztable[]; 		    // forward ref

//--------------------------- allocation of base classes --------------------------
    CPU cpu;
    CLI cli;
    EXE exe;
    OLED oled;
    PZ16 pz( cpu, oled );

//------------------------------------- main code ---------------------------------

static int my_slave = 1;            // slave address
static int my_scan  = 0;            // enable scanning
static float simul_volts = 80.0;
static float simul_amps  = 10.0;
static int my_simul = 0;

void setup() 
{
	cpu.init(9600);    
    cpu.setSecBaud( 9600 );         // baudrate for PZEM
    
    pinMode( 12, OUTPUT );	        // GPIO12 is D4. Used to OE the tranlator
	digitalWrite( 12, LOW);

	oled.dsp( O_LED130 );
	oled.dsp( 0, "\vSTARTING");

    exe.initTables();
    exe.registerTable( ztable );
    
    cli.init( ECHO_ON, "cmd: ");    // flush enabled   
    cli.prompt();
}

TICms tice(1500);                      // ready every second
void loop() 
{    
    if( (!cpu.serialSwapped()) && cli.ready() )
    {
        exe.dispatchConsole( cli.gets() );
        
        if( !cpu.serialSwapped() )
            cli.prompt();
    }
    if( my_scan )
    {
        if( tice.ready() )
        {
            if( my_simul )
                pz.simulate( my_slave, simul_volts, simul_amps );
            else
                pz.measure( my_slave );     // measure all
                
            if( my_scan&1 )
                pz.print();
            if( my_scan&2 )
                mgn_update();
        }
    }
    delay( 250 );
}
// --------------------- CLI HANDLERS --------------------------------------

static void cliSetScan( int n, char *arg[])
{
    my_scan = (n > 1)? atoi( arg[1] ) : 0;
    PFN("scan = %d", my_scan );
}
static void cliFlashSlave( int n, char *arg[])
{
    int fls = (n > 1)? atoi( arg[1] ) : 1;
    my_slave = fls;
    pz.flashSlaveAddr( fls );
    PFN("slave addr = %d flashed", fls );
}
static void cliResetkWh( int n, char *arg[])
{
    my_slave = (n > 1)? atoi( arg[1] ) : 1;
    pz.resetkWh( my_slave );
}
static void cliSetTrace( int n, char *arg[])
{
    int tr = (n > 1)? atoi( arg[1] ) : 0;
    pz.setTrace( tr );
    PFN("trace = %d", tr );
}
static void cliMeasure( int n, char *arg[])
{
    my_slave = (n > 1)? atoi( arg[1] ) : 1;
    pz.measure( my_slave );
    pz.print();
}
static void cliGetVolts( int n, char *arg[])
{
    pz.measure( my_slave, 1 );
    PFN("%.1fV", pz.getVolts() );
}
static void cliGetVoltsAmps( int n, char *arg[])
{
    pz.measure( my_slave, 3 );
    PFN("%.1fV %.3fA", pz.getVolts(), pz.getAmps() );
}
static void _Simulate( int n, char *arg[] )
{
    int test = (n > 1)? atoi( arg[1] ) : 0;
    if( test == 0 )         // stop simulation
        my_simul = 0;       // flage used to do updates
    else
    {
        my_simul = 1;
        my_slave = test;
        simul_volts = (n > 2)? atof( arg[2] ) : 100.0;
        simul_amps  = (n > 3)? atof( arg[3] ) : 5.0;
        pz.simulate( my_slave, simul_volts, simul_amps );
        pz.print();
    }
}
static void cliSimulate( int n, char *arg[] )
{
    _Simulate( n, arg );
    if( my_simul )
        pz.print();
}

#ifdef USE_MEGUNO
void mgn_update()
{
    B80( s );
    s.set( "%.1fV", pz.getVolts() );    mgn.controlSetText( "dVolts1", s.c_str() );
    s.set( "%.2fA", pz.getAmps() );     mgn.controlSetText( "dAmps1",  s.c_str() );
    s.set( "%.0fW", pz.getWatts() );    mgn.controlSetText( "dWatts1", s.c_str() );
    s.set( "%.3fkWh", pz.getkWh() );    mgn.controlSetText( "dkWh1",   s.c_str() );
    s.set( "%.3f", pz.getPF() );        mgn.controlSetText( "dPF1",    s.c_str() );

    mgn.controlSetValue( "mVolts1", pz.getVolts()  );
    mgn.controlSetValue( "mAmps1",  pz.getAmps()  );
}
static void mgnMeasure( int n, char *arg[] )
{
    my_slave = (n > 1)? atoi( arg[1] ) : 1;
    pz.measure( my_slave );
    mgn_update();
}
static void mgnResetkWh( int n, char *arg[])
{
    my_slave = (n > 1)? atoi( arg[1] ) : 1;
    pz.resetkWh( my_slave );
    mgn.controlSetText( "dkWh1","0.000kWh" );
}
static void mgnSimulate( int n, char *arg[] )
{
    _Simulate( n, arg );
    if( my_simul )
        mgn_update();
}
#endif

CMDTABLE ztable[] // must be external to be able to used by the cliSupport
{
    {"h",     	"help",         [](int n, char *arg[]){ exe.help(n,arg);} },    
  	{"scan",	"0|1=print|2=meguno. Enable Scanning",  cliSetScan},  	
    
    {"trace",   "n. Set trace of MODBUS",               cliSetTrace},
    {"meas",	 "[slave]. Measure",	                cliMeasure},
    
    {"v",       "Volts",                                cliGetVolts},
    {"va",      "Volts and Amps",                       cliGetVoltsAmps},
    {"simul",   "0=off | slave volts amps. Simulate",   cliSimulate},
    {"resetkWh","resets kWh",                           cliResetkWh},
    
    {"flashSlave","addr. Flash new slave address",      cliFlashSlave},

#ifdef USE_MEGUNO
    {"!meas",    "",                                    mgnMeasure},
    {"!simul",   "",                                    mgnSimulate},
    {"!zero",    "",                                    mgnResetkWh},
    {"!scan",    "",                                    cliSetScan},    
#endif
    {NULL, NULL, NULL}
};
