// treehash (C) 2019 Mukunda Johnson
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
   if( opt_inputs.empty() && opt_inputfile.empty() ) {
      std::cout << "No input files.\n"
                << "Use --help for usage info.";
      return -1;
   }
   std::cout << std::filesystem::current_path().append(opt_inputs[1]).lexically_normal().generic_string() << "\n";
   std::cout << opt_inputs[1] << "\n";
   Test( opt_inputs[1] );
   return 0;
}

} /////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
int main( int argc, char *argv[] ) { return Treehash::Run( argc, argv ); }
