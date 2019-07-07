// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "hash/xxh3.h"

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {
   using Hash = uint64_t;
   constexpr Hash HASH_SEED = 0;

   class Scanner;
   Hash HashInput( std::string input, Scanner &scanner ) noexcept;
   std::string HashToHex( Hash hash ) noexcept;

} /////////////////////////////////////////////////////////////////////////////
