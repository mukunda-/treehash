// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include <cctype>
#include <functional>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {
   
//-----------------------------------------------------------------------------
inline std::string trim( const std::string &s ) {
   // Pushkoff @ stackoverflow
   std::string::const_iterator it = s.begin();
   while( it != s.end() && isspace(*it) ) it++;

   std::string::const_reverse_iterator rit = s.rbegin();
   while( rit.base() != it && isspace(*rit) ) rit++;

   return std::string( it, rit.base() );
}

//-----------------------------------------------------------------------------
inline void InplaceTrim( std::string *s ) {
   auto it = s->begin();
   while( it != s->end() && std::isspace(*it) ) it++;
   if( it != s->begin() ) s->erase( s->begin(), it );

   auto rit = s->rbegin();
   while( rit != s->rend() && std::isspace(*rit) ) rit++;
   if( rit != s->rbegin() ) s->erase( rit.base(), s->end() );
}

//-----------------------------------------------------------------------------
inline bool IsDelim( char c, char *delims ) {
   for( int i = 0; delims[i]; i++ ) {
      if( delims[i] == ' ' && std::isspace(c) ) return true;
      else if( delims[i] == c ) return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
inline void SplitForeach( const std::string &string_to_parse,
                   char *null_terminated_delimiters,
                   std::function<void( std::string &piece )> func ) {

   auto &delims = null_terminated_delimiters;
   auto &input  = string_to_parse;
   auto scan    = input.begin();

   while( scan != input.end() ) {
      auto start = scan;
      // Find not-delimiter.
      while( start != input.end() && IsDelim( *start, delims )) start++;
      // Break if found end.
      if( start == input.end() ) break;

      auto end = start + 1;
      // Find delimiter.
      while( end != input.end() && !IsDelim( *end, delims )) end++;

      // Parse out string, ignore if empty, and then continue with next
      //  scan.
      std::string piece( start, end );
      InplaceTrim( &piece );
      if( !piece.empty() ) func( piece );
      scan = end;
   }
}

template< typename L >
inline std::string PluralS( L count ) {
   return count == 1 ? "" : "s";
}

} /////////////////////////////////////////////////////////////////////////////
