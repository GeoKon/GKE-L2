#include "oledClass.h"
#include "owonClass.h"
extern OLED oled;

void OWON::init( uint32 timeout, int pbaud, int sbaud, int oe )
{
    pribaudrate = pbaud;            // if 0, do not change
    secbaudrate = sbaud;            // if 0, do not change
    oe_pin = oe;                 // -1 do not use
    tmout = timeout;
    simul = false;
    state = PZ_IDLE;
}
void OWON::start()
{
	rcv_count = 0;
    err   = 0;
	state = PZ_WAITFOR_NEXT;
	startT = millis();
}
bool OWON::ready( bool debug )
{
	//oled.dsp( 2,0,"\vst=%d", state );
	//delay(100);

	switch( state )
	{
		case PZ_WAITFOR_NEXT:
            if( simul )                     // if simulation is on
            {
                err = 0;
                state = PZ_DONE;
                return false;
            }
			if( millis() > (startT+tmout) )	// timeout
			{
			    err = 1;    // timeout;
			    /* next */state = PZ_DONE;
			}
			if( Serial.available()>0 )
			{	
				int x =Serial.read();
				if( rcv_count < MAX_RESP )
				{
					if( debug )
						oled.dsp( 2, rcv_count*2, "\a%02X", x);
					response[ rcv_count++ ] = x;
					if( rcv_count >= MAX_RESP )
					{						
						if( debug )
							oled.dsp( 3, 0, "%-8s", "Done" /*getValueText()*/ );
						
						if( extract() ) // flip to 16-bits
						    err = 0;
                        else
                            err = 2;
						/* next */state = PZ_DONE;
					}
					// else, wait for next character. Stay in WAITFOR_NEXT
				}
			}
			return false;

		default:
        case PZ_IDLE:
            return false;

		case PZ_DONE:
            /* next */state = PZ_IDLE;
			return true;
	}
}
void OWON::simulate( bool onoff, float value )
{
    simul = onoff;
    if( !onoff )
        return;
    
    float x = value;
    bool positive = true;
    uint16_t measurement;

    // class variables
    function = 10;
    scale = 0;
    decimal = 0;

    if( value < 0.0 )
    {
        x = -value;
        positive = false;
    }                
    if( x <=6.0 )  // 5.999 V
    {
        measurement = (uint16_t) (x*1000.0);
        decimal = 3;
    }
    else if( x <=60.0 ) // 59.99V
    {
        measurement = (uint16_t) (x*100.0);
        decimal = 2;
    }
    else if( x<=600.0 ) // 599.9V
    {
        measurement = (uint16_t) (x*10.0);
        decimal = 1;
    }
    else if( x<=6000.0 )     // 5999V
    {
        measurement = (uint16_t) (x);
        decimal = 0;
    }
    else                // overflow
    {
        
    }
    resp16[0] = 0xF000 | (scale<<3) | (function <<6) | decimal;
    resp16[1] = 0;
    resp16[2] = positive ? measurement : (0x8000 | measurement);
    
    //PF("Measu = %f, %d, dec=%d\r\n", x, measurement, decimal );
}

// Sets 'resp16[]', decimal, type, scale, function
bool OWON::extract()
{
    for( int i=1; i<MAX_RESP; i++ )
    {
        if( (response[i] & 0xF0) == 0xF0 )
        {
            uint8 *msb = &response[i];
            uint8 *lsb = &response[i-1];
            
            for( int j=0; j<3; j++, msb+=2, lsb+=2 )
                resp16[j] = (*msb)*256+(*lsb);

            // Extract data items from first number
            function    = (resp16[0] >> 6) & 0x0f;
            scale       = (resp16[0] >> 3) & 0x07;
            decimal     =  resp16[0] & 0x07;
            return true;                
        }        
    }
    function    = 0;
    scale       = 0;
    decimal     = 0;
    return false;
}

