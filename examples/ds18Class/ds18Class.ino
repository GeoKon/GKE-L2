//#define TEST1       // simplest ONE sensor
//#define TEST2       // explicit error check
//#define TEST3       // 2-sensors
#define TEST4       // simulation toggle

#include <cpuClass.h>
//#include <timeClass.h>
#include <ticClass.h>

#include <OneWire.h>
#include <ds18Class.h>

CPU cpu;
//ModuloTic tic(2);       // one tic per second
TICsec      tic(2);       // one tic per second
    
OneWire  ds(4);         // on pin D2 of the NodeMCU. Use 10k Pull-up!
DS18 temp( &ds );       // associate DS18B20 class with OneWire

#ifdef TEST1
void setup() 
{
    cpu.init();
    temp.search( true );               // look for 3-devices with debug enabled
}
void loop() 
{
    if( tic.ready() )
        temp.start( 0 );                // start with 1st sensor

    if( temp.ready() && temp.success(true) )
        PF( "Temp is %5.1fC or %5.1fF\r\n", temp.getDegreesC(), temp.getDegreesF() );
}
#endif

#ifdef TEST2
void setup() 
{
    cpu.init();
    temp.search( true );               // look for 3-devices with debug enabled
}
void loop() 
{
    if( tic.ready() )
        temp.start( 0 );                // start with 1st sensor

    if( temp.ready() )
    {
        if( temp.success() )
            PF( "Temp is %5.1fC or %5.1fF\r\n", temp.getDegreesC(), temp.getDegreesF() );
        else
            PF( "Error %d\r\n", temp.error() );
    }
}
#endif

#ifdef TEST3
void setup() 
{
    cpu.init();
    temp.search( true );               // look for 3-devices with debug enabled
}
void loop() 
{
    static int id = 0;
    
    if( tic.ready() )
        temp.start( id );                // start with 1st sensor

    if( temp.ready() )
    {
        if( temp.success(true) )         
            PF( "Temp of %d is %5.1fC or %5.1fF\r\n", id, temp.getDegreesC(), temp.getDegreesF() );
        
        id = temp.nextID();             // regardless if success or not, increment id
    }
}
#endif

#ifdef TEST4
void setup() 
{
    cpu.init();
    temp.search( true );               // look for 3-devices with debug enabled
    
    PRN("Press button to toggle simulation");
}
void loop() 
{
    if( tic.ready() )
        temp.start( 0 );                // start with 1st sensor

    if( cpu.buttonPressed() )
    {
        static bool toggle = true;
        if( toggle ) 
        {
            temp.simulF( 100.0 );
            PRN("Simulation ON");
        }
        else  
        {
            temp.simulOff();
            PRN("Simulation OFF");
        }
        toggle = !toggle;
    }
    if( temp.ready() )
    {
        temp.success(true);
        PF( "Temp is %5.1fF\r\n", temp.getDegreesF() );
    }
}
#endif

// ---- this is not debugged yet! ---------------
#ifdef TEST5
FLT filter(3);

void setup() 
{
    cpu.init();
    temp.init( 3, true );
    
    char *p = cpu.prompt("Enter coef: ");
    filter.setCoef( atof( p ) );        
    temp.simulate( true, 0.0 );
}
void loop() 
{
    if( tic.ready() )
        temp.start( 0 );   // specify index or use thisID()

    if( cpu.buttonPressed() )
        temp.simulate( true, 100.0 );
        
    if( temp.ready() )
    {
        temp.read();
        filter.smooth( 0, temp.getDegreesC() );
        
        PF("Temp=%5.1f°C\tsmoothed=%5.1f°C\r\n", temp.getDegreesC(), filter.getValue(0) );
    }
}
#endif
