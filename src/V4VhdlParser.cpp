#include "V4VhdlParser.h"
#include "V4VhdlTranslate.h"

V4VhdlParser::V4VhdlParser(AstNetlist* rootp) {
  m_rootp = rootp;
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

  VhdlTranslateVisitor vhdlTranslate(filename, m_rootp);
  vhdlTranslate.visitDesign_file((vhdlParser::Design_fileContext*)tree);
}
