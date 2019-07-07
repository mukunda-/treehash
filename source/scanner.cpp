// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "scanner.h"
#include "default_scanner.h"
#include "fastwin_scanner.h"

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

std::shared_ptr<Scanner> CreateScanner( std::string_view type ) noexcept {
   if( type == "default" ) {
      return std::make_shared<DefaultScanner>();
   }

#ifdef TARGET_WINDOWS
   if( type == "fastwin" ) {
      return std::make_shared<FastwinScanner>();
   }
#endif
   
   return std::make_shared<DefaultScanner>();
}

} /////////////////////////////////////////////////////////////////////////////