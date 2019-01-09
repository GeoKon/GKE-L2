//#define EXAMPLE1
//#define EXAMPLE2
  #define EXAMPLE3

#include <webSupport.h>     // needed for initWiFi()
#include <timeClass.h>
#include "ntpClass.h"
#include <Ticker.h>         // needed for the time()

char ssid[] = "kontopidis2GHz";  //  your network SSID (name)
char pass[] = "123456789a";       // your network password

// A UDP instance to let us send and receive packets over UDP

// ----------------------- allocation of base classes -----------------------------
    Ticker tk;
    CPU cpu;
    TIME tm;
    NTP ntp;
    WiFiUDP udp;

#ifdef EXAMPLE1 //------------------------------------------------------------------

void setup() 
{
    cpu.init();
    initWiFi( ssid, pass );

    PR("Starting UDP");
    ntp.init();
    ntp.request();      // get time immediately
}
ModuloTic tic( 10 );    // N-sec tic
void loop()
{
    if( tic.ready() )   // get time on a N-sec tic
        ntp.request();
        
    if( ntp.ready() )
    {
        ntp.getUnixTime();
        int *p=ntp.getDateTime();
        PF( "HH:MM:SS = %02d:%02d:%02d  YYYY/MM/DD =%d/%02d/%02d\r\n", *p++, *p++, *p++, *p++, *p++, *p++);
    }
    delay( 100 );
}
#endif

#ifdef EXAMPLE2 //------------------------------------------------------------------

void setup() 
{
    cpu.init();
    initWiFi( ssid, pass );

    tm.init();  // start the timer
    tm.setTime( 0,0,0 );
    tm.setDate( 2018,11,10 );
    tk.attach( 1.0, [](){ tm.update(); } );
    
    PF("Time is: %s %s\r\n", !tm.getTimeString(), !tm.getDateVerbose() );
    
    PR("Starting UDP");
    ntp.init();
    ntp.request();      // get time immediately

    while( !ntp.ready() )
    {
        PN(".");
        delay( 500 );
    }
    PR("");
    ntp.getUnixTime();
    int *p=ntp.getDateTime();
    PF( "HH:MM:SS = %02d:%02d:%02d  MM/DD/YYYY =%d/%02d/%02d\r\n", 
        ntp.getTime( HOUR_MASK ),
        ntp.getTime( MINUTE_MASK ),
        ntp.getTime( SECOND_MASK ),
        ntp.getTime( MONTH_MASK ),
        ntp.getTime( DAY_MASK ),
        ntp.getTime( YEAR_MASK ) );

    tm.setTime( ntp.getTime( HOUR_MASK ),
                ntp.getTime( MINUTE_MASK ),
                ntp.getTime( SECOND_MASK ) );

    tm.setDate( ntp.getTime( YEAR_MASK ),
                ntp.getTime( MONTH_MASK ),
                ntp.getTime( DAY_MASK ) );
}
 
ModuloTic tic( 10 );    // N-sec tic
void loop()
{
    if( tic.ready() )   // get time on a N-sec tic
        PF("Time is: %s %s\r\n", !tm.getTimeString(), !tm.getDateVerbose() );
}
#endif

#ifdef EXAMPLE3 //------------------------------------------------------------------
void setup() 
{
    cpu.init();
    initWiFi( ssid, pass );
    initTimeBase( ntp/*Allocated in ntpClass*/, tm/*Allocated in timeClass*/ );
}
ModuloTic tic( 10 );    // N-sec tic
void loop()
{
    if( tic.ready() )   // get time on a N-sec tic
        PF("Time is: %s %s\r\n", !tm.getTimeString(), !tm.getDateVerbose() );
}
#endif
