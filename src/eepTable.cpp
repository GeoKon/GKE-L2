#include <cliClass.h>
#include "eepClass.h"

// ------------------------------- minimal CLI: EEPROM ---------------------------------------

#define RESPONSE _exe->respond

namespace eepTable
{ 
	static EXE *_exe;			// pointer to EXE Class
	static EEP *_eep;			// pointer to EEPROM parameter structure
	
	void init( EXE &myexe, EEP &myeep )
	{
		_exe = &myexe;
		_eep = &myeep;
	}
	
	static void cliEEInit( int n, char **arg )
	{
		_eep->initWiFiParms();    // initializes WiFi parametes
		_eep->initUserParms();    
		_eep->saveParms( ALL_PARMS );
		RESPONSE("%d bytes saved\r\n", HEAD_SIZE+WIFI_SIZE+_eep->USER_SIZE );
	}
	static void cliUpdateWiFi( int n, char **arg )
	{
		if( n>1 )
			_eep->updateWiFiParms( arg[1], _eep->wifi.pwd, _eep->wifi.stIP, _eep->wifi.port );
		if( n>2 )
			_eep->updateWiFiParms( arg[1], arg[2], _eep->wifi.stIP, _eep->wifi.port );
		if( n>3 )
			_eep->updateWiFiParms( arg[1], arg[2], arg[3], _eep->wifi.port );
		if( n>4 )    
			_eep->updateWiFiParms( arg[1], arg[2], arg[3], atoi( arg[4] ) );
		
		RESPONSE("SSID=%s, PWD=%s, StaticIP=%s, port=%d\r\n", _eep->wifi.ssid, _eep->wifi.pwd, _eep->wifi.stIP, _eep->wifi.port );
	}
	static void cliUpdatePort( int n, char **arg )
	{
		_eep->fetchParms( WIFI_PARMS );
		
		int port = n>1 ? atoi( arg[1] ) : 80;
		_eep->updateWiFiParms( NULL, NULL, NULL, port );
		RESPONSE( "Port=%d\r\n", _eep->wifi.port );
	}
	static void cliUpdateCount( int n, char **arg )
	{
		_eep->fetchParms( HEAD_PARMS );
		if( n>1 )
		{
			_eep->head.reboots = atoi( arg[1] );
			_eep->saveParms( HEAD_PARMS );
		}
		RESPONSE( "Reboot Counter=%d\r\n", _eep->head.reboots );
	}
	static void cliHealth( int n, char **arg )
	{
		RESPONSE( "Rebooted %d times\r\n", _eep->head.reboots );
		//RESPONSE( "Heap used=%d, max=%d\r\n", cpu.heapUsedNow(), cpu.heapUsedMax() );
	}

	static void cliEEDump( int n, char **arg )     // edump base [N]
	{
		int base = (n>1) ? atoi( arg[1] ) : 0;
		int    N = (n>2) ? atoi( arg[2] ) : HEAD_SIZE + WIFI_SIZE + _eep->USER_SIZE;

		// prints a list of memory locations in HEX or ASCII.
		EEPROM.begin( HEAD_SIZE + WIFI_SIZE + _eep->USER_SIZE );
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
	static void cliEEZero( int n, char **arg )     // edump base [N]
	{
		// prints a list of memory locations in HEX or ASCII.
		EEPROM.begin( HEAD_SIZE + WIFI_SIZE + _eep->USER_SIZE );
		int i, j;
		char c = 0;
		
		j = HEAD_SIZE;
		for( i=0; i<WIFI_SIZE+_eep->USER_SIZE; i++ )
			EEPROM.put( j++, c ); 
		RESPONSE("All WiFi and User Parms Zeroed\r\n");
		EEPROM.end();
	}

	static void synerr( char *name, int n )
	{
		RESPONSE( "%s requires at least %d arguments", name, n);
	}
	// DIAGNOSTICS OF EEPROM
	static void cliEEPut( int n, char **arg )     // eset base value8bit
	{
		if( n<=2 )
		{synerr( arg[0], 2 ); return;}
		
		int addr = atoi( arg[1] );
		char patt = atoi( arg[2] );

		EEPROM.begin( HEAD_SIZE+WIFI_SIZE+_eep->USER_SIZE );
		EEPROM.put( addr, patt );
		EEPROM.end();  
	}    
	static void cliPrintParms( int n, char **arg )     // define
	{
		int select = (n>1)? atoi(arg[1]) : 0xF;
		RESPONSE( !_eep->getParmString( "", (select_t) select ) );
	}
	static void cliSaveParms( int n, char **arg )     
	{
		int select = (n>1)? atoi(arg[1]) : 0xF;
		_eep->saveParms( (select_t) select );
	}
	static void cliFetchParms( int n, char **arg )     
	{
		int select = (n>1)? atoi(arg[1]) : 0xF;
		int error = _eep->fetchParms( (select_t) select );
		RESPONSE("Fetch error=%d\r\n", error);
	}

	static void help( int n, char **arg )     
	{
		RESPONSE("EEPROM commands\r\n");
	}

	CMDTABLE table[]= // must be external to be able to used by the cliSupport
	{
		{"e",       "--- EEPROM CLI Commands ---",                  help     },
		{"estat",   "Shows reboots and heap",                       cliHealth},

	// updates
		{"ewifi",   "[ssid] [pwd] [staticIP] [port]. Updates EEPROM WiFi parms",  cliUpdateWiFi },
		{"eport",   "port. Updates EEPROM port. Reboot!",                         cliUpdatePort },
		{"einit",   "Initializes parameters and writes them to EEPROM",             cliEEInit },
		{"bcount",  "[value] prints or sets reboot counter. Writes to EEPROM",      cliUpdateCount },

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
}

