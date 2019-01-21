#ifndef EEPTABLE_H
#define EEPTABLE_H

#include <cliClass.h>   // includes cpuClass.h. Definition for COMMAND
#include "eepClass.h"

namespace eepTable
{ 
	extern void init( EXE &myexe, EEP &myeep );
	extern CMDTABLE table[];
}

#endif
