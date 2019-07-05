// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "hash.h"
#include "hash/xxh3.h"

#include <fstream>
#include <unordered_set>

namespace fs = std::filesystem;

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

constexpr Hash HASH_SEED = 0;

std::unordered_set<std::string> g_excluded_extensions;
std::unordered_set<std::string> g_excluded_files;

std::string HashHex( Hash hash ) {
   std::string output;
   output.reserve( 16 );
   const char digit_map[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', 
      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
   };
   for( int i = 15; i >= 0; i-- ) {
      int digit = (hash >> (4 * i)) & 0xF;
      output.push_back( digit_map[digit] );
   }
   return output;
}

//-----------------------------------------------------------------------------
bool IsExcluded( const fs::path &path ) {
   // Ignore files that start with "."
   std::string filename = path.filename().string();

   if( filename[0] == '.' ) {
      return true;
   }

   // Ignore files that have an excluded extension.
   if( g_excluded_extensions.find( path.extension().string() ) != g_excluded_extensions.end() ) {
      return true;
   }

   // Ignore files that match the pattern specified.
   if( g_excluded_files.find( filename ) != g_excluded_files.end() ) {
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------
Hash AddFolder( Hash hash, fs::path path, bool recursive ) {
   for( auto &p : fs::directory_iterator( path )) {
      if( p.is_directory() && recursive ) {
         hash ^= AddFolder( hash, p, recursive );
      } else if( p.is_regular_file() ) {
         auto path = p.path().lexically_relative( opt_basepath );
         if( IsExcluded( path )) continue;

         std::string file = path.generic_string();
         hash ^= XXH64( file.data(), file.size(), 0 );
         std::cout << file << "\n";
      }
   }
   return hash;
}


//-----------------------------------------------------------------------------
void AddExclude( std::string pattern ) {
   if( pattern.empty() ) return;

   if( pattern[0] == '.' ) {
      g_excluded_extensions.insert( pattern );
   } else {
      g_excluded_files.insert( pattern );
   }
}

//-----------------------------------------------------------------------------
void ResetExcludes() {
   g_excluded_extensions.clear();
   g_excluded_files.clear();

   for( auto &exclude : opt_excludes ) {
      AddExclude( exclude );
   }
}

//-----------------------------------------------------------------------------
Hash ProcessInputFile( std::string path ) {
   std::ifstream file( path );
   std::string line;

   Hash hash = HASH_SEED;

   ResetExcludes();

   enum {
      FOLDERS, EXCLUDES
   } input_mode;

   while( std::getline( file, line )) {
      if( line == "[files]" ) {
         input_mode = FOLDERS;
         continue;
      }

      line = trim(line);
      if( line.empty() ) continue;

      if( input_mode == FOLDERS ) {
         bool recursive = false;
         if( line[line.size()-1] == '*' ) {
            recursive = true;
            line.pop_back();
         }
         hash ^= AddFolder( hash, line, recursive );
      } else if( input_mode == EXCLUDES ) {
         AddExclude( line );
      }
   }

   return 0;
}

//-----------------------------------------------------------------------------
Hash HashInput( std::string input ) {

   if( fs::is_regular_file( input )) {
      // Input file
      return ProcessInputFile( input );
   } else if( fs::is_directory( input )) {
      // Directory
      ResetExcludes();
      return AddFolder( 0, input, true );
   }
   
   std::cout << "Bad input.\n";
   std::exit( 1 );
}

}
