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
// A fast Windows directory scanner. Here be dragons.
// It's not actually that much faster than the default scanner. Most of the
//                                     time spent for either is with the IO.
class FastwinScanner : public Scanner {
//-----------------------------------------------------------------------------
   // Maximum number of extension filters. Will error and ignore if the user
   //  specifies more than this.
   static constexpr int MAX_EXT_FILTERS    = 64;
   //--------------------------------------------------------------------------
   // Maximum number of ignore filters. Will error and ignore if the user tries
   //  to add more than this.
   static constexpr int MAX_IGNORE_FILTERS = 64;
   //--------------------------------------------------------------------------
   // Max length for an ignored filename. Will error and ignore if there is an
   //  input larger than this.
   static constexpr int IGNORE_FIELDSIZE = 64;
   // The above filters if used in excess will also have a tendency to slow
   //  down the scanner quite a bit (maybe--most of it would still be IO).
   //--------------------------------------------------------------------------
   // Maximum length of paths we can search. Will probably blow up if there
   //                                   is a path that somehow exceeds this.
   static constexpr int PATHSIZE = 32768+4096;
   //--------------------------------------------------------------------------
   // Extension filters are stored as a 128-bit SSE2 values. The mask is used
   //  to determine the length of the comparison.
   // Input: foo\hello.world
   //               ^^^^^^^^ 8 chars are loaded, ending at the end of the input.
   //               lo.world
   // Extension for .world will be 00.world, and the mask will be 00FFFFFF
   //  (with each F being a whopping 16-bit 0xFFFF mask)
   // Transformation example using extension ".world":
   // foo\hello.world  # Initial input.
   // lo.world         # Load last 8 characters.
   // lo000000         # bitwise xor with extension value (00.world).
   // 00000000         # Mask by mask value (00FFFFFF - F = 0xFFFF whole wchar)
   // If the result is 0, then it matches the extension.
   __m128i m_ext_masks[MAX_EXT_FILTERS];
   __m128i m_exts[MAX_EXT_FILTERS];
   //--------------------------------------------------------------------------
   // The number of extension filters that have been registered.
   int m_ext_count = 0;
   //--------------------------------------------------------------------------
   // Ignores are just null terminated strings.
   wchar_t m_ignores[IGNORE_FIELDSIZE][MAX_IGNORE_FILTERS] = {0};
   int m_ignore_count = 0;
   //--------------------------------------------------------------------------
   // Shared FindXFile data structure.
   WIN32_FIND_DATAW m_find_data;
   //--------------------------------------------------------------------------
   // Just in case we try and read before the beginning of m_current_path (such
   //  as when over-reading an extension test), hopefully we read from here
   //  instead of anything else.
   wchar_t m_path_prepadding[32] = {0};
   //--------------------------------------------------------------------------
   // We work on the path variable in-place as we traverse the tree. It needs
   //  to be as wide as any path ever will be.
   wchar_t m_current_path[PATHSIZE];
   //--------------------------------------------------------------------------
   // True if this is a recursive search, set by the initial * in the input
   //  string.
   bool m_recursive;
   