// get floating value based on "scancmd" 
// full scale count is 6000
float OWON::getValue( bool notscaled )
{
    // Extract and convert measurement value
    float measurement;
    if (resp16[2] < 0x7fff) 
        measurement = (float)resp16[2];
    else 
        measurement = -1 * (float)(resp16[2] & 0x7fff);

	if( notscaled )
		return measurement;
    switch( decimal )
    {
        case 0:
            return measurement;
        case 1:
            return measurement/10.0;
        case 2:
            return measurement/100.0;
        case 3:
            return measurement/1000.0;
        default:
            return 10000.0;
    }
}

char * OWON::getValueText()
{
    static char vtext[20];
    float measurement = getValue();
    switch( decimal )
    {
        case 0:
            sprintf( vtext, "%.0f", measurement);
            break;           
        case 1:
            sprintf( vtext, "%.1f", measurement);
            break;           
        case 2:
            sprintf( vtext, "%.2f", measurement);
            break;           
        case 3:
            sprintf( vtext, "%.3f", measurement);
            break;           
        default:
            sprintf( vtext, "Overload ");    
            break;
    }
    return vtext;
}
char * OWON::getReading()
{
    static char text[20];
    sprintf( text, "%s %s %s %s", getValueText(), getUnits(), getACDC(), getType() );
    return text;
}
// Uses 'type' to return measurement type string
char * OWON::getType( bool ascii )
{
    static char s_type[20];
    char *sbat = "";
    char *stype = "";
    int type = resp16[1];
    
    // trasnalte 'type' to stype and sbat
    if( type & 0x8 ) 
        sbat = "BAT";    // low battery
    
    if( type & 0x30 )
    {
        if (type & 0x10) stype = "min";
        if (type & 0x20) stype = "max";
    }
    else if( type & 0x03 )
    {
        int t = type & 3;
        if( t == 2 )
            if( ascii )
                stype = "D";
            else
                stype = "Δ";    
        else if( t == 1 )
            stype = "H";    
        else
            if( ascii )
                stype = "DH";
            else
                stype = "ΔH";    
    }
    if( *sbat )
        if( *stype )
            sprintf( s_type, "%s %s", stype, sbat );
        else
            sprintf( s_type, "%s", sbat );
    else
        sprintf( s_type, "%s", stype );
    return s_type;
}
// Uses 'scale' and 'function' to return measurement units string
char * OWON::getUnits( bool ascii )
{
    static char s_unit[8];
    
    char c = 0;
    char *sunit = "";
    char *stype = "";
    
    switch (scale) 
    {
        case 1:
            c = 'n'; break;
        case 2:
            c = 'u'; break;
        case 3:
            c = 'm'; break;
        case 5:
            c = 'k'; break;
        case 6:
            c = 'M'; break;
        default:
            c = 0;
    }
    switch (function) 
    {
        case 0:
        case 1:
        case 10:
            sunit = "V";  break;
        case 2:
        case 3:
            sunit = "A";  break;
        case 4:
        case 11:
            sunit = "Ohms"; break;
        case 5:
            sunit = "F";   break;
        case 6:
            sunit = "Hz";  break;
        case 7:
            sunit = "%%";  break;            
        case 8:
            sunit = (char *)(ascii ? "degC" : "°C");  break;            
        case 9:
            sunit = (char *)(ascii ? "degF" : "°F");  break;            
        case 12:
            sunit = "hFE";  break;
        default:
            sunit = "";  break;            
    }
    if( c )
      sprintf( s_unit, "%c%s", c, sunit );
    else
      sprintf( s_unit, "%s", sunit );
    return s_unit;
}
// Uses 'scale' and 'function' to return measurement units string
char * OWON::getACDC()
{
    char *stype;
    
    switch (function) 
    {
        case 0:
            stype = "DC"; break;
        case 1:
            stype = "AC"; break;
        case 2:
            stype = "DC"; break;
        case 3:
            stype = "AC"; break;
         default:
            stype = "";  break;            
    }
    return stype;
}
int OWON::error()
{
    return err;
}
void OWON::swapSerialSec()
{
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
void OWON::swapSerialPri()
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
}
// must use init() before this!
int OWON::readMeter()
{
    swapSerialSec();
    start(); 
    
    while( !ready() )
        yield();
    swapSerialPri();
    
    return err;
}
