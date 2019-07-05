// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "version.h"

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

const std::string USAGE {R"==(
treehash v{VERSION}

$ treehash [OPTIONS] inputs...
)=="};

void PrintUsage() {
   auto &out = std::cout;
   std::string usage = USAGE;
   usage = std::regex_replace( usage, std::regex( "\\{VERSION\\}" ), VERSION );
   out << usage;
   
}

} /////////////////////////////////////////////////////////////////////////////
