#include "macros.h"
#include "ds18Class.h"
#include <OneWire.h>

int DS18::search( bool debug )
{
    ds->reset_search();
    found = 0;
    for( int id=0; id<DS18MAX_SENSORS; id++ )
    {
        byte *addr = (byte*) &list[id];
        
        // Check for device present
        if ( !ds->search(addr) )
            break;
                              
        // Check is this device has the right CRC
        if( OneWire::crc8(addr, 7) != addr[7] ) 
        {
            if( debug )
                PF( "Dev%d bad CRC\r\n", id);
            break;
        }
        
        // check if DS18B20
        int type = *addr;
        if(  !(type == 0x10 // DS18S20
            || type == 0x28 // DS18B20
            || type == 0x22 // DS1822
            || type == 0x3B // DS1825 
            ) )
        {
            if( debug )
                PF("Dev:%d unsupported type:%02x\r\n", id, type );
            break;
        }
        if( debug )
        {
            PF("SENSOR:%d TYPE:%02X ID:", id, type );
            for( int i = 0; i < 8; i++) 
                PF("%02X", addr[i] );
            PF("\r\n");
        }
        found++;
        delay(750 );
    }
	simulON = false;
    state   = 0;
    idx     = 0;
    if( debug )
		PF("%d devices found\r\n", found );
	return found;
}

void DS18::start( int devid )  
{
    if( (state !=0) || (found <= 0) )             // do nothing if state is not zero
        return;
            
    idx = devid;
    err = 0;					// clear all errors
	
    if( idx >= found )          // just in case there is an error somewhere
        idx = 0;

    if( !simulON )				// do nothing if we are in simulation mode
	{
		byte *addr = (byte *) &list[ idx ];		

		ds->reset();
		ds->select( addr );
		ds->write(0x44, 1);     // 0x44 is STARTCONVO
	}
	T0 = millis();      		// get timestamp
    state = 1;
}

bool DS18::ready()
{
    if( (state!=1) || (millis()-T0 < 750) )		// timeout not elapsed yet
		return false;

	T0 = millis();				// reset the time because simulON takes less than 1ms!
	if( simulON )
	{
		tempC 	= simTemp;		// error remains 0, as set by start()
		state 	= 0;
		return true;
	}
	byte *addr = (byte *) &list[ idx ];
	byte data[12];
		
	if( !ds->reset() )
	{
		tempC = INVALID_TEMP;   // invalid reading in case of an error
		err = DS18ERR_NO_SENSOR;
	}
	else
	{
		// Scratchpad locations
		//    #define TEMP_LSB        0
		//    #define TEMP_MSB        1
		//    #define HIGH_ALARM_TEMP 2
		//    #define LOW_ALARM_TEMP  3
		//    #define CONFIGURATION   4
		//    #define INTERNAL_BYTE   5
		//    #define COUNT_REMAIN    6
		//    #define COUNT_PER_C     7
		//    #define SCRATCHPAD_CRC  8

		ds->select(addr); 
		ds->write (0xBE);                // 0xBE is READSCRATCH
		 
		for ( int i = 0; i < 9; i++) 
			data[i] = ds->read();
	 
		if( OneWire::crc8(data, 8) != data[8] )
		{
			tempC = INVALID_TEMP;   // invalid reading in case of an error
			err = DS18ERR_BAD_CRC;
		}
		else
		{
			int16_t raw = (data[1] << 8) | data[0];     // TEMP LSB and MSB

			byte type_s = *addr;
			// PF("TYPE=%02X CFG=%02X\r\n", type_s, data[4] & 0x60);
			
			if (type_s==0x10)                           // only applicable to DS18S20
			{
				raw = raw << 3; 
				if (data[7] == 0x10) 
					raw = (raw & 0xFFF0) + 12 - data[6];
			} 
			else 
			{
				byte cfg = (data[4] & 0x60);            // check CONFIGURATION [4]
				
				if (cfg == 0x00) raw = raw & ~7;        // 9 bit resolution, 93.75 ms
				else if (cfg == 0x20) raw = raw & ~3;   // 10 bit res, 187.5 ms
				else if (cfg == 0x40) raw = raw & ~1;   // 11 bit res, 375 ms
			}
			tempC = (float)raw / 16.0;
			err = 0;
		}					// CRC is OK
	}						// Sensor is present
	state = 0;				// ready to re-start()
	return true;
}
bool DS18::success( bool debug )
{
	if( !err )
		return true;
	if( debug )
		switch( err )
		{
			case DS18ERR_NO_SENSOR: 
				PF("DS18 missing sensor ID:%d\r\n", idx ); 
				break;
			case DS18ERR_BAD_CRC:
				PF("DS18 bad CRC of ID:%d\r\n", idx ); 
				break;
			default:
				PF("DS18 unspecified error %d\r\n", err ); 
				break;
		}
	return false;
}
void DS18::simulC( float C )
{
    simulON = true;
    simTemp = C;
}
void DS18::simulF( float F )
{
    simulON = true;
    simTemp = (F-32.0)*5.0/9.0;
}
void DS18::simulOff()
{
    simulON = false;
    simTemp = INVALID_TEMP;
}
