#ifndef _V4VHDLPARSER_H_
#define _V4VHDLPARSER_H_

#include "antlr4-runtime.h"
#include "vhdlLexer.h"
#include "vhdlParser.h"
#include "V3Ast.h"
#include "V3FileLine.h"
#include "V4VhdlSymtable.h"
#include <iostream>
#include <string>

using namespace std;
using namespace antlr4;

class V4VhdlParser {
public:
  V4VhdlParser(AstNetlist* rootp, VhdlScopeTable* symt);
  void parseFile(const string& filename);

private:
  AstNetlist* m_rootp;
  VhdlScopeTable *m_scopeTable;
};

#endif
