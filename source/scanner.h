// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "hash.h"

namespace Treehash {

///////////////////////////////////////////////////////////////////////////////
class Scanner {

protected:
   bool m_recursive = false;

public:
   virtual ~Scanner() noexcept;
   virtual void ResetExts() noexcept;
   virtual void ResetIgnores() noexcept;
   virtual void AddExt( std::string_view ext ) noexcept;
   virtual void AddIgnore( std::string_view ignore ) noexcept;

   virtual Hash Scan( std::string_view path ) noexcept;

   void SetRecursive( bool recursive ) noexcept {
      m_recursive = recursive;
   }
};

}