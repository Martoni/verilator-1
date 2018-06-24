#ifndef _V4VHDLPARSER_H_
#define _V4VHDLPARSER_H_

#include "antlr4-runtime.h"
#include "vhdlLexer.h"
#include "vhdlParser.h"
#include <iostream>
#include <string>

using namespace std;
using namespace antlr4;

class V4VhdlParser {
public:
  void parseFile(const string& filename);
};

#endif
