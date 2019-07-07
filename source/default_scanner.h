// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "scanner.h"
#include "hash.h"
#include "options.h"

#include <filesystem>
#include <unordered_set>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

//-----------------------------------------------------------------------------
class DefaultScanner : public Scanner {
//-----------------------------------------------------------------------------
   std::unordered_set<std::string_view> m_exts;
   std::unordered_set<std::string_view> m_ignores;

   //--------------------------------------------------------------------------
   inline bool IsExcluded( const std::filesystem::path &path,
                                                    bool directory ) noexcept {
      // Ignore files that start with "."
      std::string filename = path.filename().string();

      if( filename[0] == '.' ) {
         return true;
      }

      if( !directory ) {
         // Ignore files that have an excluded extension.
         
         if( !m_exts.empty() 
              && m_exts.find( path.extension().string() ) == m_exts.end() ) {
            return true;
         }
      }

      // Ignore files that match the files specified.
      for( auto &i : m_ignores ) {
         if( filename == i ) return true;
         if( path.generic_string() == i ) return true;
      }

      return false;
   }

   //--------------------------------------------------------------------------
   Hash ScanRecursion( const std::filesystem::path &path ) noexcept {
      Hash hash = 0;
      Hash hash = 0;
      namespace fs = std::filesystem;

      std::error_code error_code;
      fs::directory_iterator iter( path
                        , fs::directory_options::follow_directory_symlink
                        | fs::directory_options::skip_permission_denied
                        , error_code );

      if( error_code ) return;
      
      for( auto &file : iter ) {
         if( file.is_directory() && m_recursive ) {
            if( IsExcluded( file.path(), true )) continue;
            return ScanRecursion( file );
         } else {
            auto path = file.path();
            if( IsExcluded( file.path(), false )) {
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
   
public:
   //--------------------------------------------------------------------------
   Hash Scan( std::string_view path ) noexcept override {
      std::error_code error_code;
      std::filesystem::current_path( opt_basepath, error_code );
      if( error_code ) {
         std::cout << "Error with basepath.";
         return 0;
      }
      return ScanRecursion( path );
   }

   //--------------------------------------------------------------------------
   void ResetExts() noexcept override {
      m_exts.clear();

      for( auto &e : opt_exts )
         m_exts.insert( e );
   }

   //--------------------------------------------------------------------------
   void ResetIgnores() noexcept override {
      m_ignores.clear();

      for( auto &i : opt_ignores )
         m_ignores.insert( i );
   }

   //--------------------------------------------------------------------------
   void AddExt( std::string_view ext ) noexcept override {
      m_exts.insert( ext );
   }

   //--------------------------------------------------------------------------
   void AddIgnore( std::string_view ignore ) noexcept override {
      m_ignores.insert( ignore );
   }

   //--------------------------------------------------------------------------
   DefaultScanner(){
      ResetExts();
      ResetIgnores();
   }
};

} /////////////////////////////////////////////////////////////////////////////
