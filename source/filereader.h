// filereader (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <ifstream>
#include <filesystem>

///////////////////////////////////////////////////////////////////////////////
class FileReader {
   ifstream file;

   FileReader( std::string path ) : file( path ) {}

   std::string GetLine() {
      
   }
};