#define TEST 2
/*                SELECTION OF TESTS 
 * 
 * 1 = interactive blocking demo CLI in setup() 
 * 2 = typical boot with WiFi with no user EEPROM parms
 * Use the eepGlobals.ino to test user+eep parameters
*/
#include <cliClass.h>       // CLI and EXE classes. Also includes the cpuClass.h
#include <eepClass.h>   
#include "eepTable.h"

#include <FS.h>

// --------------- forward references (in this file) ---------------------------

    void interactForever();
    namespace mypTable 
    {
        extern void init( EXE &myexe );
        extern CMDTABLE table[]; 
    }
// ------------------- allocation of base classes ------------------------------
    CPU cpu;
    CLI cli;
    EXE exe;
    EEP eep;

// -----------------------------------------------------------------------------
#if TEST == 1        // primitive boot in CLI to test functionality
void setup()
{
    cpu.init();
    SPIFFS.begin();                     // crashes sometimes if not present
    PF("This is test #%d\r\n", TEST );
    
    mypTable::init( exe );              // use namespace 'myTable' to associate the exe class
    eepTable::init( exe, eep );         // use namespace 'eepTable' to associate the eep tables
     
    exe.registerTable( mypTable::table );
    exe.registerTable( eepTable::table );
    
    interactForever();
}
#endif

// -----------------------------------------------------------------------------
#if TEST == 2        // typical boot with WiFi with no user EEPROM parms

#define myMAGIC 0x1234
void setup()
{
    cpu.init();
    SPIFFS.begin();  
    PF("This is test #%d\r\n", TEST );
    
    if( !eep.checkEEParms( myMAGIC, 0 ) )    // fetches parameters and returns TRUE or FALSE
    {
        PF("Initializing parms!\r\n" );
        eep.initHeadParms( myMAGIC, 0 );
        eep.initWiFiParms();            // initialize with default WiFis
    }
    eep.incrBootCount();
    eep.printHeadParms("--- Current Parms");    // print current parms
    eep.printWiFiParms();                 
    
    eepTable::init( exe, eep );         // create link to eep tables
    mypTable::init( exe );               // create link to MY table
     
    exe.registerTable( mypTable::table );
    exe.registerTable( eepTable::table );
    
    interactForever();
}
#endif

void loop() 
{
    yield();
}

// ----------------------------- local CLI tables ---------------------------
namespace mypTable
{
    static EXE *_exe;       //pointer to EXE class defined in main()
    
    void init( EXE &myexe ) {_exe = &myexe;}
    void help( int n, char *arg[] ) {_exe->help( n, arg );}

    CMDTABLE table[]= 
    {
        {"h", "Help! List of all commands", help },
        {NULL, NULL, NULL}
    };
}

// -----------------------------------------------------------------------------
void interactForever()
{
    cli.init( ECHO_ON, "cmd: " );       
    PR("CLI Mode. Press 'h' for help\r\n");
    
    cli.prompt();
    for(;;)
    {
        if( cli.ready() )
        {
            char *p = cli.gets();
//            exe.dispatchConsole( p );
//            PF("--- Repeated ---\r\n");
            
            BUF temp(2000);                // response buffer
            exe.dispatchBuf( p, temp );
            temp.print();
            
            cli.prompt();
        }
    }
}
