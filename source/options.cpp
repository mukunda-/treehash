// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "options.h"
#include "usage.h"

#include <iostream>
#include <algorithm>
///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

//-----------------------------------------------------------------------------
struct NoMoreArgs : public std::runtime_error {
   NoMoreArgs() : std::runtime_error( "Not enough arguments to option." ) {}
};

//-----------------------------------------------------------------------------
class ArgIterator {
   int    m_argc;
   char **m_argv;
   int    m_index = 0;
public:
   //--------------------------------------------------------------------------
   ArgIterator( int argc, char *argv[] )
         : m_argc( argc )
         , m_argv( argv ) {
   }
   //--------------------------------------------------------------------------
   std::string Get() {
      if( End() ) throw NoMoreArgs();
      return m_argv[ m_index++ ];
   }
   //--------------------------------------------------------------------------
   std::string GetLast() const noexcept {
      if( m_index == 0 ) return "";
      return m_argv[ m_index - 1 ];
   }
   //--------------------------------------------------------------------------
   bool End() const noexcept {
      return m_index == m_argc;
   }
};

//-----------------------------------------------------------------------------
void ReadOption( ArgIterator &args ) {
   if( args.End() ) return;

   std::string arg = args.Get();
   if( arg.empty() ) ReadOption( args );
   
   if( arg[0] == '-' ) {
      //if( arg == "--in"  || arg == "-i" ) opt_inputfile = args.Get();
      //if( arg == "--out" || arg == "-o" ) opt_output = args.Get();
      if( arg == "--base" || arg == "-b" ) {
         opt_basepath = args.Get();
      } else if( arg == "--help" || arg == "-h" ) {
         PrintUsage();
         std::exit( 0 );
      }
   } else {
      // Input.
      opt_inputs.emplace_back( arg );
   }

   ReadOption( args );
}

//-----------------------------------------------------------------------------
void ReadOptions( int argc, char *argv[] ) {
   ArgIterator args( argc, argv );
   try {
      ReadOption( args );
   } catch( NoMoreArgs& ) {
      std::cout << "Missing expected argument after " << args.GetLast();
      std::exit( -1 );
   }
}

}