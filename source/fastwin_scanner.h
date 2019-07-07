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
class FastwinScanner : public Scanner {

   static constexpr int MAX_EXT_FILTERS    = 64;
   static constexpr int MAX_IGNORE_FILTERS = 64;

   __m128i m_ext_masks[MAX_EXT_FILTERS];
   __m128i m_exts[MAX_EXT_FILTERS];
   int m_ext_filters = 0;
   //__declspec(align(32)) uint64_t m_exts_masks[MAX_EXT_FILTERS][3] = {0};
   //__declspec(align(32)) uint64_t m_exts[MAX_EXT_FILTERS][3] = {0};
   wchar_t m_ignores[MAX_PATH][MAX_IGNORE_FILTERS] = {0};
   WIN32_FIND_DATAW m_find_data;
   
   // Extra wide to make it extra-safe.
   wchar_t m_path_prepadding[32] = {0};
   wchar_t m_current_path[MAX_PATH*2];
   int m_path_start;
   
   inline bool IsExcluded( const wchar_t *path_short, const wchar_t *path_end,
                                                 bool is_directory ) noexcept {
      if( path_short[0] == L'.' ) return true;

      // Extensions
      if( !is_directory && m_ext_filters > 0 ) {
         __m128i ext = _mm_loadu_si128(
                          reinterpret_cast<const __m128i*>( path_end - 16 ));
         
         for( int i = 0; i < m_ext_filters; i++ ) {
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
      return false;

   }

   Hash ScanInner( wchar_t *path_start ) {
      path_start[0] = '*';
      path_start[1] = 0;

      HANDLE handle = FindFirstFile( m_current_path, &m_find_data );
      if( handle == INVALID_HANDLE_VALUE ) return 0;

      Hash hash = 0;
      
      do {
         wchar_t *path_end = path_start;
         for( int i = 0; *path_end++ = m_find_data.cFileName[i]; i++ );
         
         if( m_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
            if( IsExcluded( path_start, path_end, true )) continue;
            if( m_find_data.cFileName[0] == L'.' ) continue;
            *path_end++ = '\\';
            *path_end++ = '0';
            hash ^= ScanInner( path_end+1 );
         } else {
            if( IsExcluded( path_start, path_end, false )) continue;

            hash ^= XXH64( path_start
                         , (path_end - path_start) * sizeof(path_start[0])
                         , HASH_SEED );
         }
      } while( FindNextFile( handle, &m_find_data ));

      return hash;
   }

};


} /////////////////////////////////////////////////////////////////////////////