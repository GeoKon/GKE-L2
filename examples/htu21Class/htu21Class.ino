#include "cpuClass.h"
#include "ticClass.h"
#include "htu21Class.h"

HTU21 htu;
CPU cpu;

void setup() 
{
  cpu.init();
  Serial.println("HTU21D test");

  for( int i=0; !htu.init(); i++)
  {
    PFN("Couldn't find sensor! %d", i );
    delay( 1000 );
  }
}

TICms tic(500);
void loop() 
{
    if( tic.ready() )
    {
        float temp = htu.readTemperature();
        float rel_hum = htu.readHumidity();
    
        PFN("Temp: %.1fC\t\tHumidity: %.0f%%", temp, rel_hum);
    }
}
