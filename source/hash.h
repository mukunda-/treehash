// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {
   using Hash = uint64_t;
   Hash HashInput( std::string input );
   std::string HashToHex( Hash hash );

} /////////////////////////////////////////////////////////////////////////////
