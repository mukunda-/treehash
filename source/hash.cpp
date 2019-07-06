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
   std::vector<std::string> ignores;

   void ResetExts() {
      exts.clear();
      for( auto &i : opt_exts ) {
         // _ = no extension.
         if( i == "_" ) exts.insert( "" );
         else exts.insert( i );
      }
   }

   void ResetIgnores() {
      ignores.clear();
      for( auto &i : opt_ignores ) {
         ignores.push_back( i );
      }
   }

   void Reset() {
      ResetExts();
      ResetIgnores();
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
bool IsExcluded( const fs::path &path, bool directory ) {
   // Ignore files that start with "."
   std::string filename = path.filename().string();

   if( filename[0] == '.' ) {
      return true;
   }

   if( !directory ) {
      // Ignore files that have an excluded extension.
      auto &exts = Filter.exts;
      if( !exts.empty() && exts.find( path.extension().string() ) == exts.end() ) {
         return true;
      }
   }

   // Ignore files that match the pattern specified.
   auto &ignores = Filter.ignores;
   for( auto &i : ignores ) {
      if( filename == i ) return true;
      if( path.generic_string() == i ) return true;
   }

   return false;
}

//-----------------------------------------------------------------------------
Hash AddFolder( fs::path path, bool recursive ) {
   Hash hash = 0;

   try {
      if( !fs::exists( path )) {
         //if( opt_ignore_missing ) return hash;
         return hash;
      }

      for( auto &p : fs::directory_iterator( path )) {
         if( p.is_directory() && recursive ) {
            auto path = p.path().lexically_relative( opt_basepath );
            if( IsExcluded( path, true )) continue;
         
            hash ^= AddFolder( p, recursive );
         } else if( p.is_regular_file() ) {
            auto path = p.path().lexically_relative( opt_basepath );
            if( IsExcluded( path, false )) {
               if( opt_verbose ) {
                  std::string file = path.generic_string();
                  std::cout << "   " << file << "\n";
               }
               continue;
            }

            std::string file = path.generic_string();
            hash ^= XXH64( file.data(), file.size(), HASH_SEED );

            if( opt_verbose ) {
               std::cout << " * " << file << "\n";
            }
         }
      }
      return hash;
   } catch( fs::filesystem_error &error ) {
      std::cout << "Encountered error: " << error.what()
                << " (" << error.code() << ")\n"; 
      return 0;
   }
}

//-----------------------------------------------------------------------------
Hash ProcessInputFolder( std::string path ) {
   InplaceTrim( &path );
   if( path.empty() ) return 0;

   bool recursive = false;
   if( path[path.size()-1] == '*' ) {
      recursive = true;
      path.pop_back();
   }

   return AddFolder( path, recursive );
}

std::regex re_inputfile_directive( R"(^\[([^]*)\])" );

//-----------------------------------------------------------------------------
Hash ProcessInputFile( std::string path ) {
   std::ifstream file( path );
   std::string line;

   Hash hash = HASH_SEED;

   while( std::getline( file, line )) {
      InplaceTrim( &line );
      if( line.empty() || line[0] == '#' ) continue;

      if( line[0] == '[' ) {
         std::smatch match;
         std::regex_search( line, match, re_inputfile_directive );
         if( !match.empty() ) {
            if( match[1] == "ext" || match[1] == "exts" || match[1] == "extensions" ) {
               Filter.ResetExts();
               SplitForeach( line.substr( 6 ), " |", []( std::string piece ) {
                  if( piece == "_" ) piece = "";
                  Filter.exts.insert( piece );
               });
            } else if( match[1] == "ignores" || match[1] == "ignore" ) {
               Filter.ResetIgnores();
               SplitForeach( line.substr( 9 ), "|", []( std::string piece ) {
                  Filter.ignores.push_back( piece );
               });
            }
         } else {
            if( opt_verbose ) {
               std::cout << "Unknown directive in input file: " << line << "\n";
            }
         }
      } else {
         hash ^= ProcessInputFolder( opt_basepath + "/" + line );
      }
   }

   return hash;
}

//-----------------------------------------------------------------------------
Hash HashInput( std::string input ) {
   Filter.Reset();
   InplaceTrim( &input );
   if( input.empty() ) return 0;
   std::string without_mods = input;
   bool recursive = input[input.size()-1] == '*';
   if( recursive ) {
      without_mods.pop_back();
   }

   try {
      if( !fs::exists( without_mods )) {
         return 0;
      }

      if( !recursive && fs::is_regular_file( without_mods )) {
         if( opt_verbose )
            std::cout << "Input is an input file (or we think it is).\n";
         // Input file
         return ProcessInputFile( input );
      } else if( fs::is_directory( without_mods )) {
         if( opt_verbose )
            std::cout << "Input is a directory. Scanning directly!\n";
         // Directory
         return ProcessInputFolder( input );
      }
   } catch( fs::filesystem_error &e ) {
      std::cout << "Filesystem error: " << e.what()
                << " (" << e.code() << ")\n";
   }
   
   std::cout << "Invalid input: " << input << "\n";
   std::exit( 1 );
}

}
