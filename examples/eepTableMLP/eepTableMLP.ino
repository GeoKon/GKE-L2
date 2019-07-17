/*                SELECTION OF TESTS 
 * 
 * 1 = interactive blocking demo CLI in setup() 
 * 2 = typical boot with WiFi with no user EEPROM parms
 * Use the eepGlobals.ino to test user+eep parameters
*/
//#include <cpuClass.h> 
#include <cliClass.h>       
#include <eepClass.h>   
#include "eepTable.h"
#include "MegunoLink.h"
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
// -----------------------------------------------------------------------------
struct parms_t
{
    int lastsent = 0;
    int progress = 0;
    int i1 = 10;
    int i2 = 20;
    int trace = 0;
    char str[20]="TEST";
    int v = 0;
} myp;

void setup()
{
    cpu.init( 4800  );

    exe.registerTable( mypTable::table );

    myp.progress = 0;
    myp.lastsent = millis();
    
    //interactForever();
    cli.init( ECHO_OFF, "cmd: ", /*flush*/ true );       
    PR("CLI Mode. Press 'h' for help\r\n");
    cli.prompt();
}
class MUI
{
	public:
	void setValue( char *element, int value )
	{
	    PF("{UI|SET|%s.Value=%d}\r\n", element, value );
	}
	void setValue( char *element, float value )
	{
	    PF("{UI|SET|%s.Value=%5.3f}\r\n", element, value );
	}
	void setValue( char *element, bool value )
	{
	    PF("{UI|SET|%s.Checked=%s}\r\n", element, value?"True":"False" );
	}
	void setText( char *element, char *value )
	{
	    PF("{UI|SET|%s.Text=%s}\r\n", element, value );
	}
	void setText( char *element, int iv )
	{
	    char temp[20];
	    sprintf( temp, "%d", iv );
	    PF("{UI|SET|%s.Text=%s}\r\n", element, temp );
	}


};
MUI mui;

void loop() 
{
  if( cli.ready() )
  {
    exe.dispatchConsole( cli.gets() );
    //cli.prompt();
  }

  if ((millis() - myp.lastsent) > 10000)
  {
    myp.lastsent = millis();

    mui.setValue("Progress", myp.progress); 
    mui.setValue("Current", myp.progress);

    if (myp.progress == 0)
      mui.setText("Status", "Start");
    else if (myp.progress == 8)
      mui.setText("Status", "Nearly there");
    else if (myp.progress == 9)
      mui.setText("Status", "End");
    else
      mui.setText("Status", "Working");

    myp.progress = (myp.progress + 1) % 10;
  }
}

// ----------------------------- local CLI tables ---------------------------
namespace mypTable
{
    void help( int n, char *arg[] ) {exe.help( n, arg );}

    void cliInitValues( int n, char **arg )
    {
      	mui.setValue("i1", myp.i1 );
      	mui.setValue("i2", myp.i2 );
      	mui.setText("str", myp.str );
      	mui.setText("trace", myp.trace );
    }
    void cliSetIValues( int n, char **arg )
    {
      	if( n>2 )
      	{
      		char *sv = arg[1];
      		int vl = atoi( arg[2] );
      		if( strcmp("i1", sv )==0 )
      			myp.i1 = vl;
      		if( strcmp("i2", sv )==0 )
      			myp.i2 = vl;
      	}
    }
    void cliSetTrace( int n, char **arg )
    {
        if( n> 1)
            myp.trace = atoi( arg[1] );
        mui.setText( "trace", myp.trace );       
    }
    void cliTest( int n, char **arg )
    {
        if( n> 1)
            myp.v = atoi( arg[1] );
        PF( "Test = %d\r\n", myp.v );        
    }
    void cliLED( int n, char **arg )
    {
        static bool flag = false;
        if( flag )
        {
            cpu.led( ON );
            mui.setValue( "bLedOnOff", "Flip LED OFF" );
            mui.setValue( "bLed", true );
        }
        else
        {
            cpu.led( OFF );
            mui.setValue( "bLedOnOff", "Flip LED ON" );
            mui.setValue( "bLed", false );
        }        
        flag = !flag;
    }
    CMDTABLE table[]= 
    {
        {"h", "Help! List of all commands", help },
        {"test", "num. Test", cliTest },
        
        {"!ToggleLED", "flip LED", cliLED },
        {"!init", "initialize variables", cliInitValues },
        {"!seti", "varname value", cliSetIValues },
        {"!trace", "value", cliSetTrace },
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