   //--------------------------------------------------------------------------
   // `path_short` and `path_end` are both pointers into the `m_current_path`
   //  with `end` pointing to the null terminator (which should be guaranteed
   //  to be present), and `short` pointing to the first character of the
   //  filename part. `m_current_path` can also be used for the full filename
   //  with any number of trailing directories leading to the basepath.
   // `is_directory` is set if we're checking if a directory is excluded
   //  (basically skip extension matching).
   inline bool IsExcluded( const wchar_t *path_short, const wchar_t *path_end,
                                                 bool is_directory ) noexcept {
      // Directories skip extension matching. Here be dragons.
      if( !is_directory && m_ext_count > 0 ) {
         // Load the last 8 characters of the filename string. This can read
         //                             into data before the filename starts.
         __m128i ext = _mm_loadu_si128(
                          reinterpret_cast<const __m128i*>( path_end - 16 ));

         // For each extension, mask the input, and xor it against the pattern.
         // The xor is basically one of the only ways to compare equality
         //  between two m128 registers, using the SSE4.x test instruction
         //  after.
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
      // Ignores are much simpler and less optimized, but honestly I don't know
      //  of a good use case for ignores. Just don't clutter up your source
      //                                          tree with garbage, right?
      for( int i = 0; i < m_ignore_count; i++ ) {
         wchar_t *ignorestring = m_ignores[i];
         // lstrcmpW seems like a nasty thing to use but oh well.
         if( lstrcmpW( path_short, ignorestring ) == 0 ) {
            return true;
         }
         if( lstrcmpW( m_current_path, ignorestring ) == 0 ) {
            return true;
         }
      }
      return false;
   }

   //--------------------------------------------------------------------------
   // Basically strcpy. Returns the character after the end of the string.
   inline wchar_t *MoveString( wchar_t *dest, const wchar_t *source ) noexcept {
      while( *source ) {
         *dest++ = *source++;

         // Just imagine if someone were to design a virus that could
         //  miraculously fit inside a file path, that which would be committed
         //  to a source repo and infect anyone who tries to hash the source
         //  tree with it present.

         //if( dest >= m_current_path + PATHSIZE ) {
         //   std::terminate();
         //}
      }
      return dest;
   }

   //--------------------------------------------------------------------------
   // We accept a pointer into our shared path memory.
   Hash ScanInner( wchar_t *path_start ) noexcept {
      // FindFirstFile accepts a path+pattern string, appending an asterisk
      //  matches all files in a folder. There's likely no feasible alternative
      //  that will let you get away from this pattern-matching overhead.
      // Who knows how they implemented it anyway; maybe it's super efficient.
      // The bottleneck will always be the IO.
      path_start[0] = '*';
      path_start[1] = 0;

      // The last param can accept "FIND_FIRST_EX_LARGE_FETCH" which likely
      //  makes the scanner faster if looking at a directory with a ton of
      //  files. Could add an option for that, but I've had better performance
      //  results without that flag (likely because of using too much
      //  additional memory).
      HANDLE handle = FindFirstFileEx( m_current_path, FindExInfoBasic
                         , &m_find_data, FindExSearchNameMatch
                         , NULL, 0 );
      // FindFirstFileEx won't create a handle if no files exist in the folder
      //  or if the path is bad, etc. As far as I know, there will always be
      //  at least one or two matches, due to the directory references "." and
      //  "..".
      if( handle == INVALID_HANDLE_VALUE ) return 0;

      Hash hash = 0;
      
      do {
         // We always ignore things starting with dot. Just something we won't
         //  be flexible on.
         if( m_find_data.cFileName[0] == L'.' ) continue;

         // No way to avoid this copy. `path_end` points to after the end of
         //  the string copied. cFileName is the filename part of the match
         //  returned by the directory iterator. That is "hello.txt" without
         //  the directory name.
         wchar_t *path_end = MoveString( path_start, m_find_data.cFileName );
         *path_end = 0;
         
         // Ignore directories if we aren't in recursive mode.
         if( m_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                                                             && m_recursive ) {
            // Directory exclusions are only with the ignore list.
            if( IsExcluded( path_start, path_end, true )) continue;
            
            // Some recommend that you should buffer the directory list and
            //  recurse through them outside of this loop, but I don't think it
            //  matters. With testing the difference was negligible, in favor
            //  of not wasting time buffering the directories encountered.
            // It may have better performance with directories that have an
            //  absurd amount of files, but that can likely be improved with
            //  the FIND_FIRST_EX_LARGE_FETCH flag above.

            // Add a trailing slash and start next level of recursion.
            // Don't need to add a null terminator because it's added at the 
            //  start of ScanInner.
            *path_end++ = '\\';
            hash ^= ScanInner( path_end );
         } else {
            // File exclusions check extension and path and filename.
            if( IsExcluded( path_start, path_end, false )) continue;

            // The resulting hash is dependent on what scanner is used. In this
            //  case we're hashing wide strings with backslash separators.
            hash ^= XXH64( m_current_path
                         , (path_end - m_current_path) * sizeof(*m_current_path)
                         , HASH_SEED );
         }
      } while( FindNextFile( handle, &m_find_data ));

      // I had some basic RAII used for this before, but that was unnecessary
      //  clutter. I added `noexcept` to above instead.
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
         mask = _mm_slli_si128( mask, 16 );
         mext = _mm_slli_si128( mext, 16 );
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
      // TODO: There is a special token that you can prefix to paths,
      //  i.e. "\?\" or something, that allows the length of paths to be
      //  extended. There are also other ways to opt-in. Needs experimentation
      //  and research.
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