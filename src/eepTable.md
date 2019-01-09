Summary
-------
<a id="example"></a>
- __registerUserParms()__ registers user parameters & format
- __registerInitCallbk()__ resisters parameter initiaziation callback function
<br><br>
- __initWiFiParms()__ initializes WiFi parameter structure
- __initUserParms()__ initializes user parameter structure calling the registed callback
- __saveParms( sel )__ saves parameters in EEPROM
- __fetchParms( sel )__ fetches parameters from EEPROM and populates the WiFi and user data structures, if previously registered. Returns an error code or 0 if all parameters were fetched.<br><br> __sel__ can take the following values

> HEAD_PARMS to refer to header parameters<br>
> WIFI_PARMS to refer to WiFi parameters<br>
> USER_PARMS to refer to user parameter<br>
> ALL_PARMS to refer to all above<br>
> *default* __sel__ is ALL_PARMS
- __getParmsString( sel )__ returns a String with a selection of parameters
- __printParms( sel )__ prints the parameters selectively
- __updateWiFiParms(...)__ defines new wifi parameters and updates eeprom
- __updateRebootCount()__ increments reboot counter and updates eeprom

Code Example
------------
Define user parm data structure and format

`struct { int i1, i2, i3 } us, us0={10, 20, 30};`<br>
`char *format ="i1=%d, i2=%d, i3=%d\r\n";`

main program follows
```
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
```
- now, jump there <a href="#example">Example headline</a>
- and [this one](<a href="#example2">CODE REF</a>)
- [opem test](file://eepClass.cpp)
- [reference below](below)
- [text below](reftext)
```