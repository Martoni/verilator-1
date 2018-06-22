#ifndef _V4VHDLPARSER_H_
#define _V4VHDLPARSER_H_

#include <string>
using namespace std;

class V4VhdlParser {
public:
  V4VhdlParser();
  void parseFile(const string& filename);
};

#endif
