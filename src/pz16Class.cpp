/* ---------------------------------------------------------------------------------
 *  Copyright (c) George Kontopidis 1990-2019 All Rights Reserved
 *  You may use this code as you like, as long as you attribute credit to the author
 * ---------------------------------------------------------------------------------
 */
 #include "pz16Class.h"

// ----------------------------- MODBUS CLASS ------------------------------------

    MODBUS::MODBUS()
    {
        ole = NULL;
    }
    void MODBUS::setOLED( OLED *poled )
    {
        ole = poled;
    }    
    void MODBUS::fillCMD( byte slave, byte b[], int count )
    {
        uint16 crc;
        
        memset( cmdbytes, 0, PZ_CMDSIZE );
        cmdbytes[0] = slave;
        for( cmdcount=1; (cmdcount<=count) && (cmdcount<PZ_CMDSIZE); cmdcount++ )
            cmdbytes[cmdcount] = *b++;

        crc = crcBlock( cmdbytes, cmdcount );
        cmdbytes[cmdcount++] = crc&0xFF;            // CRC LSB first according to MODBUS std
        cmdbytes[cmdcount++] = (crc >> 8) & 0xFF;
    }
    void MODBUS::parseCMD( const char *s )                  // Fills the cmdbyte[] array by parsing the string
    {
       const char *p = s;
       int hexM, hexL;
       uint16 crc;
       
       memset( cmdbytes, 0, PZ_CMDSIZE );
       
       while( *p  )                         // skip non-hex characters
         if( ishex( *p++, &hexM ) )
            break;

       for( cmdcount=0; cmdcount<PZ_CMDSIZE; )     // index across the cmd array
       {
            if( !ishex( *p++, &hexL ) )         // if hex, convert to binay
                break;
            cmdbytes[cmdcount++]=hexM*16+hexL;
            if( *p == 0 )
                break;
            if( !ishex( *p++, &hexM ) )         // if hex, convert to binay
                break;                        
        }            
        crc = crcBlock( cmdbytes, cmdcount );
        cmdbytes[cmdcount++] = PZ_LSB(crc);    // CRC LSB first according to MODBUS std
        cmdbytes[cmdcount++] = PZ_MSB(crc);        
        
        //PFN("CMD+CRC is %d bytes", cmdcount );
    }
    void MODBUS::printCMD( char *prompt )
    {
        if( *prompt )
            PF( "%s: ", prompt );
//        PFN("Adr:%02X, Cmd:%02X, BaseReg:%04X, Count:%04X, CRC:%04X (%d bytes)",
//            cmdbytes[0],
//            cmdbytes[1],
//            PZ_16BITS( cmdbytes, 2 ),
//            PZ_16BITS( cmdbytes, 4 ),
//            PZ_16BITS( cmdbytes, 6 ),
//            cmdcount);
        for( int i=0; i<cmdcount; i++ )
            PF("%02X", cmdbytes[i] );
        PFN( " (%d bytes)", cmdcount );
    }
    void MODBUS::printRSP( char *prompt )
    {
        if( *prompt )
            PF( "%s: ", prompt );
        for( int i=0; i<rcvcount; i++ )
            PF("%02X", rcvbytes[i] );
        PFN( " (%d bytes)", rcvcount );
    }
    void MODBUS::sendCMD(  )
    {
        PZ_DSP( 3, "Sending...");
        PZ_DSP( 4, O_CLEAR_ROW );
        for( int i=0; i<cmdcount; i++ )
        {
            Serial.write( cmdbytes[i] );
            if( i<8 )
                { PZ_DSP(4, 2*i, "%02X", cmdbytes[i] ); }
        }
    }
    void MODBUS::fillRSP( byte slave, byte reg[], int count )
    {
        uint16 crc;
        memset( rcvbytes, 0, PZ_RCVSIZE ); 
        
        rcvbytes[0] = slave;
        rcvbytes[1] = 4;            // read register
        if( count > PZ_RCVSIZE-5 )
            count = PZ_RCVSIZE-5;        
        rcvbytes[2] = count ; 
        
        for( int i=0; i<count; i++ )
            rcvbytes[ 3+i ] = reg[i];

        rcvcount = 3 + count;
        crc = crcBlock( rcvbytes, rcvcount );
        rcvbytes[rcvcount++] = PZ_LSB(crc);            // CRC LSB first according to MODBUS std
        rcvbytes[rcvcount++] = PZ_MSB(crc);
    }
    int MODBUS::receiveRSP( const int expectedcount )
    {
        unsigned long T0 = millis();
        unsigned long delta = PZ_TMOUT1st;      // initial waiting period
        uint8 x;
        
        memset( rcvbytes, 0, PZ_RCVSIZE ); 
            
        PZ_DSP( 6, "Receiving...");
        PZ_DSP( 7, O_CLEAR_ROW );
                
        rcvcount = 0;
        while( millis() < (T0+delta) )
        {
            if( Serial.available() > 0 )
            {
                x = Serial.read();
                if( rcvcount == 0 )             // 1st char received
                    delta = PZ_TMOUTnth;        // interchar delay
                T0 = millis();                  // reset ms timer
                if( (rcvcount < expectedcount) &&
                    (rcvcount < PZ_RCVSIZE ) )
                {
                    if( rcvcount<8 )
                    { PZ_DSP( 7, rcvcount*2, "%02X", x ); }
                    rcvbytes[ rcvcount++ ] = x;
                }
                else
                    break;                
            }
        }
        if( rcvcount == 0 )         // initial timeout
            return (tmerror=1);
        else
            return (tmerror=0);             
    }
    int MODBUS::rcvError()
    {
        return tmerror;
    }
