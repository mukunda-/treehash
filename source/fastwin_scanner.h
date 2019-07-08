// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Windows only.
#ifndef TARGET_WINDOWS
#  endinput
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <xmmintrin.h>


///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

//-----------------------------------------------------------------------------
// A fast Windows directory scanner.
// Here be dragons.
class FastwinScanner : public Scanner {
   
   static constexpr int MAX_EXT_FILTERS    = 64;
   static constexpr int MAX_IGNORE_FILTERS = 64;
   static constexpr int PATHSIZE = 32768;
   static constexpr int IGNORE_FIELDSIZE = 64;

   __m128i m_ext_masks[MAX_EXT_FILTERS];
   __m128i m_exts[MAX_EXT_FILTERS];
   int m_ext_count = 0;
   //__declspec(align(32)) uint64_t m_exts_masks[MAX_EXT_FILTERS][3] = {0};
   //__declspec(align(32)) uint64_t m_exts[MAX_EXT_FILTERS][3] = {0};
   wchar_t m_ignores[IGNORE_FIELDSIZE][MAX_IGNORE_FILTERS] = {0};
   int m_ignore_count = 0;
   WIN32_FIND_DATAW m_find_data;
   
   // Extra wide to make it extra-safe.
   wchar_t m_path_prepadding[32] = {0};
   wchar_t m_current_path[PATHSIZE];
   int m_path_start;
   bool m_recursive;
   
   inline bool IsExcluded( const wchar_t *path_short, const wchar_t *path_end,
                                                 bool is_directory ) noexcept {
      //if( path_short[0] == L'.' ) return true;

      // Extensions
      if( !is_directory && m_ext_count > 0 ) {
         __m128i ext = _mm_loadu_si128(
                          reinterpret_cast<const __m128i*>( path_end - 16 ));
         
         // Here be dragons.
         for( int i = 0; i < m_ext_count; i++ ) {
            __m128i masked =_mm_and_si128( ext, m_ext_masks[i] );
            __m128i neq = _mm_xor_si128( masked, m_exts[i] );
            if( _mm_test_all_zeros( neq, neq )) {
               goto found_matched_extension;
            }
         }
         return true;
      }

   found_matched_extension:
      // Ignores

      for( int i = 0; i < m_ignore_count; i++ ) {
         wchar_t *ignorestring = m_ignores[i];
         if( lstrcmpW( path_short, ignorestring ) == 0 ) {
            return true;
         }
         if( lstrcmpW( m_current_path, ignorestring ) == 0 ) {
            return true;
         }
      }
      return false;

   }

   Hash ScanInner( wchar_t *path_start ) {
      path_start[0] = '*';
      path_start[1] = 0;

      HANDLE handle = FindFirstFileEx( m_current_path, FindExInfoBasic
                         , &m_find_data, FindExSearchNameMatch
                         , NULL, 0 );
      if( handle == INVALID_HANDLE_VALUE ) return 0;

      Hash hash = 0;
      
      do {/*
         int length = 0;
         for( ; m_find_data.cFileName[length]; ) length++;
         wchar_t *path_end = path_start + length;

         // length+1 to copy null terminator as well.
         memcpy( path_start, m_find_data.cFileName, (length+1) * sizeof(wchar_t) );*/
         
         if( m_find_data.cFileName[0] == L'.' ) continue;

         wchar_t *path_end = path_start;
         wchar_t *copyname = m_find_data.cFileName;
         while( *copyname ) {
            *path_end++ = *copyname++;
         } while( *copyname );
         *path_end = 0;
         
         if( m_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                                                             && m_recursive ) {
            if( IsExcluded( path_start, path_end, true )) continue;
            //if( m_find_data.cFileName[0] == L'.' ) continue;
            *path_end++ = '\\';
            // Don't need null terminator here as it's added in ScanInner.
            hash ^= ScanInner( path_end );
         } else {
            if( IsExcluded( path_start, path_end, false )) continue;

            hash ^= XXH64( m_current_path
                         , (path_end - m_current_path) * sizeof(*m_current_path)
                         , HASH_SEED );
         }
      } while( FindNextFile( handle, &m_find_data ));

      // Unsafe?
      FindClose( handle );

      return hash;
   }

public:
   
   //--------------------------------------------------------------------------
   void AddExt( std::string_view ext ) noexcept override {
      if( m_ext_count == MAX_EXT_FILTERS ) {
         if( opt_verbose )
            std::cout << "Too many extension filters.\n";
         return;
      }
      wchar_t work[8] = {0};
      int length;
      if( !(length = MultiByteToWideChar( CP_UTF8, 0
             , ext.data(), static_cast<int>(ext.size()), m_current_path, 8 ))) {
         if( opt_verbose ) {
            std::cout
               << "Extension string is too large for fastwin scanner: "
               << ext << "\n";
         }
         return;
      }
      
      // Here be dragons...
      __m128i mask = _mm_set1_epi8( (char)0xFF );
      __m128i mext = _mm_loadu_si128( reinterpret_cast<const __m128i*>( work ));
      for( int i = 0; i < (int)ext.size(); i++ ) {
         mask = _mm_srli_si128( mask, 16 );
         mext = _mm_srli_si128( mext, 16 );
      }
      
      m_ext_masks[m_ext_count] = mask;
      m_exts[m_ext_count]      = mext;
      m_ext_count++;
   }

   //--------------------------------------------------------------------------
   void AddIgnore( std::string_view ignore ) noexcept override {
      if( m_ignore_count == MAX_IGNORE_FILTERS ) {
         if( opt_verbose )
            std::cout << "Too many ignore filters.\n";
         return;
      }
      //m_ignores.insert( ignore );
      wchar_t work[8] = {0};
      int length;
      if( !(length = MultiByteToWideChar( CP_UTF8, 0
             , ignore.data(), static_cast<int>(ignore.size())
             , m_ignores[m_ignore_count], IGNORE_FIELDSIZE-1 ))) {
         if( opt_verbose ) {
            std::cout
               << "Ignore string is too large for fastwin scanner: "
               << ignore << "\n";
         }
         return;
      }
      m_ignores[m_ignore_count][length] = 0;
      for( wchar_t *scan = m_ignores[m_ignore_count]; *scan; scan++ ) {
         if( *scan == L'/' ) *scan = L'\\';
      }
      m_ignore_count++;
   }

   //--------------------------------------------------------------------------
   FastwinScanner() {
      for( auto &e : opt_exts ) {
         AddExt( e );
      }
      for( auto &i : opt_ignores ) {
         AddIgnore( i );
      }
   }

   //--------------------------------------------------------------------------
   Hash Scan( std::string_view path, bool recursive ) noexcept override {
      int length;
      if( !(length = MultiByteToWideChar( CP_UTF8, 0
                     , path.data(), static_cast<int>(path.size())
                     , m_current_path, PATHSIZE ))) {
         std::cout << "Invalid path given.\n";
         return 0;
      }

      m_recursive = recursive;
      wchar_t *path_start = m_current_path + length;
      if( *(path_start-1) != L'\\' ) {
         *path_start++ = L'\\';
      }
      *path_start = 0;

      return ScanInner( path_start );
   }
   
   //--------------------------------------------------------------------------
   void ResetExts() noexcept override {
      m_ext_count = static_cast<int>(opt_exts.size());
   }

   //--------------------------------------------------------------------------
   void ResetIgnores() noexcept override {
      m_ignore_count = static_cast<int>(opt_ignores.size());
   }

};


} /////////////////////////////////////////////////////////////////////////////