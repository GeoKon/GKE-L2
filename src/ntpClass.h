#ifndef NTPCLASS_H
#define NTPCLASS_H

#include <cpuClass.h>
#include <timeClass.h>              // used only for the mask_t
#include <WiFiUdp.h>

#define NTP_PACKET_SIZE 48         // NTP time stamp is in the first 48 bytes of the message
class NTP
{
private:
    bool state;
    /* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */

    byte packetBuffer[ NTP_PACKET_SIZE];    //buffer to hold incoming and outgoing packets
    uint32_t unixtime;

    int mytime[6];    // hr/min/sec year/month/day
    
public:
    void init();
    void request();
    bool ready();                           // gets the packetBuffer[]
    uint32_t getUnixTime();                 // uses the packetBuffer[]. Sets 'unixtime'. Returns 'epoch'
    int *getDateTime( uint32_t epoch=0 );   // uses 'epoch' or internal 'unixtime'. Returns pointer to 'mytime'
    int getTime( mask_t m );
};
extern NTP ntp;

void initTimeBase( NTP &mytp, TIME &mytt );

#endif
