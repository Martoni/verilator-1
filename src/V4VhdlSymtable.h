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

  virtual string to_string() {
    return m_name;
  }

  virtual string getItemClass() {
    return "";
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

  virtual string getItemClass() {
    return "";
  }

private:
    VhdlScope *m_type;
};

class VhdlTypeScope : public VhdlScope {
public:
  VhdlTypeScope(string name, VhdlScope *parent_type, bool isArray) : VhdlScope(name) {
    m_parent_type = parent_type;
    m_isArray = isArray;
  }

  VhdlScope *getParentType() {
    return m_parent_type;
  }

  virtual string getItemClass() {
    return "_type_";
  }

private:
    VhdlScope *m_parent_type;
    bool m_isArray;
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
    VhdlScope *slu = new VhdlTypeScope("std_ulogic", NULL, false);
    addItem(slu);

    VhdlScope *sl = new VhdlTypeScope("std_logic", slu, false);
    addItem(sl);

    VhdlScope *slv = new VhdlTypeScope("std_logic_vector", sl, true);
    addItem(slv);

    VhdlScope *sluv = new VhdlTypeScope("std_ulogic_vector", sl, true);
    addItem(sluv);

    VhdlScope *sig = new VhdlTypeScope("signed", sl, true);
    addItem(sig);

    VhdlScope *unsig = new VhdlTypeScope("unsigned", sl, true);
    addItem(unsig);

    /*addItem(new VhdlScope("_fn_and_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_or_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_xor_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_nand_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_nor_std__logic_std__logic_std__logic"));
    addItem(new VhdlScope("_fn_xnor_std__logic_std__logic_std__logic")); */
  }

  void addItem(VhdlScope *element) {
    if(element) {
      string itemClass = element->getItemClass();
      string caseInsensitiveId = itemClass + boost::to_lower_copy(escapeName(element->to_string()));
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
