// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
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

} /////////////////////////////////////////////////////////////////////////////