//--------------------------- PZEM16 CLASS ----------------------------

    PZ16::PZ16( CPU &pcpu, OLED &poled )
    {
        mpu   = &pcpu;
        trace = 0;
        error = false;
        setOLED( &poled );
    }
    bool PZ16::getError()
    {
        return error;
    }
    void PZ16::setTrace( int mtrace )
    {
        trace = mtrace;
    }
    // to measure just volts, use nreg=1
    // to measure volts & amps, use nreg=3
    int PZ16::measure( int slave, int nreg )
    {
        byte pdu[]={04, 00, 00, 00, 07 };        // command to fetch 16-bit registers
        pdu[4] = nreg;
        
        fillCMD( slave, pdu, sizeof( pdu ) );    
        if( trace )
            printCMD("CMD");
        mpu->swapSerialSec();                
        sendCMD();                              // send command
        receiveRSP( 3 + nreg*2 + 2 );               // receive response
        mpu->swapSerialPri();

        error = rcvError();                     // 0 or 1 for Timeout
        if( rcvbytes[1] != cmdbytes[1] )
            error++;
        if( trace )
        {
            if( error==1 )      printRSP( "TMO" );
            else if( error>1 )  printRSP( "ERR" );
            else                printRSP( "RCV" );
        }
        return error;
    }
    int PZ16::simulate( int slave, float volts, float amps )
    {
        byte values[21];
        uint16 value;
        uint32 value32;
        uint16 vL, vH;

        memset( values, 0, sizeof( values ) );
        value = volts*10.0;
        values[0] = PZ_MSB(value);
        values[1] = PZ_LSB(value);
        
        value32 = (amps * 1000.0);
        vL      = value32 & 0xFFFF;
        vH      = (value32 << 16 ) & 0xFFFF;
        values[2] = PZ_MSB( vL );
        values[3] = PZ_LSB( vL );
        values[4] = PZ_MSB( vH );
        values[5] = PZ_LSB( vH );

        value32 = (amps * volts);
        vL      = value32 & 0xFFFF;
        vH      = (value32 << 16 ) & 0xFFFF;
        values[6] = PZ_MSB( vL );
        values[7] = PZ_LSB( vL );
        values[8] = PZ_MSB( vH );
        values[9] = PZ_LSB( vH );

        byte pdu[]={04, 00, 00, 00, 05 };        // command to fetch 16-bit registers
        fillCMD( slave, pdu, sizeof( pdu ) ); 
        fillRSP( slave, values, sizeof( values ) );
        
        error = false;
        if( trace )
        {
            printCMD("CMD");
            printRSP("SIM");
        }
    }
    float PZ16::getVolts()
    {
        uint16 value = PZ_16BITS( rcvbytes, 3 );
        return (float)value/10.0;
    }
    float PZ16::getAmps()
    {
        uint32 value = PZ_32BITS( rcvbytes, 5 );
        return ((float)value)/1000.0;
    }
    float PZ16::getWatts()
    {
        uint32 value = PZ_32BITS( rcvbytes, 9 );
        return ((float)value)/10.0;
    }
    float PZ16::getkWh()
    {
        uint32 value = PZ_32BITS( rcvbytes, 13 );
        return ((float)value)/1000.0;
    }
    float PZ16::getHertz()
    {
        uint16 value = PZ_16BITS( rcvbytes, 17 );
        return ((float)value)/10.0;
    }
    float PZ16::getPF()
    {
        uint16 value = PZ_16BITS( rcvbytes, 19 );
        return ((float)value)/100.0;
    }
    void PZ16::print( char *prompt )
    {
        if( *prompt )
            PF("%s ");
        PF( error? "ERR! " : " OK: " );
        PFN( "%.1fV, %.3fA, %.2fW, %.3fkWh, %.1fHz, %.3fPF",
            getVolts(),
            getAmps(),
            getWatts(),
            getkWh(),
            getHertz(),
            getPF() );
    }
    int PZ16::flashSlaveAddr( int newslave )
    {
        byte pdu[]={ 06,        // write single register
                     00, 02,    // address of the register
                     00, 00 };  // slave address
        pdu[4] = newslave;
        fillCMD( 0, pdu, sizeof( pdu ) );       // broadcast command, no response 
        
        mpu->swapSerialSec();   
        sendCMD();                              // send command
        mpu->swapSerialPri();
        if( trace )
            printCMD("CMD");
        return (error = 0);
    }
    int PZ16::resetkWh( int slave )
    {
        byte pdu[]={ 0x42 };      // reset energy
        fillCMD( slave, pdu, sizeof( pdu ) );       // broadcast command, no response 
        if( trace )
            printCMD("CMD");
        
        mpu->swapSerialSec();                
        sendCMD();                                  // send command
        receiveRSP( 2 + 2 );                        // receive response: OK 4-bytes. Error 5-bytes
        mpu->swapSerialPri();

        error = rcvError();                         // 0 or 1 for Timeout
        if( rcvbytes[1] != cmdbytes[1] )
            error++;
        
        if( trace )
        {
            if( error==1 )      printRSP( "TMO" );
            else if( error>1 )  printRSP( "ERR" );
            else                printRSP( "RCV" );
        }
        return error;
    }
