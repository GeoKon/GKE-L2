---
typora-root-url: .\pz16Class.images\
---
#ifdef DOC

Hardware Setup
--------------

1. CPU --> OLED.

1.  Use RS485 interface. 
    Connect RS485 RxD to NodeMCU Sec RxD GPIO13 (D7)    (RxD to RxD!)
    Connect RS485 TxD to NodeMCU Sec TxD GPIO15 (D8)
    
    | <img src="/PZEM16_Block_Diagram.JPG" style="zoom:45%;" /> | <img src="/Wiring_Diagram.JPG" alt="Wiring Diagram" style="zoom:45%;" /> |
    | --------------------------------------------------------- | ------------------------------------------------------------ |
    | Block Diagram                                             | Wiring Diagram                                               |
    
    
How to Debug
------------
* To debug use the TERMINAL with RxD connected either to TxD or RxD of the RS485
* Use the pzm16Class application
    Ensure that Slave Addr is correct. Use `flashSlave()` 1 to set it. One device at a time

Registers
---------

![](/PZEM16_Responses.JPG)

Class Methods
-------------

```c++
PZ16( CPU &pcpu, OLED &poled );				// Specify CPU and OLED (used for MODBUS)
int measure( int slave, int nreg=9 );		// Sends cmd and receives response
int simulate( int slave, 					// Same as above but with simulated data`
				float volts, 
				float amps );	
void setTrace( int value );					// 0=no trace, 1=printf of low 
bool getError();							// Measure error 0=none, 1=tmout 2=data err
float getVolts();							// Reports after measure() or simulate()
float get	Amps();
float getWatts();
float getkWh();
float getHertz();
float getPF();
void print( char *prompt="" );				// prints all above(diagnostics)
int flashSlaveAddr( int newaddr );			// Broadcast Slave Address (no response)
int resetkWh( int slave );								

```

UART Swap Pinouts
-----------------

Serial uses UART0, which is mapped to pins:

  UART0 (Tx, Rx) (GPIO1, GPIO3) (.) 
  SWAP0 (Tx Rx)  (GPIO-15, GPIO-13) (D8, D7)
  UART1 (Tx Rx)  (GPIO-2, GPIO-?) (D4, NC)

Serial may be remapped to GPIO15 (TX) and GPIO13 (RX) by calling Serial.swap() after Serial.begin. 
Calling swap again maps UART0 back to GPIO1 and GPIO3.

    Serial1 uses UART1, TX pin is GPIO2 (D4). 
    UART1 can not be used to receive data because normally it's RX pin is occupied for 
    flash chip connection. To use Serial1, call Serial1.begin(baudrate).
    
    D4  GPIO2  TXD1
    D7  GPIO13 RXD2
    D8  GPIO15 TXD2

// returned CRCs: 01 04 02 04 d7 FBAE
// returned CRCs: 01 04 02 04 d6 3A6E
// returned CRCs: 01 04 02 04 d5 7A6F
/*  
 *   SEC Tx is pin GPIO-15 (D8)
 *   SEC Rx is pin GPIO-13 (D7)
 *   
 *   SEC Tx is pin GPIO-1
 *   SEC Rx is pin GPIO-3
 */

#endif
