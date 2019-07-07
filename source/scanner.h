// treehash (C) 2019 Mukunda Johnson (mukunda@mukunda.com)
///////////////////////////////////////////////////////////////////////////////

namespace Treehash {
class Scanner {

public:
   virtual ~Scanner();
   virtual void AddExt( std::string ext );
   virtual void AddIgnore( std::string ignore );
}

}