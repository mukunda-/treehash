// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

void ReadOptions( int argc, char **argv );

inline bool opt_verbose  = false;
inline bool opt_symlinks = false;
inline std::string opt_basepath;
inline std::vector<std::string> opt_inputs;

inline std::vector<std::string> opt_exts;
inline std::vector<std::string> opt_ignores;

} /////////////////////////////////////////////////////////////////////////////