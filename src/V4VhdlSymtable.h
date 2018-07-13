#ifndef _V4VHDLSYMTABLE_H_
#define _V4VHDLSYMTABLE_H_

#include <vector>
#include <map>
#include <string>
#include <boost/algorithm/string.hpp>
#include <string>

// Trash this
#include <iostream>

using namespace std;

class VhdlScope {
private:
  string m_name; // Item name

public:
  VhdlScope(string name) {
    m_name = name;
  }

  string getName() {
    return m_name;
  }
};

class VhdlVarScope : public VhdlScope {
public:
  VhdlVarScope(string name, VhdlScope *type) : VhdlScope(name) {
    m_type = type;
  }

  VhdlScope *getType() {
    return m_type;
  }

private:
    VhdlScope *m_type;
};

class VhdlScopeTable {
private:
  map<string, VhdlScope*> scopes;

public:
  VhdlScopeTable() {
    addItem(new VhdlScope("_type_std_logic"));
    addItem(new VhdlScope("_fn_and_std_logic_std_logic_std_logic"));
    addItem(new VhdlScope("_fn_or_std_logic_std_logic_std_logic"));
    addItem(new VhdlScope("_fn_xor_std_logic_std_logic_std_logic"));
    addItem(new VhdlScope("_fn_nand_std_logic_std_logic_std_logic"));
    addItem(new VhdlScope("_fn_nor_std_logic_std_logic_std_logic"));
    addItem(new VhdlScope("_fn_xnor_std_logic_std_logic_std_logic"));
  }

  void addItem(VhdlScope *element) {
    if(element) {
      string caseInsensitiveId = boost::to_lower_copy(element->getName());
      scopes[caseInsensitiveId] = element;
    }
  }

  VhdlScope* searchItem(string name) {
    string caseInsensitiveId = boost::to_lower_copy(name);
    if ( scopes.find(caseInsensitiveId) == scopes.end() ) {
      return NULL;
    } else {
      return scopes[caseInsensitiveId];
    }
  }

  VhdlScope* searchType(string name) {
    string typeName = boost::to_lower_copy("_type_" + name);
    if ( scopes.find(typeName) == scopes.end() ) {
      return NULL;
    } else {
      return scopes[typeName];
    }
  }
};

#endif
