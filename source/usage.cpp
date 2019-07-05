// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#include "version.h"

///////////////////////////////////////////////////////////////////////////////
namespace Treehash {

const std::string USAGE {R"==(
~ treehash v{VERSION} (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
  
  Licensed under MIT.

-------------------------------------------------------------------------------
Usage:
 $ treehash [OPTIONS] inputs...

Inputs can be either folders or input list files (see manual).

Options arguments are passed as --option <arg> / -o <arg>.

OPTIONS:
 -b --base       Sets the directory (absolute or relative to the working
                 directory) that is used as the base for all other filenames
                 for inputs (especially for input lists).
                 
                   -b ../dev/    # Base directory is up one level and into the
                                 # dev folder, where for example the main
                                 # source tree might lie.

 -h --help       Prints this help.

 -e --exts       Any extensions that aren't given will be excluded from the
                 tree hash. If this is omitted, everything will match.
                 
                   -e .txt -e .cpp    # Only look for .txt or .cpp files
                   -e .txt|.cpp       # Same thing, shorthand.

 -i --ignore     Any filenames listed here will be ignored from the scan. Any
                 files beginning with a dot "." will always be ignored.
                 Examples:
                   -i CMakeThing.txt    # Ignore any files named this. That is,
                                        # if a file named this is created, it 
                                        # won't affect the tree hash.
                   -i dev/core/genie    # Ignore this folder specifically.

 -v --verbose    Using this option causes a lot of extra information to be spit
                 out, to allow you to diagnose what is going on when the trees
                 are hashed.

 -m --symlinks   Using this options causes any symlinks to be followed, which
                 are otherwise ignored.
       
-------------------------------------------------------------------------------
Example input list file (thingy.txt):

# Only include .txt files or .cpp file paths in the hash.
[exts] .txt .cpp
# Ignore any files named this
[exts] CMakeLists.txt
# Folder listing follows
dev/apps/fiddle
dev/core
dev/proto
dev/tests

-------------------------------------------------------------------------------
If for example we're in the "build" folder with thingy.txt nearby, we'd run
that file as:
 $ treehash --base .. thingy.txt
)=="};

void PrintUsage() {
   auto &out = std::cout;
   std::string usage = USAGE;
   usage = std::regex_replace( usage, std::regex( "\\{VERSION\\}" ), VERSION );
   out << usage;
   
}

} /////////////////////////////////////////////////////////////////////////////
