#ifndef _V4VHDLSYMTABLE_H_
#define _V4VHDLSYMTABLE_H_

#include <vector>
#include <map>
#include <string>
#include <boost/algorithm/string.hpp>
#include <string>
#include <regex>

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

  string escapeName(string name) {
    regex original("_");
    return regex_replace(name, original, "__");
  }

public:
  VhdlScopeTable() {
    addItem(new VhdlScope("std_logic"), "_type_");
    /*addItem(new VhdlScope("_fn_and_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_or_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_xor_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_nand_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_nor_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_xnor_std__logic_std__logic_std__logic")); */
  }

  void addItem(VhdlScope *element) {
    addItem(element, "");
  }

  void addItem(VhdlScope *element, string itemClass) {
    if(element) {
      string caseInsensitiveId = itemClass + boost::to_lower_copy(escapeName(element->getName()));
      //cout << "Declaring " << caseInsensitiveId << endl;
      scopes[caseInsensitiveId] = element;
    }
  }

  VhdlScope* searchItem(string name) {
    string caseInsensitiveId = escapeName(boost::to_lower_copy(name));
    if ( scopes.find(caseInsensitiveId) == scopes.end() ) {
      return NULL;
    } else {
      return scopes[caseInsensitiveId];
    }
  }

  VhdlScope* searchType(string name) {
    string typeName = boost::to_lower_copy("_type_" + escapeName(name));
    //cout << "Lookup for " << typeName << endl;
    if ( scopes.find(typeName) == scopes.end() ) {
      return NULL;
    } else {
      return scopes[typeName];
    }
  }

  void dump() {
    cout << "VHDL symbols dump" << endl << "------------------------"<< endl;
    for(auto iter = scopes.begin(); iter != scopes.end(); ++iter) {
      cout << iter->first << endl;
    }
  }
};

#endif
