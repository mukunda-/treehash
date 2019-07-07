// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "options.h"
#include "util.h"
#include "hash.h"
#include "default_scanner.h"

#include <string>
#include <iostream>
#include <filesystem>
#include <regex>
#include <chrono>

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

//-----------------------------------------------------------------------------
int Run( int argc, char **argv ) {
   auto start_time = std::chrono::steady_clock::now();
   ReadOptions( argc, argv );
   if( opt_verbose )
      std::cout << "treehash v" << VERSION
                << " (C) 2019 Mukunda Johnson (mukunda@mukunda.com)\n";
   
   if( opt_inputs.empty() ) {
      std::cout << "No input files.\n"
                << "Use --help for usage info.\n";
      return 1;
   }

   if( opt_basepath.empty() ) {
      if( opt_verbose ) {
         std::cout << "No --base given. Using current working directory.\n";
      }
      opt_basepath = std::filesystem::current_path().generic_string();
   }

   if( opt_verbose ) {
      std::cout << "Running in verbose mode.\n";
      size_t inputcount = opt_inputs.size();
      std::cout << inputcount << " input" << PluralS(inputcount) << ".\n";

      std::cout << "Any files listed without a * were excluded by your filters.\n";
   }

   DefaultScanner scanner;
   
   Hash hash = 0;
   for( auto &input : opt_inputs ) {
      if( opt_verbose )
         std::cout << "Processing input \"" << input << "\"\n";
      hash ^= HashInput( input, scanner );
      if( opt_verbose )
         std::cout << "Hash for \"" << input << "\": " << HashToHex(hash) << "\n";
   }

   if( opt_verbose ) std::cout << "Final result: ";
   // In non verbose mode, this should be the only output under normal
   //  circumstances:
   std::cout << HashToHex( hash );
   if( opt_print_time ) std::cout << "\n";

   auto end_time = std::chrono::steady_clock::now();
   if( opt_print_time ) {
      auto time = std::chrono::duration_cast<std::chrono::milliseconds>
                  ( end_time - start_time ).count();
      
      std::cout << "Time elapsed: " << time << "ms\n";
   }
   return 0;
}

} /////////////////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] ) { return Treehash::Run( argc, argv ); }
