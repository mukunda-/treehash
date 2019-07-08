// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "options.h"
#include "scanner.h"
#include "default_scanner.h"
#include "fastwin_scanner.h"

#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

std::shared_ptr<Scanner> CreateScanner( std::string_view type ) noexcept {
   if( type == "default" ) {
      if( opt_verbose )
         std::cout << "Creating default scanner.\n";
      return std::make_shared<DefaultScanner>();
   }

#ifdef TARGET_WINDOWS
   if( type == "fastwin" ) {
      if( opt_verbose )
         std::cout << "Creating fastwin scanner.\n";
      return std::make_shared<FastwinScanner>();
   }
#endif
   
   if( opt_verbose )
      std::cout << "Scanner of type \"" << type << "\" isn't supported."
                << " Creating default scanner.\n";
   return std::make_shared<DefaultScanner>();
}

} /////////////////////////////////////////////////////////////////////////////