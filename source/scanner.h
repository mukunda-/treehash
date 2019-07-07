// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "hash.h"

#include <string_view>

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

//-----------------------------------------------------------------------------
class Scanner {

public:
   virtual ~Scanner() noexcept = default;
   virtual void ResetExts() noexcept = 0;
   virtual void ResetIgnores() noexcept = 0;
   virtual void AddExt( std::string_view ext ) noexcept = 0;
   virtual void AddIgnore( std::string_view ignore ) noexcept = 0;

   virtual Hash Scan( std::string_view path, bool recursive ) noexcept = 0;
};

//-----------------------------------------------------------------------------
std::shared_ptr<Scanner> CreateScanner( std::string_view type ) noexcept;

} /////////////////////////////////////////////////////////////////////////////
