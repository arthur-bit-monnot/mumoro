
#ifdef CWDEBUG

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
 
class KxStackTrace
{
public:
    KxStackTrace();
};

KxStackTrace stackTraceHandler;

void abortHandler( int signum );
static void printStackTrace( FILE *out = stderr, unsigned int max_frames = 63 );
 
KxStackTrace::KxStackTrace()
{
  signal( SIGABRT, abortHandler );
  signal( SIGSEGV, abortHandler );
  signal( SIGBUS,  abortHandler );
  signal( SIGILL,  abortHandler );
  signal( SIGFPE,  abortHandler );
  
  std::cout << "Initializing signal handler\n";
}

void abortHandler( int signum )
{
   // associate each signal with a signal name string.
   const char* name = NULL;
   switch( signum )
   {
   case SIGABRT: name = "SIGABRT";  break;
   case SIGSEGV: name = "SIGSEGV";  break;
   case SIGBUS:  name = "SIGBUS";   break;
   case SIGILL:  name = "SIGILL";   break;
   case SIGFPE:  name = "SIGFPE";   break;
   }
 
   // notify the user which signal was caught. We use printf, because this is the most
   // basic output function. Once you get a crash, it is possible that more complex output
   // systems like streams and the like may be corrupted. So we make the most basic call
   // possible to the lowest level, most standard print function.
   if ( name )
      fprintf( stderr, "Caught signal %d (%s)\n", signum, name );
   else
      fprintf( stderr, "Caught signal %d\n", signum );
 
   // Dump a stack trace.
   // This is the function we will be implementing next.
   printStackTrace();
 
   // If you caught one of the above signals, it is likely you just 
   // want to quit your program right now.
   exit( signum );
}


 
static inline void printStackTrace( FILE *out, unsigned int max_frames )
{
   fprintf(out, "stack trace:\n");
 
   // storage array for stack trace address data
   void* addrlist[max_frames+1];
 
   // retrieve current stack addresses
   uint addrlen = backtrace( addrlist, sizeof( addrlist ) / sizeof( void* ));
 
   if ( addrlen == 0 ) 
   {
      fprintf( out, "  \n" );
      return;
   }
 
   // create readable strings to each frame.
   char** symbollist = backtrace_symbols( addrlist, addrlen );
 
   // print the stack trace.
   for ( uint i = 4; i < addrlen; i++ )
      fprintf( out, "%s\n", symbollist[i]);
 
   free(symbollist);
}

#endif  // CWDEBUG