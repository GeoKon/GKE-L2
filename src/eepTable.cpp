#include <cpuClass.h>
#include <cliClass.h>
#include <eepClass.h>
#include <serClass.h>

// ------------------------------- minimal CLI: EEPROM ---------------------------------------

#define RESPONSE _exe->respond
#define RESPSTR  _exe->respondStr

// namespace eepTable
// { 
	// static EXE *_exe;			// pointer to EXE Class
	// static EEP *_eep;			// pointer to EEPROM parameter structure
	// static SER *_ser;
	
	// void init( EXE &myexe, EEP &myeep, SER &myser )
	// {
		// _exe = &myexe;
		// _eep = &myeep;
		// _ser = &myser;
	// }
	
namespace eepTable
{ 
	static EXE *_exe;			// pointer to EXE Class
	static EEP *_eep;			// pointer to EEPROM parameter structure
	static SER *_ser;
	
	void init( EXE &myexe, EEP &myeep )
	{
		_exe = &myexe;
		_eep = &myeep;
	}
	
	static void cliEEInit( int n, char **arg )
	{
		_eep->initHeadParms();  
		_eep->initWiFiParms();    		// initializes ALL structures and writes them to EEPROM
		RESPSTR("OK\r\n");
	}
	static void cliShowParms( int n, char **arg )
	{
		RESPSTR( _eep->getHeadString().c_str() );
		RESPSTR( _eep->getWiFiString().c_str() );
	}	
	static void cliFetchParms( int n, char **arg )
	{
		_eep->fetchHeadParms();			
		_eep->fetchWiFiParms();			
		cliShowParms( 0, NULL );
	}	
	
	static void cliUpdateWiFi( int n, char **arg )
	{
		_eep->fetchWiFiParms();
		
		if( n>1 )
			_eep->initWiFiParms( arg[1], _eep->wifi.pwd, _eep->wifi.stIP, _eep->wifi.port );
		if( n>2 )
			_eep->initWiFiParms( arg[1], arg[2], _eep->wifi.stIP, _eep->wifi.port );
		if( n>3 )
			_eep->initWiFiParms( arg[1], arg[2], arg[3], _eep->wifi.port );
		if( n>4 )    
			_eep->initWiFiParms( arg[1], arg[2], arg[3], atoi( arg[4] ) );
		
		_eep->saveWiFiParms();
		RESPSTR( _eep->getWiFiString().c_str() );
	}
	static void cliUpdatePort( int n, char **arg )
	{
		_eep->fetchWiFiParms();
		
		int port = n>1 ? atoi( arg[1] ) : 80;
		_eep->initWiFiParms( NULL, NULL, NULL, port );
		RESPONSE( "Port=%d\r\n", _eep->wifi.port );
	}
	static void cliUpdateCount( int n, char **arg )
	{
		_eep->fetchHeadParms();
		if( n>1 )
		{
			_eep->head.reboots = atoi( arg[1] );
			_eep->saveHeadParms();
		}
		RESPONSE( "Reboot Counter=%d\r\n", _eep->head.reboots );
	}
	static void cliStatus( int n, char **arg )
	{
		RESPONSE( "Rebooted %d times\r\n", _eep->head.reboots );
//		RESPONSE( "Heap used=%d, max=%d\r\n", cpu.heapUsedNow(), cpu.heapUsedMax() );
	}
	// static void cliPrintUserParms( int n, char **arg )     // define
	// {
		// _eep->fetchUserParms();
		// RESPSTR( _eep->getUserString().c_str() );
	// }
	// static void cliSetUserParm( int n, char **arg )
	// {
		// int e;
		// _eep->fetchUserParms();
		// if( n<3 )
		// {
			// RESPSTR("Missing <name> <fvalue>\r\n");
			// return;
		// }
		// int type = _eep->getUserParmType( arg[1] );

