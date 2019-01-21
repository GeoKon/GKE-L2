Guide to GKESP-L2 Library
-------------------------

This library includes the files and classes shown below. More details
can be found in the `\src` subdirectory and the associated `.md` files.


| Files  | Class(es)   | ..\examples | .                  Functionality                 .  |
|------  |:---------:  |--------------- |---------                  |
|eepTable.cpp eepTable.h   |  EEP | eeTable.ino   | Exports `namespace eepTable` with a command-table relating to EEPROM |  
|ntpClass.cpp nttpClass.h  |  NTP | ntpClass.ino  | Gets date-time from NTP server | 
|owonClass.cpp owonClass.h | OWON | owonClass.ino | Connect to the OWON B35T multimeter via BTLE/2nd Serial |
|pzmClass.cpp owonClass.h | PZEM PZM4 | pzmClass.ino | Connects to PZEM-04T power monitor via 2nd Serial |

*** See important conventions used in all libraries in CONVENTIONS.md of GKESP-L1 ***

### Dependencies

In general, all L2 classes depend on L1 classes. More specifically,

| Class or namespace  | Depends on L1 Classes |
|-------:   | ----------------------|
| eepTable	| CPU, CLI, EEP	        |
| NTP       | CPU, Time             |
| OWON      | CPU, OLED             |
| PZM	    | CPU, OLED				| 

See the following for a graphical representation
![](https://i.imgur.com/IlBnKaE.gif)