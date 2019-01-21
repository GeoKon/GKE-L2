How to use eepTable
-------------------

- `registerUserParms()` registers user parameters & format
- `registerInitCallbk()` resisters parameter initiaziation callback function
- `initWiFiParms()` initializes WiFi parameter structure
- `initUserParms()` initializes user parameter structure calling the registed callback
- `saveParms( sel )` saves parameters in EEPROM
- `fetchParms( sel )` fetches parameters from EEPROM and populates the WiFi and 
user data structures, if previously registered. Returns an error code or 0 if 
all parameters were fetched. 

- `sel` can take the following values
	> `HEAD_PARMS` to refer to header parameters<br>
	> `WIFI_PARMS` to refer to WiFi parameters<br>
	> `USER_PARMS` to refer to user parameter<br>
	> `ALL_PARMS` to refer to all above<br>
	> *default* `sel` is `ALL_PARMS`
 
- `getParmsString( sel )` returns a String with a selection of parameters
- `printParms( sel )` prints the parameters selectively
- `updateWiFiParms(...)` defines new wifi parameters and updates eeprom
- `updateRebootCount()` increments reboot counter and updates eeprom

Code Example
------------
Define user parm data structure and format

`struct { int i1, i2, i3 } us, us0={10, 20, 30};`<br>
`char *format ="i1=%d, i2=%d, i3=%d\r\n";`

main program follows

	void main()
	{
	    eep.registerUserParms( &us, sizeof(us), format );
	    
	    if( !fetchParms() ) // if EEPROM has bad data, initializes
	    {
	        eep.initWiFiParms( / Use the defaults / );
	        us = us0;
	        eep.saveParms();
	    }
	    eep.updateRebootCount();
	    eep.printParms( "Current Parms");
	    .....
	    // use freely: eep.wifi.ssid, etc
	    // use also:   eep.us.i1, etc
	    // if you modify, use eep.saveParms()
	}

See `\examples` subdirectory for working code examples
 