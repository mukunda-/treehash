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

struct {
   std::unordered_set<std::string> exts;
   std::unordered_set<std::string> ignores;

   void Reset() {
      exts.clear();
      ignores.clear();

      for( auto &i : opt_exts )
         exts.insert( i );

      for( auto &i : opt_ignores ) 
         ignores.insert( i );
   }
} Filter;


//-----------------------------------------------------------------------------
std::string HashToHex( Hash hash ) {
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
   auto &exts = g_included_extensions;
   if( !exts.empty() && exts.find( path.extension().string() ) == exts.end() ) {
      return true;
   }

   // Ignore files that match the pattern specified.
   if( g_excluded_files.find( filename ) != g_excluded_files.end() ) {
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------
Hash AddFolder( fs::path path, bool recursive ) {
   Hash hash = 0;

   for( auto &p : fs::directory_iterator( path )) {
      if( p.is_directory() && recursive ) {
         hash ^= AddFolder( p, recursive );
      } else if( p.is_regular_file() ) {
         auto path = p.path().lexically_relative( opt_basepath );
         if( IsExcluded( path )) continue;

         std::string file = path.generic_string();
         hash ^= XXH64( file.data(), file.size(), HASH_SEED );
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
Hash ProcessFolder( std::string path ) {
   path = trim( path );
   if( path.empty() ) return 0;

   bool recursive = false;

   if( path[path.size()-1] == '*' ) {
      recursive = true;
      path.pop_back();
   }

   return AddFolder( path, true );
}

//-----------------------------------------------------------------------------
Hash ProcessInputFile( std::string path ) {
   std::ifstream file( path );
   std::string line;

   Hash hash = HASH_SEED;

   enum {
      FOLDERS, EXCLUDES
   } input_mode;

   while( std::getline( file, line )) {
      InplaceTrim( &line );
      if( line.empty() || line[0] == '#' ) continue;

      if( line == "[files]" ) {
         input_mode = FOLDERS;
         continue;
      } else if( line == "[excludes]" ) {
         input_mode = EXCLUDES;
         continue;
      }

      if( input_mode == FOLDERS ) {
         hash ^= ProcessFolder( line );
      } else if( input_mode == EXCLUDES ) {
         AddExclude( line );
      }
   }

   return 0;
}

//-----------------------------------------------------------------------------
Hash HashInput( std::string input ) {
   Filter.Reset();

   if( fs::is_regular_file( input )) {
      // Input file
      return ProcessInputFile( input );
   } else if( fs::is_directory( input )) {
      // Directory
      ResetExcludes();
      return ProcessFolder( input );
      
   }
   
   std::cout << "Bad input.\n";
   std::exit( 1 );
}

}
