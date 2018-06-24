#include "V4VhdlParser.h"

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
}
