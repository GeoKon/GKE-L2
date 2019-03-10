// --------------------- Add in this file CLI handlers -------------------------

#include "eepGlobals.h"

// ------------------- allocation of base classes ------------------------------
    CPU cpu;
    CLI cli;
    EXE exe;
    EEP eep;
    SER ser;
    Global myp;
    
// ---------------------------- User Interaction --------------------------------
void interactForever()
{
    cli.init( ECHO_ON, "cmd: " );       
    PR("CLI Mode. Press 'h' for help\r\n");
    
    cli.prompt();
    for(;;)
    {
        if( cli.ready() )
        {
            char *p = cli.gets();
//            exe.dispatchConsole( p );
//
//            PF("--- Repeated ---\r\n");
            
            BUF temp(2000);                // response buffer
            exe.dispatchBuf( p, temp );
            temp.print();
            
            cli.prompt();
        }
    }
}

// ----------------------------- CLI Command Handlers ---------------------------
namespace mypTable
{
    static EXE *_exe;       //pointer to EXE class defined in main()
    
    void init( EXE &myexe ) {_exe = &myexe;}
    void help( int n, char *arg[] ) {_exe->help( n, arg );}

    void setUserParm( int n, char **arg )
    {
        if( n<2 )
        { PF("Use <name> <value>\r\n"); return;}

        int type = ser.getParmType( arg[1] );
        if( type==0 )
            PF("Parm %s not found\r\n", arg[1]);
        else
        {
            ser.setParmByStr( arg[1], arg[2] );                    
            PF("OK. Use 'usave' after you are done\r\n");
        }
    }
    CMDTABLE table[]= 
    {
        {"h", "--- List of all commands ---", help },
        {"usave",  "Save User Parms",   [](int, char**){ myp.saveMyParms();}},
        {"uinit",  "Initialize default User Parms",   [](int, char**){ myp.initMyParms();}},
        {"ufetch", "fetch User Parms",  [](int, char**){ myp.fetchMyParms();}},
        {"ushow",  "show User Parms",   [](int, char**){ ser.printParms();}},
        {"uset",   "<name> <value>. Set User Parms",  setUserParm },
        
        {NULL, NULL, NULL}
    };
}
