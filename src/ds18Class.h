#pragma once

#include <OneWire.h>

typedef byte DevAddr[8];        // structure to hold DS18 address

#define DS18MAX_SENSORS     4   // max number of sensors supported
#define DS18ERR_NO_SENSOR	1
#define DS18ERR_BAD_CRC		2
#define INVALID_TEMP      -40.0	// invalid temperature. Could be set to NAN

#define C_TO_F(C) ((C*1.80) + 32.0)
#define F_TO_C(F) ((F-32.0) * 0.555555556)

class DS18
{
public:
    DS18( OneWire *myds) { ds = myds; }	// associate this class with the OnwWire class

	int search( bool debug=false );	// searches for sensors, fills private table. Returns devices found
    
	void start( int devid );        // use thisID() to start with zero and increment
	
	DevAddr *getAddr( int idx ) { return &list[ idx<DS18MAX_SENSORS ? idx : 0]; }

    bool ready();   				// has conversion been completed?
 	
	float getDegreesC()	{ return tempC; }
	float getDegreesF()	{ return C_TO_F( tempC ); }

	bool success( bool debug = false );// if debug, prints error messages

	bool error() {return err; }

    void simulC( float tempC );
	void simulF( float tempF );
	void simulOff();
	int count() { return found; }
	int nextID() { if( ++idx>=found ) idx = 0; return idx; }
	
private:
    OneWire *ds;
    DevAddr list[DS18MAX_SENSORS];  // MAX sets of 8-byte addresses
	float tempC;				// temp in deg C

	int  err;					// error while reading the n-th temperature
    int  found;                 // number of devices found by init()
    int  idx;                   // current device addressed 0...found-1
    byte state;                 // current state 0, 1 or 2

    bool  simulON; 				// simulation on or off for 0th temp
    float simTemp; 				// simulated temperature in deg C
	uint32_t T0;
};

// #define MAX_FILTER_IDs 4
// class FLT
// {
// private:
    // float smoothed  [ MAX_FILTER_IDs ];
    // bool started    [ MAX_FILTER_IDs ];
    // float coef;
    // int maxids;
    
// public:
    // FLT( int mxids );
    // void setCoef( float coef );
    // float smooth( int id, float x );    
    // float getValue( int id );
// };


