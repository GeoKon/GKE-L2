//  #define TEST1         // interactive blocking demo CLI in setup() 
//  #define TEST2         // typical boot with WiFi with no user EEPROM parms
    #define TEST3         // Includes USER parameters. CLI mode
  
#include <cliClass.h>       // CLI and EXE classes. Also includes the cpuClass.h
#include <eepClass.h>   
#include "eepTable.h"

#include <FS.h>

// --------------- forward references (in this file) ---------------------------

    extern void interactForever();
    namespace myTable 
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
#ifdef TEST1        // primitive boot in CLI to test functionality
void setup()
{
    cpu.init();
    SPIFFS.begin();                     // crashes sometimes if not present

     myTable::init( exe );
    eepTable::init( eep );              // create link to eep tables
     
    exe.registerTable(  myTable::table );
    exe.registerTable( eepTable::table );
    
    interactForever();
}
void loop() 
{
    yield();
}
#endif

// -----------------------------------------------------------------------------
#ifdef TEST2        // typical boot with WiFi with no user EEPROM parms
void setup()
{
    cpu.init();
    SPIFFS.begin();                     // crashes sometimes if not present
    
    int err = eep.fetchParms();          // fetch parms
    if( err )
    {
        PF("Initalizing parms (code %d)!\r\n", err );

        eep.initWiFiParms();            // initialize with default WiFis
        eep.saveParms();    
    }
    eep.updateRebootCount();
    eep.printParms("Current Parms");    // print current parms

     myTable::init( exe );
    eepTable::init( eep );              // create link to eep tables
     
    exe.registerTable(  myTable::table );
    exe.registerTable( eepTable::table );
    
    interactForever();
}
void loop() 
{
    yield();
}
#endif

// -----------------------------------------------------------------------------
#ifdef TEST3                // Includes USER parameters. CLI mode
struct myparm_t
{
    float x;
    int n;
    char test[16];
    int i;
} myeep, defaults = {2.3, 10, "nothing", 45};

#define FORMAT "USER PARMS\r\n%f %d %s %d\r\n"
    
void setup()
{
    cpu.init();
    SPIFFS.begin();                     // crashes sometimes if not present

    eep.registerUserParms( &myeep, sizeof( myparm_t ), FORMAT );
    eep.registerInitCallBk( [](){ myeep=defaults; } );

    if( int err = eep.fetchParms() )    // wifi and user fetch parms
    {
        eep.initWiFiParms();            // initialize with default WiFis
        eep.initUserParms();             
        eep.saveParms();    
    }
    eep.updateRebootCount();
    eep.printParms("Current Parms");    // print current parms
    
    myTable::init( exe );
    eepTable::init( eep );              // create link to eep tables
     
    exe.registerTable(  myTable::table );
    exe.registerTable( eepTable::table );
    
    interactForever();
}
void loop() 
{
    yield();
}
#endif

// ----------------------------- local CLI tables ---------------------------
namespace myTable
{
    static EXE *exe;
    void init( EXE &myexe )
    {
        exe = &myexe;
    }
    void help( int n, char **arg, Buf &bf )     
    {
        if( n<=1 ) exe->printTables("");
        else       exe->printHelp( arg[1] );
    }
    CMDTABLE table[]= // must be external to be able to used by the cliSupport
    {
        {"h",       "Help! List of all commands",             help     },
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
            //PF("See string %s\r\n", p );
            exe.dispatch( p );
            x.print();
            cli.prompt();
        }
    }
}
