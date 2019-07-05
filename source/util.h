// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include <cctype>

namespace Treehash {
   
//-----------------------------------------------------------------------------
std::string trim( const std::string &s ) {
   // Pushkoff @ stackoverflow
   std::string::const_iterator it = s.begin();
   while( it != s.end() && isspace(*it) ) it++;

   std::string::const_reverse_iterator rit = s.rbegin();
   while( rit.base() != it && isspace(*rit) ) rit++;

   return std::string( it, rit.base() );
}

//-----------------------------------------------------------------------------
void InplaceTrim( std::string *s ) {
   auto it = s->begin();
   while( it != s->end() && std::isspace(*it) ) it++;
   if( it != s->begin() ) s->erase( s->begin(), it );

   auto rit = s->rbegin();
   while( rit != s->rend() && std::isspace(*rit) ) rit++;
   if( rit != s->rbegin() ) s->erase( rit.base(), s->end() );
}

} /////////////////////////////////////////////////////////////////////////////
