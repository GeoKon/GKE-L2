    #define TEST1   // uses CLI only
//  #define TEST2   // uses blocked mode
//  #define TEST3   // uses unblocked mode
    
    #include "owonClass.h"
    #include "cliClass.h"   // Support.h"
    #include <timeClass.h>

    CPU cpu;
    CLI cli;
    OLED oled;
    OWON owon;
/*
 * TEST1 289988 bytes (27%) of program storage space. Maximum is 1044464 bytes.
 * Global variables use 30864 bytes (37%) of dynamic memory, leaving 51056 bytes for local variables. Maximum is 81920 bytes.
 * Uploading 294128 bytes from C:\Users\GEORGE~1\AppData\Local\Temp\arduino_build_846785/owon.ino.bin to flash at 0x00000000
 * 
 */

#define  RESPONSE PF

uint16_t rdbuf1[]={ 0xF260, 0x0001, 0x004A };
uint16_t rdbuf2[]={ 0xF05A, 0x0004, 0x0118 };
extern COMMAND owontable[];

ModuloTic tic(5);  // every 5-sec

void setup() 
{
    cpu.init( 115200 );
    cli.init( ECHO_ON, "cmd: ", true/*flush on*/);
    
//    pinMode( 12, OUTPUT );  
//    digitalWrite( 12, LOW);
    
    oled.init( BIG );
    oled.dsp( 0, 0, "\aSTARTING");
    delay(1000);
    oled.clearDisplay();
    
    owon.init( 1200 /* timeout for serial */, 115200/*pri baud*/, 2400/*sec baud*/);
    cli.prompt();
}

void loop() 
{   
#ifdef TEST1
    if( cli.ready() )
    {
        if( !cli.exec( &owontable[0] ) )
            PF("Cmnd \"%s\" not found\r\n", *cli.args() );
        cli.prompt();
    }
#endif
#ifdef TEST2
    // THIS IS A BLOCKING VERSION THAT WORKS OK
    if( tic.ready() )
    {
        PF("Reading: ");
        owon.readMeter();
        if( !owon.error() )
        {
            PF( "%s\r\n", owon.getReading() );
                    
            oled.dsp( 1, 0, "\v%s%s%s %s\r", 
                owon.getValueText(), 
                owon.getUnits( true ),
                owon.getACDC(), 
                owon.getType(true) );
        }
        else
        {
            PF( "Timeout\r\n" );
            oled.dsp( 0, 0, "\vTIMEOUT\r" );
        }
    }
#endif
#ifdef TEST3
    if( tic.ready() )
    {
        PF("Triggered\r\n");
        owon.swapSerialSec();
        owon.start(); 
    }
    if( owon.ready( DEBUG_OFF ) )
    {    
        owon.swapSerialPri();
        
        PF("Reading: ");
        if( !owon.error() )
        {
            PF( "%s\r\n", owon.getReading() );
        
            oled.dsp( 0, 0, "\v%s%s %s\r", 
                owon.getValueText(), 
                owon.getUnits( true ),
                owon.getACDC()); 
                
            oled.dsp( 3, 0, "\v%s\r", 
                owon.getType(true) );
        }
        else
        {
            PF( "Timeout\r\n" );
            oled.dsp( 0, 0, "\vTIMEOUT\r" );
        }
    }
#endif
}
// ---------------------------------------------- CLI COMMANDS ----------------------------------------
 static void help1( int n, char **arg)
{
  COMMAND *row = &owontable[0];
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
 *    UART0 (Tx, Rx) (GPIO1, GPIO3) (.) 
 *  SWAP0 (Tx Rx)  (GPIO-15, GPIO-13) (D8, D7)
*   UART1 (Tx Rx)  (GPIO-2, GPIO-?) (D4, NC)
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
    PF("SWAPPING PORT to SEC (D8, D7)\r\n");
    delay( 200 );  // to empty the transmit buffer
    owon.swapSerialSec(); 
    PF("\r\nSEC ACTIVE\r\n");
}
static void cliSwapPri( int n, char **arg)
{
    PF("SWAPPING PORTS to PRI (USB)\r\n");
    delay( 200 );  // to empty the transmit buffer
    owon.swapSerialPri();
    PF("\r\nPRI ACTIVE\r\n");
}

static void synerr( char *arg0 )
{
  COMMAND *row = &owontable[0];
  for( int i=0; row->cmd ; i++, row++ )
  {
    if( strcmp(row->cmd, arg0)==0 )
    {
        RESPONSE( "Use: %s %s\r\n", arg0, row->help );
        return;
    }
  }
  RESPONSE("Impossible\r\n");
}
  
static void cliOLED( int n, char **arg)
{
    if( n<3 )
        synerr( arg[0] );
    else
    {
        int row = atoi( arg[1] );
        oled.dsp( row, 0, "\a%-10s", arg[2] );
    }
}
static void cliSimul( int n, char **arg)
{
    if( n>1 )
    {
        owon.simulate( true, atof( arg[1] ) );
        PF("Simulation ON\r\n");
    }
    else 
    {
        owon.simulate( false );
        PF("Simulation OFF\r\n");
    }
}
static void cliOWON( int n, char **arg)
{
    owon.readMeter();

//    PF("\r\nError=%d\r\n", owon.error() );
//    for( int i=0;i<MAX_RESP; i++ )
//        PF("%02X ", owon.response[i] );
//    PF("\r\n");

//    owon.extract();
//    for( int j=0; j<3; j++ )
//         PF("%04X ", owon.resp16[j] );
    
    PF("\r\n");
    PF( "Reading %f\r\n", owon.getValue() );
    PF( "Units %s %s\r\n", owon.getUnits(), owon.getACDC() );
    PF( "Type %s\r\n", owon.getType() );
    PF( "all %s\r\n", owon.getReading() );
}
COMMAND owontable[] // must be external to be able to used by the cliSupport
{
    {"h",       "help",                             help1    },
    {"health",  "Shows health stats",               cliHealth},
    {"oled",    "row text. Display to OLED",        cliOLED},
    {"ow",       "get OWON data",                    cliOWON},
    {"sim",      "x. Simulate reading",              cliSimul},
    
    {"pri",     "swap port to Primary",             cliSwapPri},
    {"sec",     "swap port to Secondary",           cliSwapSec},
  
    {NULL, NULL, NULL}
};
