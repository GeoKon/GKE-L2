#ifndef EEGLOBALS_H
#define EEGLOBALS_H

#include <cliClass.h>       // CLI and EXE classes. Also includes the cpuClass.h
#include <eepClass.h>   
#include <serClass.h>  
#include <eepTable.h>

#include <FS.h>
   
// ----------------- defined in eepGlobals.cpp -----------------------
    extern CPU cpu;
    extern CLI cli;
    extern EXE exe;
    extern EEP eep;
    extern SER ser;

    extern void interactForever();

// -------------------------------------------------------------------
    namespace mypTable 
    {
        extern void init( EXE &myexe );
        extern CMDTABLE table[]; 
    }

// ---------------- add here global parameters -----------------------
    class Global
    {
    public:    
        float f1;                       // named user parameters
        float f2;
        int i1;
        char cp[20];
        
        #define NPARMS 4                // number of named user parameters 
        
        byte ubuf[ NPARMS*4+40+16 ];    // buffer to hold encoded data
        int bsize;                      // set by 'install()' below
//        void registerMyParms();         // (1) Must be called at the beginning of the program!
//        void initMyParms();             // (2a) initialize parameters saved them in EEPROM
//        void fetchMyParms();            // (2b) OR, fetch them from EEPROM
//        void saveMyParms();             // (3) use after modifying myp.parameters

        void registerMyParms()                              // (1) Must be called at the beginning of the program!
        {
            ser.resetRegistry();
            ser.registerParm( "f1", 'f', &f1, "=%4.2f Volts");
            ser.registerParm( "f2", 'f', &f2 );
            ser.registerParm( "i1", 'd', &i1 );
            ser.registerParm( "cp", 's', &cp[0] );
            ser.registryStatus("After cp");

            ser.encodeParms( &ubuf[0], sizeof ubuf );       // encode them
            bsize = ser.nbytes;                             // remember the size
        }
        void initMyParms()                                  // (2a) initialize parameters saved them in EEPROM
        {
            f1=4.56;
            f2=123.0;
            i1=10;
            strcpy( &cp[0], "ABCDabcd");
            saveMyParms();                                  // and save the new values            
        }
        void fetchMyParms()                                 // (2b) OR, fetch them from EEPROM
        {
            eep.fetchUserStruct( &ubuf[0], sizeof ubuf );   // fetch them
            ser.decodeParms( &ubuf[0] );                    // then decode them in SER
        }        
        void saveMyParms()                                  // (3) use after modifying myp.parameters
        {
            ser.encodeParms( &ubuf[0], sizeof ubuf );
            eep.saveUserStruct( &ubuf[0], bsize );
        }
    };

    extern Global myp;

#endif
