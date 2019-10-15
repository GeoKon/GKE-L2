#include <oledClass.h>
#include "pzmClass.h"
extern OLED oled;

//#ifndef pzem_allocated		// class allocation, if not allready done so.
//	PzemClass pzem;
//#endif
//#ifndef pzm4_allocated        // class allocation, if not allready done so.
//    Pzm4Class pzm4;
//#endif
static char cmdTable[5][7] =
{
	{0xB0, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1A},
	{0xB1, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1B},
	{0xB2, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1C},
	{0xB3, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1D},
	{0xB4, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1E}
};
void PZEM::init( uint32 timeout, int pbaud, int sbaud, int oe )
{
    pribaudrate = pbaud;            // if 0, do not change
    secbaudrate = sbaud;            // if 0, do not change
    oe_pin = oe;                    // -1 do not use
    safeiov = true;

    coilscale = 1.0;    
    tmout = timeout;
    simonoff[0]=simonoff[1]=simonoff[2]=simonoff[3]= false;
    state = PZ_IDLE;
    debugv = false;
}

void PZEM::start( cmdindex_t c )   // trigger the beginning of measurement
{
    if( (int) c > 4 )
        c = VOLTS;
	cmdindx = c;
	if( !simonoff[ cmdindx & 3] )                        // do not do anything if simulation is requested
	    sendCmd( cmdindx );
	//expecting = 0xA0 + (int) cmdindx;
	rcv_count = 0;
	state = PZ_WAITFOR_1ST;
	startT = millis();
    noerrdly = 0;
}

bool PZEM::ready()
{
	if( simonoff[ cmdindx &3 ] )
    {
        noerrdly = 0;
        state/* next */=PZ_IDLE;
        err = 0;
        return true;
    }
	switch( state )
	{
		case PZ_WAITFOR_1ST:
			if( millis() > (startT+tmout) )	// timeout
	        {
	            state/* next */ = PZ_DONE;
			    err = 1;
        	}
			if( Serial.available()>0 )
			{
				byte x = Serial.read();
                if( debugv )
                    oled.dsp( 7, "\a%02x", x );

				if( x == (0xA0 + cmdindx) )
				{
					response[ rcv_count++ ] = (0xA0 + cmdindx);
					state/* next */ = PZ_WAITFOR_NEXT;
				}
			}
			break;
		
		case PZ_WAITFOR_NEXT:
			// if any of the following ifs happen, returns false
			
			if( millis() > (startT+tmout) )	// timeout
			{
			    err = 2;
			    state/* next */ = PZ_DONE;
			}				
			if( Serial.available()>0 )
			{	
				int x =Serial.read();
				if( rcv_count < 7 )
				{
					if( debugv )
						oled.dsp( 7, rcv_count*2, "\a%02X", x);
					response[ rcv_count++ ] = x;
					if( rcv_count >= 7 )
					{						
						if( debugv )
							oled.dsp( 3, "%-8s", getValueText() );
						err = 0;                        
						state/* next */ = PZ_DONE;
					}
					// else, wait for next character. Stay in WAITFOR_NEXT
				}
			}
			break;

		default:
        case PZ_IDLE:
            noerrdly = millis()-startT;
            return false;
            
		case PZ_DONE:
			noerrdly = millis()-startT;
			state/* next */=PZ_IDLE;
			return true;
	}
	return false;
}
void PZEM::sendCmd( cmdindex_t c )
{
	for( int i=0; i<7; i++ )
	{
		int x =cmdTable[ (int) c ][ i ];
		if( debugv )
		    oled.dsp( 6,i*2,"\a%02X", x );
		Serial.write( x );
	}
}
// get floating value based on "scancmd" 
float PZEM::getValue()
{
	if( simonoff[ cmdindx &3 ] )
        return simvalue[ cmdindx &3 ];
	float f;
	float d1 = (float)response[1];
	float d2 = (float)response[2];
	float d3 = (float)response[3];
	switch( cmdindx )
	{
		case VOLTS:
			f = d1*256.0 + d2 + 0.1 * d3;
            break;
		case AMPS:
			f = (d2 + 0.01 * d3)*coilscale;
            break;
		case WATTS:
			f = (d1*256.0 + d2)*coilscale/1000.0;
            break;
		case ENERGY: // kWh
			f = ((d1*256.0 + d2)*256.0+d3)*coilscale/1000.0; 
            break;
		default:
		case SET_ADDRESS:
			f= 192.168;
            break;
	}
	return f;
}
char * PZEM::getValueText()
{
    float f = getValue();
    switch( cmdindx )
    {
        case VOLTS:
            sprintf( text, "%5.1f", f );       // 220.0V
            break;
        case AMPS:
            sprintf( text, "%5.2f", f );       // 99.99A
            break;
        case WATTS:
            sprintf( text, "%5.3f", f );       // 9.999kW
            break;
        case ENERGY: // Wh
            sprintf( text, "%5.2f", f );       // 9.999kWh      
            break;
        default:
        case SET_ADDRESS:
            sprintf( text, "192.168.1.1" );  
            break;
    }
    return text;
}
char *PZEM::getUnits( )
{
    switch( cmdindx )
    {
        default:
        case VOLTS: return "V";
        case AMPS:  return "A";
        case WATTS: return "kW";
        case ENERGY: return "kWh";
        case SET_ADDRESS: return "";
    }
}
void PZEM::simulate( cmdindex_t c, float value)
{
    int i = ((int)c) & 3;
    if( value == NONE )
        simonoff[ i ] = false;
    else
    {
        simonoff[ i ] = true;
        simvalue[ i ] = value;
    }       
}

void PZEM::swapSerialSec()
{
    safeiov = false;
    Serial.flush();             // empty Primary transmit buffer
    
    if( secbaudrate )
    {
        Serial.end();                   // change baud rate
        Serial.begin( secbaudrate );    // change baudrate to Primary
    }
    Serial.swap();              // swap pins
    
    if( oe_pin!=-1 )
        digitalWrite( oe_pin, (oe_pin & NEGATIVE_LOGIC) ? LOW : HIGH);    // enabe translator

    while( Serial.available() ) // empty Secondary receive buffer
    {
        Serial.read();
        yield();
    }
}
void PZEM::swapSerialPri()
{        
    Serial.flush();             // empty Secondaary transmit buffer

    if( oe_pin!=-1 )
        digitalWrite( oe_pin, (oe_pin & NEGATIVE_LOGIC) ? HIGH : LOW);    // enabe translator
    
    Serial.swap();              // swap pins
    if( pribaudrate )
    {
        Serial.end();                   // Stop
        Serial.begin( pribaudrate );   // change baudrate of Primary
    }
    while( Serial.available() ) // empty Primary receive buffer
    {
        Serial.read();
        yield();
    }
    safeiov = true;
}

// must use init() before this!
float PZEM::readMeter( cmdindex_t c )
{
    swapSerialSec();
    start( c ); 
    
    while( !ready() )
        yield();
    swapSerialPri();
    
    return getValue();
}

// ---------------- DERIVED CLASS FROM ABOVE ------------------------------

    void PZM4::start( int nxt )
    {
        PZEM::start( (cmdindex_t) nxt );
    }
    bool PZM4::readyNext()
    {
        if( PZEM::ready() )
        {
            int i = (int) cmdindx;
            meter[i].value = PZEM::getValue();
            strcpy( meter[i].text, PZEM::getValueText() );
            strcpy( meter[i].units, PZEM::getUnits() );
            return true;
        }
        return false;
    }
