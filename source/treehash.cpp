// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include <filesystem>
#include <regex>

//-----------------------------------------------------------------------------
#include "usage.cpp"
#include "options.cpp"
#include "hash.cpp"

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

//-----------------------------------------------------------------------------
int Run( int argc, char **argv ) {
   ReadOptions( argc, argv );
   if( opt_inputs.empty() ) {
      std::cout << "No input files.\n"
                << "Use --help for usage info.";
      return 1;
   }
   
   Hash hash = 0;
   for( auto &input : opt_inputs )
      hash ^= HashInput( input );
      std::cout << hash;
   
   std::cout << HashToHex( hash );
   return 0;
}

} /////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
int main( int argc, char *argv[] ) { return Treehash::Run( argc, argv ); }