//------------------------------------- UTILITIES ---------------------------------
/*
    Processor-independent CRC-16 calculation.

    Polynomial: x^16 + x^15 + x^2 + 1 (0xA001)<br>
    Initial value: 0xFFFF

    This CRC is normally used in disk-drive controllers.

   uint16_t crc (0x0000..0xFFFF)
   uint8_t a (0x00..0xFF)
   RETURNS calculated CRC (0x0000..0xFFFF)
*/

    uint16_t crc16( uint16_t crc, int a )
    {
      crc ^= a;
      for (int i = 0; i < 8; ++i)
      {
        if (crc & 1)
          crc = (crc >> 1) ^ 0xA001;
        else
          crc = (crc >> 1);
      }
      return crc;
    }
    uint16_t crcBlock( const byte b[], size_t n )
    {
        uint16 crc;
        
        crc = 0xFFFF;                       // compute CRC
        for( int i=0; i<n; i++ )
            crc = crc16( crc, *b++ );
        return crc;            
    }
    bool ishex( const char c, int *ip )
    {
        if( (c>='0') && (c<='9') )
            *ip = c-'0';
        else if( (c>='A') && (c<='F') )
            *ip = c-'A'+10;
        else if( (c>='a') && (c<='f') )
            *ip = c-'a'+10;
        else
        {
            *ip = 0;
            return false;
        }
        return true;
    }
/*
 * Serial uses UART0, which is mapped to pins (
 * 
 *  UART0 (Tx, Rx) (GPIO1, GPIO3) (.) 
 *  SWAP0 (Tx Rx)  (GPIO-15, GPIO-13) (D8, D7)
 *  UART1 (Tx Rx)  (GPIO-2, GPIO-?) (D4, NC)
 *  
 * Serial may be remapped to GPIO15 (TX) and GPIO13 (RX) by calling Serial.swap() after Serial.begin. 
 * Calling swap again maps UART0 back to GPIO1 and GPIO3.

    Serial1 uses UART1, TX pin is GPIO2 (D4). 
    UART1 can not be used to receive data because normally it's RX pin is occupied for 
    flash chip connection. To use Serial1, call Serial1.begin(baudrate).

    D4  GPIO2  TXD1
    D7  GPIO13 RXD2
    D8  GPIO15 TXD2
 
 */
