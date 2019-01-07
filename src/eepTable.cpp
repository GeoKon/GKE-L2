/*
Summary
-------
<a id="example"></a>
- __registerUserParms()__ registers user parameters & format
- __registerInitCallbk()__ resisters parameter initiaziation callback function
<br><br>
- __initWiFiParms()__ initializes WiFi parameter structure
- __initUserParms()__ initializes user parameter structure calling the registed callback
- __saveParms( sel )__ saves parameters in EEPROM
- __fetchParms( sel )__ fetches parameters from EEPROM and populates the WiFi and user data structures, if previously registered. Returns an error code or 0 if all parameters were fetched.<br><br> __sel__ can take the following values

> HEAD_PARMS to refer to header parameters<br>
> WIFI_PARMS to refer to WiFi parameters<br>
> USER_PARMS to refer to user parameter<br>
> ALL_PARMS to refer to all above<br>
> *default* __sel__ is ALL_PARMS
- __getParmsString( sel )__ returns a String with a selection of parameters
- __printParms( sel )__ prints the parameters selectively
- __updateWiFiParms(...)__ defines new wifi parameters and updates eeprom
- __updateRebootCount()__ increments reboot counter and updates eeprom

Code Example
------------
Define user parm data structure and format

`struct { int i1, i2, i3 } us, us0={10, 20, 30};`<br>
`char *format ="i1=%d, i2=%d, i3=%d\r\n";`

main program follows
```
void main()
{
    eep.registerUserParms( &us, sizeof(us), format );
    
    if( !fetchParms() ) // if EEPROM has bad data, initializes
    {
        eep.initWiFiParms( / Use the defaults / );
        us = us0;
        eep.saveParms();
    }
    eep.updateRebootCount();
    eep.printParms( "Current Parms");
    .....
    // use freely: eep.wifi.ssid, etc
    // use also:   eep.us.i1, etc
    // if you modify, use eep.saveParms()
}
```
ok
- now, jump there <a href="#example">Example headline</a>
- and [this one](<a href="#example2">CODE REF</a>)
- [opem test](file://eepClass.cpp)
- [reference below](below)
- [text below](reftext)
*/
/*
```
*/
#include "eepTable.h"

#define RESPONSE  rbuf.add

// -------------------------------- Common Handlers -----------------------------------------
static void help( int n, char **arg )
{
  COMMAND *row = &etable[0];
  for( int i=0; row->cmd ; i++, row++ )
    RESPONSE( "%s\t%s\r\n", row->cmd, row->help );
}
static int synerr( char *arg0 )
{
  COMMAND *row = &etable[0];
  for( int i=0; row->cmd ; i++, row++ )
  {
    if( strcmp(row->cmd, arg0)==0 )
    {
        RESPONSE( "Use: %s %s\r\n", arg0, row->help );
        return 0;
    }
  }
  RESPONSE("Impossible\r\n");
}
// ------------------------------- minimal CLI: EEPROM ---------------------------------------

static void cliEEInit( int n, char **arg)
{
    eep.initWiFiParms();    // initializes WiFi parametes
    eep.initUserParms();    
    eep.saveParms( ALL_PARM_MASK );
    RESPONSE("%d bytes saved\r\n", HEAD_SIZE+WIFI_SIZE+eep.USER_SIZE );
}

static void cliUpdateWiFi( int n, char **arg)
{
    if( n>1 )
        eep.updateWiFiParms( arg[1], eep.wifi.pwd, eep.wifi.stIP, eep.wifi.port );
    if( n>2 )
        eep.updateWiFiParms( arg[1], arg[2], eep.wifi.stIP, eep.wifi.port );
    if( n>3 )
        eep.updateWiFiParms( arg[1], arg[2], arg[3], eep.wifi.port );
    if( n>4 )    
        eep.updateWiFiParms( arg[1], arg[2], arg[3], atoi( arg[4] ) );
    
    RESPONSE("SSID=%s, PWD=%s, StaticIP=%s, port=%d\r\n", eep.wifi.ssid, eep.wifi.pwd, eep.wifi.stIP, eep.wifi.port );
}
static void cliUpdatePort( int n, char **arg)
{
    eep.fetchParms( WIFIPARM_MASK );
    
    int port = n>1 ? atoi( arg[1] ) : 80;
    eep.updateWiFiParms( NULL, NULL, NULL, port );
    RESPONSE( "Port=%d\r\n", eep.wifi.port );
}
static void cliUpdateCount( int n, char **arg)
{
    eep.fetchParms( HEADPARM_MASK );
    if( n>1 )
    {
        eep.head.reboots = atoi( arg[1] );
        eep.saveParms( HEADPARM_MASK );
    }
    RESPONSE( "Reboot Counter=%d\r\n", eep.head.reboots );
}
static void cliHealth( int n, char **arg)
{
    RESPONSE( "Rebooted %d times\r\n", eep.head.reboots );
    RESPONSE( "Heap used=%d, max=%d\r\n", cpu.heapUsedNow(), cpu.heapUsedMax() );
}

