// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "hash/xxh3.h"

namespace fs = std::filesystem;
namespace fs = std::filesystem;

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

uint64_t g_hash = 0x00000000;

void AddFolder( fs::path path, bool recursive ) {
   for( auto &p : fs::directory_iterator( path )) {
      if( p.is_directory() && recursive ) {
         AddFolder( p, recursive );
      } else if( p.is_regular_file() ) {
         std::string file = p.path().lexically_relative( opt_basepath ).generic_string();
         g_hash = g_hash ^ XXH64( file.data(), file.size(), 0 );
         std::cout << file << "\n";
      }
   }
}

void ParseInputFile( std::string path ) {

}

void Hash( std::string input ) {

   if( fs::is_regular_file( input )) {
      // Input file
      ParseInputFile( input );
   } else if( fs::is_directory( input )) {
      // Directory
      AddFolder( input, true );
   }
   
   //std::cout << XXH64( g_hash.data(), g_hash.size(), 0 );
}

}