		// if( type == 'f' )
		// {
			// float f = atof( arg[2] );
			// int e = _eep->setUserParmValue( arg[1], &f );
			// if( !e )
				// RESPONSE( "%s set to %f\r\n", arg[1], f );
		// }
		// else if( type == 'd' )
		// {
			// int iv = atoi( arg[2] );
			// int e = _eep->setUserParmValue( arg[1], &iv );
			// if( !e )
				// RESPONSE( "%s set to %f\r\n", arg[1], iv );
		// }
		// else if( type == 's' )
		// {
			// int e = _eep->setUserParmValue( arg[1], arg[2] );
			// if( !e )
				// RESPONSE( "%s set to %s\r\n", arg[1], arg[2] );
		// }
		// else
		// {
			// RESPONSE( "Invalid type %d\r\n", type );
			// return;
		// }
		// if( e )
			// RESPONSE( "Cannot set %s to %s (error:%d)\r\n", e, arg[1], arg[2], e );
		// else
			// _eep->saveUserParms();
	// }

	static void help( int n, char **arg )     
	{
		RESPONSE("EEPROM commands\r\n");
	}
	
// -------------------------- DIAGNOSTICS ---------------------------------------------------
#if DIAGNOSTICS
	static void cliEEDump( int n, char **arg )     // edump base [N]
	{
		int base = (n>1) ? atoi( arg[1] ) : 0;
		int    N = (n>2) ? atoi( arg[2] ) : HEAD_PSIZE + WIFI_PSIZE + _eep->USER_PSIZE;

		// prints a list of memory locations in HEX or ASCII.
		EEPROM.begin( HEAD_PSIZE + WIFI_PSIZE + _eep->USER_PSIZE );
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
		EEPROM.begin( HEAD_PSIZE + WIFI_PSIZE + _eep->USER_PSIZE );
		int i, j;
		char c = 0;
		
		j = HEAD_PSIZE;
		for( i=0; i<WIFI_PSIZE+_eep->USER_PSIZE; i++ )
			EEPROM.put( j++, c ); 
		RESPONSE("All WiFi and User Parms Zeroed\r\n");
		EEPROM.end();
	}

	static void synerr( char *name, int n )
	{
		RESPONSE( "%s requires at least %d arguments", name, n);
	}

	static void cliEEPut( int n, char **arg )     // eset base value8bit
	{
		if( n<=2 )
		{synerr( arg[0], 2 ); return;}
		
		int addr = atoi( arg[1] );
		char patt = atoi( arg[2] );

		EEPROM.begin( HEAD_PSIZE+WIFI_PSIZE+_eep->USER_PSIZE );
		EEPROM.put( addr, patt );
		EEPROM.end();  
	}    
#endif

	CMDTABLE table[]= // must be external to be able to used by the cliSupport
	{
		{"e",       "--- EEPROM CLI Commands ---",                  			help     },
//		{"estat",   "Shows number reboots and heap",                       		cliStatus},

	// updates
		{"eshow",   "Shows EEPROM (header and WiFi)",  		cliShowParms  },
		{"einit",   "Initializes default Header and WiFi EEPROM parameters",        cliEEInit },
		{"ewifi",   "[ssid] [pwd] [staticIP] [port]. Updates EEPROM WiFi parms",  	cliUpdateWiFi },
		{"eport",   "port. Updates EEPROM port. Reboot!",                         	cliUpdatePort },
		{"efetch",  "Fetches and displays EEPROM",  		cliFetchParms },

		
		{"ecount",  "[value] prints or sets reboot counter",      cliUpdateCount },
		

	// changes to structures only
	//	{"uprint",  "Prints user parameters", 										cliPrintUserParms },
	//	{"uset",    "<name> <value>. Sets user parameter", 							cliSetUserParm },
#if DIAGNOSTICS 
	// direct to EEPROM. No changes to structures
		{"edump",   "[base=0] [count=all]. Prints EEPROM contents",    cliEEDump },
		{"ezero",   "Zeros all EEPROM contents",    cliEEZero },
		{"eput",    "base value. Sets an EEPROM location",    cliEEPut },
#endif
		{NULL, NULL, NULL}
	};

}