static void cliEEDump( int n, char **arg)     // edump base [N]
{
    int base = (n>1) ? atoi( arg[1] ) : 0;
    int    N = (n>2) ? atoi( arg[2] ) : HEAD_SIZE + WIFI_SIZE + eep.USER_SIZE;

    // prints a list of memory locations in HEX or ASCII.
    EEPROM.begin( HEAD_SIZE + WIFI_SIZE + eep.USER_SIZE );
    int i;
    char c;
    
    for( i=0; i<N; i++ )
    {
        EEPROM.get( base+i, c );
        if( (c>=' ') && (c<0x7f) )
            RESPONSE(" %c ", c );
        else
            RESPONSE( "%02X ", c );
        if( (i%20)==19 )
            RESPONSE("\r\n");
    }
    if( (i%20)!=0)
        RESPONSE("\r\n");
    EEPROM.end();
}
static void cliEEZero( int n, char **arg)     // edump base [N]
{
    // prints a list of memory locations in HEX or ASCII.
    EEPROM.begin( HEAD_SIZE + WIFI_SIZE + eep.USER_SIZE );
    int i, j;
    char c = 0;
    
    j = HEAD_SIZE;
    for( i=0; i<WIFI_SIZE+eep.USER_SIZE; i++ )
        EEPROM.put( j++, c ); 
    RESPONSE("All WiFi and User Parms Zeroed\r\n");
    EEPROM.end();
}

// DIAGNOSTICS OF EEPROM
static void cliEEPut( int n, char **arg)     // eset base value8bit
{
    if( n<=2 )
    {synerr( arg[0] ); return;}
    
    int addr = atoi( arg[1] );
    char patt = atoi( arg[2] );

    EEPROM.begin( HEAD_SIZE+WIFI_SIZE+eep.USER_SIZE );
    EEPROM.put( addr, patt );
    EEPROM.end();  
}    

/*
```
<a id="example2"></a>
```
*/
static void cliPrintParms( int n, char **arg)     // define
{
    int select = (n>1)? atoi(arg[1]) : 0xF;
    RESPONSE( !eep.getParmString( "", (select_t) select ) );
}
static void cliSaveParms( int n, char **arg)     
{
    int select = (n>1)? atoi(arg[1]) : 0xF;
    eep.saveParms( (select_t) select );
}
static void cliFetchParms( int n, char **arg)     
{
    int select = (n>1)? atoi(arg[1]) : 0xF;
    int error = eep.fetchParms( (select_t) select );
    RESPONSE("Fetch error=%d\r\n", error);
}
 
COMMAND etable[]= // must be external to be able to used by the cliSupport
{
    {"e",       "EEPROM parameter help",                                  help     },
    {"estat",   "Shows reboots and heap",                       cliHealth},

// updates
    {"ewifi",   "[ssid] [pwd] [staticIP] [port]. Updates EEPROM WiFi parms",  cliUpdateWiFi },
    {"eport",   "port. Updates EEPROM port. Reboot!",                         cliUpdatePort },
    {"einit",   "Initializes parameters and writes them to EEPROM",             cliEEInit },
    {"ebcount", "[value] prints or sets reboot counter. Writes to EEPROM",      cliUpdateCount },

// changes to structures only
    {"eprint",  "[selection]. Prints all parameters", cliPrintParms },

    {"esave",   "[selection]. Saves current data structures to EEPROM",         cliSaveParms },
    {"efetch",  "[selection]. Fetches EEPROM data structures to structures",    cliFetchParms },

// direct to EEPROM. No changes to structures
    {"edump",   "[base=0] [count=all]. Prints EEPROM contents",    cliEEDump },
    {"ezero",   "Zeros all EEPROM contents",    cliEEZero },
    {"eput",    "base value. Sets an EEPROM location",    cliEEPut },
    {NULL, NULL, NULL}
};
/*
```
*/
