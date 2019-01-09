//#define TEST1         // interactive demo CLI with etables[]
//#define TEST2         // typical boot with WiFi with no user EEPROM parms
  #define TEST3         // Includes USER parameters. CLI mode
  #define TEST3_INIT    // include both to initialize user parms
  
#include "eepTable.h"   
#include <FS.h>

// ------------------- allocation of base classes ------------------------------
    CPU cpu;
    CLI cli;
    EEP eep;
    Buf rbuf(500);          
// -----------------------------------------------------------------------------

#ifdef TEST1        // primitive boot in CLI to test functionality
void setup()
{
    cpu.init();
    SPIFFS.begin();                     // crashes sometimes if not present
    
    cli.init( ECHO_ON, "cmd: " );    // by default, ECO is OFF
    interact( &etable[0] );
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
    eep.printParms("Current Parms") );                   // print current parms

    cli.init( ECHO_ON, "cmd: " );       // by default, ECO is OFF
    interact( &etable[0] );
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

#ifdef TEST3_INIT
    if( int err = eep.fetchParms() )    // wifi and user fetch parms
    {
        eep.initWiFiParms();            // initialize with default WiFis
        eep.initUserParms();             
        // myeep = defaults             // this is an alternative to initUserParms()
        eep.saveParms();    
    }
    eep.updateRebootCount();
    PF( !eep.getParmString("Current Parms") );                   // print current parms
#endif

    cli.init( ECHO_ON, "cmd: " );       // by default, ECO is OFF
    interact( &etable[0] );
}
void loop() 
{
    yield();
}
#endif
