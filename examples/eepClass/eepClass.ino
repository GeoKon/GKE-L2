#define TEST3

#include <ESP8266WiFi.h>
#include <nodeClass.h>
#include "eepClass.h"
#include <FS.h>

#ifdef TEST1
void setup()
{
	cpu.init();
	
	eep.ensureValidParms();			    // fetch current parms
	eep.printParms("Current Parms");	
	
	char *p = cpu.prompt("Do you want to initialize all?");
	if( atoi(p) )
		eep.initParms( "noSSID", "noPWD" );
		
	strcpy( eep.parm.ssid, cpu.prompt("Enter new SSID: ") );
	
	eep.printParms("See new parms");
	eep.saveParms();			        // save parms
	
	cpu.die( "System halted", 3 );
}
void loop() 
{
  delay(100);
}
#endif

#ifdef TEST2
#include <cliClass.h>
#include "eepTable.h"
void setup()
{
    cpu.init();
        
    eep.ensureValidParms();             // fetch current parms
    eep.printParms("Current Parms");    
    
    buf.init();                         // initialize response buffer
    cli.init( ECHO_ON, "cmd: " );
    cli.prompt(); 
}
void loop() 
{
  bool e;
  if( cli.ready() )
  {
    e = cli.exec( &etable[0] );
    if( !e )
      buf.str.add( "Cmnd %s not found\r\n", *cli.args() );
    
    cpu.printf( "%s", buf.c_str() );        // print buffer
    buf.init();                             // reset the buffer
    cli.prompt();
  }
  delay(10);
}
#endif

#ifdef TEST3
#include <cliClass.h>

#include "eepTable.h"
struct myparm_t
{
    float x;
    int n;
    char test[16];
    int i;
} myeep;

void mydflt()   // initialization callback
{
    myeep.x = 1.25;
    myeep.n = 10;
    myeep.i = 20;
    strcpy( &myeep.test[0], "test");
}

void setup()
{
    cpu.init();
    SPIFFS.begin();                     // crashes sometimes if not present
    
    eep.defineUserParms( &myeep, "%f %d %s %d", mydflt );
    
    eep.ensureValidParms();                  // use whatever EEPROM has
    eep.printParms("Current Parms");    
    
    buf.init();                         // initialize response buffer
    cli.init( ECHO_ON, "cmd: " );
    cli.prompt(); 
}
void loop() 
{
  bool e;
  if( cli.ready() )
  {
    e = cli.exec( &etable[0] );
    if( !e )
      buf.str.add( "Cmnd %s not found\r\n", *cli.args() );
    
    cpu.printf( "%s", buf.c_str() );        // print buffer
    buf.init();                             // reset the buffer
    cli.prompt();
  }
  delay(10);
}
#endif
