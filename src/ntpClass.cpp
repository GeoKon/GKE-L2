//#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
//#include "cpuClass.h"
#include "ntpClass.h"

extern WiFiUDP udp;

// -------------------- class definition -----------------------
void NTP::init()
{
    unsigned int localPort = 2390;      // local port to listen for UDP packets

    udp.begin(localPort);
    PR( "Local port: "); PRN( udp.localPort());

    state = false;      // request not pending
}
void NTP::request()
{
    if( state )             // must not have a request pending!
        return;
        
    const char* ntpServerName = "time.nist.gov";
    //IPAddress timeServer(129, 6, 15, 28);         // time.nist.gov NTP server
    IPAddress timeServerIP;                         // time.nist.gov NTP server address
    
    //get a random server from the pool
    WiFi.hostByName(ntpServerName, timeServerIP);
    
    PRN("Sending NTP packet...");
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock PRNecision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    
    udp.beginPacket( timeServerIP, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
    
    state = true;
}

bool NTP::ready()
{
    if( !state )                        // request not sent yet
        return false;
        
    int cb = udp.parsePacket();         // request sent but packet not received
    if (!cb) 
    {
        //PRN("no packet yet");
        return false;
    }
    else 
    {
        //PR("packet received, length=");PRN(cb);
        // We've received a packet, read the data from it

        udp.read(packetBuffer, NTP_PACKET_SIZE);    // read the packet into the buffer
        state = false;                              // all done waiting
        return true;
    }
}
uint32_t NTP::getUnixTime()
{
        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, esxtract the two words:

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;

        //PF("Seconds since Jan 1 1900 = %d\r\n", secsSince1900);

        // now convert NTP time into everyday time:
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:
        unsigned long epoch = secsSince1900 - seventyYears;
        // PRNint Unix time:
        // PF("Unix time = %d\r\n", epoch);

        return unixtime = epoch;
}
// ----------------------- used to convert epoch to date ------------------------

    static bool leapYear( int y )
    {
      return ( (y%4 == false) && ((y%100 != false) || (y%400 == false)) );
    }
    static int daysInMonth( int y, int i )  // modifies February
    {
        static int days_in_month[]={0,31,28,31,30,31,30,31,31,30,31,30,31};  
        if( (i<1) || (i>12) )
            return 0;
        if( (i==2) && leapYear( y ) )
            return days_in_month[2]+1;
        return days_in_month[i];
    }
    
// ------------------------------------------------------------------------------
int *NTP::getDateTime( uint32_t epoch )
{
    
    
    if( epoch == 0 )        // use the class unixtime
        epoch = unixtime;
        
    int hour = (epoch  % 86400L) / 3600; hour = (hour+24-5)%24; // EST
    int minu = (epoch % 3600) / 60;
    int sec  = (epoch % 60);
    
    //PF("------------- TIME: %02d:%02d:%02d\r\n", hour, minu, sec );
    
    int day     = epoch % 86400L;
    int year    = epoch / 31556926L;                // number of years since 1970
    year += 1970;                                   // current year
    
    int yearSec = (epoch % 31556926L);              // seconds in this year
    int month   = yearSec/2629743L + 1;             // month this year 1...12
    int dayYear = yearSec/86400L + 1;               // day since beginning of this year 1...365

    for( int j=1, yday=0; j<=12; j++ )
    {
        yday += daysInMonth( year, j );                   // sum of days, one month at a time
        if( dayYear < yday )
        {
            yday -= daysInMonth( year, j);
            day = dayYear-yday-1;
            break;
        }
    }        
    // PF( "------------- DATE: %d/%d/%d\r\n", month, day, year );

    mytime[0] = hour;
    mytime[1] = minu;
    mytime[2] = sec;
    mytime[3] = year;
    mytime[4] = month;
    mytime[5] = day;
    return &mytime[0];
}
int NTP::getTime( mask_t m )
{
  switch( m )
  {
    case SECOND_MASK: return mytime[2];
    case MINUTE_MASK: return mytime[1];
    case HOUR_MASK:   return mytime[0];
    case DAY_MASK:    return mytime[5];
    case MONTH_MASK:  return mytime[4];
    default:
    case WDAY_MASK: 
    case YEAR_MASK:   return mytime[3];
  }
}

// this routine REQUIRES the ntp. and tm. to be defined
#include <Ticker.h>         // needed for the time()
Ticker mytk;
void initTimeBase( NTP &mytp, TIME &mytt )
{    
    mytp.init();
    mytp.request();                          // request NTP time
    while( !mytp.ready() )
    {
        PR(".");
        delay( 500 );
    }
    PRN("");
    mytp.getUnixTime();
    int *p=mytp.getDateTime();
    PF( "NTP Time %02d:%02d:%02d %2d/%2d/%d\r\n", 
        mytp.getTime( HOUR_MASK ),
        mytp.getTime( MINUTE_MASK ),
        mytp.getTime( SECOND_MASK ),
        mytp.getTime( MONTH_MASK ),
        mytp.getTime( DAY_MASK ),
        mytp.getTime( YEAR_MASK ) );

    mytt.init();                              // start the timebase
    mytt.setTime( mytp.getTime( HOUR_MASK ),
                mytp.getTime( MINUTE_MASK ),
                mytp.getTime( SECOND_MASK ) );

    mytt.setDate( mytp.getTime( YEAR_MASK ),
                mytp.getTime( MONTH_MASK ),
                mytp.getTime( DAY_MASK ) );

    mytk.attach( 1.0, [](){ tm.update(); } );
    PF("Timebase: %s %s\r\n", !mytt.getTimeString(), !mytt.getDateVerbose() );
}

//int findDayCode(int year1)
//{
//  int daycode;
//  int d1, d2, d3;
//  
//  d1 = (year1 - 1.)/ 4.0;
//  d2 = (year1 - 1.)/ 100.;
//  d3 = (year1 - 1.)/ 400.;
//  daycode = (year1 + d1 - d2 + d3) %7;
//  return daycode;
//}
