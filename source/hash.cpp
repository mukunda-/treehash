// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "hash.h"
#include "options.h"
#include "util.h"

#include <fstream>
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <regex>

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
Hash AddFolder( const fs::path &path, bool recursive ) {
   
   Hash hash = 0;

   std::error_code error_code;
   auto dir_iter = fs::directory_iterator( path
                        , fs::directory_options::follow_directory_symlink
                        | fs::directory_options::skip_permission_denied
                        , error_code );

   if( error_code ) {
      return 0;
   }

   for( auto &p : dir_iter ) {
      if( p.is_directory() && recursive ) {
         auto &path = p.path();
         if( IsExcluded( path, true )) continue;
         hash ^= AddFolder( p, recursive );
      } else if( p.is_regular_file() ) {
         auto path = p.path();
         if( IsExcluded( path, false )) {
            if( opt_verbose ) {
               std::string file = path.generic_string();
               std::cout << "   " << file << "\n";
            }
            continue;
         }

         auto &file = path.generic_string();
         hash ^= XXH64( file.data(), file.size(), HASH_SEED );

         if( opt_verbose ) {
            std::cout << " * " << file << "\n";
         }
      }
   }
   return hash;
}

//-----------------------------------------------------------------------------
bool StripRecurseMark( std::string *path ) {
   if( path->empty() ) return false;
   bool recursive = (*path)[path->size()-1] == '*';
   if( recursive ) path->pop_back();
   return recursive;
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
         bool recursive = StripRecurseMark( &line );
         hash ^= AddFolder( line, recursive );
      }
   }

   return hash;
}

//-----------------------------------------------------------------------------
Hash HashInput( std::string input ) {
   std::error_code ec;
   fs::current_path( opt_basepath, ec );
   if( ec ) {
      std::cout << "Error with basepath.\n";
      std::exit( 1 );
   }

   Filter.Reset();
   InplaceTrim( &input );
   if( input.empty() ) return 0;

   bool recurse = StripRecurseMark( &input );
   auto path = fs::relative( input );

   try {
      if( !fs::exists( path )) {
         return 0;
      }

      if( !recurse && fs::is_regular_file( path )) {
         if( opt_verbose )
            std::cout << "Input is an input file (or we think it is).\n";
         // Input file
         return ProcessInputFile( input );
      } else if( fs::is_directory( path )) {
         if( opt_verbose )
            std::cout << "Input is a directory. Scanning directly!\n";
         // Directory
         return AddFolder( path, recurse );
      }
   } catch( fs::filesystem_error &e ) {
      std::cout << "Filesystem error: " << e.what()
                << " (" << e.code() << ")\n";
   }
   
   std::cout << "Invalid input: " << input << "\n";
   std::exit( 1 );
}

}
