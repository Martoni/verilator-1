#include "V4VhdlParser.h"
#include "V4VhdlTranslate.h"

V4VhdlParser::V4VhdlParser(AstNetlist* rootp, VhdlScopeTable* symt) {
  m_rootp = rootp;
  m_scopeTable = symt;
}

void V4VhdlParser::parseFile(const string& filename) {
  cout << "Parsing " << filename << endl;

  ifstream istr;
  istr.open(filename);
  if (!istr) {
    cout << "Failed to open " << filename << endl;
  }

  ANTLRInputStream input_stream(istr);
  vhdlLexer lexer(&input_stream);
  CommonTokenStream tokens(&lexer);

  vhdlParser parser(&tokens);
  tree::ParseTree* tree = parser.design_file();
  cout << tree->toStringTree(&parser) << endl << endl;

  VhdlTranslateVisitor vhdlTranslate(filename, m_rootp, m_scopeTable);
  vhdlTranslate.visitDesign_file((vhdlParser::Design_fileContext*)tree);
}
