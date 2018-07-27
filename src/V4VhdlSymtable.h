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
protected:
  string m_name; // Item name

  string escapeName(string name) {
    regex original("_");
    return regex_replace(name, original, "__");
  }

public:
  VhdlScope(string name) {
    m_name = name;
  }

  typedef AstNode*(*translationFnType)(FileLine*);

  virtual string mangled_name() = 0;

  virtual string to_string() {
    return m_name;
  }

  virtual string getItemClass() = 0;
  virtual AstNode *translate() = 0;
};

class VhdlTypeScope : public VhdlScope {
public:
  VhdlTypeScope(string name, VhdlTypeScope *parent_type, bool isArray) : VhdlScope(name) {
    m_parent_type = parent_type;
    m_isArray = isArray;
  }

  virtual string mangled_name() {
    return getItemClass() + escapeName(m_name);
  }

  VhdlScope *getParentType() {
    return m_parent_type;
  }

  virtual string getItemClass() {
    return "_type_";
  }

  virtual AstNode *translate() {
    return NULL;
  }

private:
    VhdlTypeScope *m_parent_type;
    bool m_isArray;
};

class VhdlVarScope : public VhdlScope {
public:
  VhdlVarScope(string name, VhdlTypeScope *type) : VhdlScope(name) {
    m_type = type;
  }

  virtual string mangled_name() {
    return escapeName(m_name);
  }

  virtual VhdlTypeScope * getType() {
    return m_type;
  }

  virtual string getItemClass() {
    return "";
  }

  virtual AstNode * translate() {
    return NULL;
  }

private:
    VhdlTypeScope *m_type;
};

class VhdlFnScope : public VhdlScope {
public:
  VhdlFnScope(string name, VhdlTypeScope *return_type, VhdlTypeScope **params_type, unsigned int params_count) : VhdlScope(name) {
    m_return_type = return_type;
    m_params_type = params_type;
    m_params_count = params_count;
  }

  virtual string mangled_name() {
    string mangle_name = getItemClass() + escapeName(m_name);
    mangle_name += "_" + escapeName(m_return_type->to_string());
    for (int i=0; i < m_params_count; i++) {
      mangle_name += "_" + escapeName(m_params_type[i]->to_string());
    }
    return mangle_name;
  }

  virtual string to_string() {
    return m_name;
  }

  virtual string getItemClass() {
    return "_fn_";
  }

  virtual AstNode *translate() {
    return NULL;
  }

private:
    VhdlTypeScope *m_return_type;
    VhdlTypeScope **m_params_type;
    unsigned int m_params_count;
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
    auto slu = new VhdlTypeScope("std_ulogic", NULL, false);
    addItem(slu);

    auto sl = new VhdlTypeScope("std_logic", slu, false);
    addItem(sl);

    auto slv = new VhdlTypeScope("std_logic_vector", sl, true);
    addItem(slv);

    auto sluv = new VhdlTypeScope("std_ulogic_vector", sl, true);
    addItem(sluv);

    auto sig = new VhdlTypeScope("signed", sl, true);
    addItem(sig);

    auto unsig = new VhdlTypeScope("unsigned", sl, true);
    addItem(unsig);

    VhdlTypeScope* sl_sl[] = {sl, sl};
    addItem(new VhdlFnScope("and", sl, sl_sl, 2));
    addItem(new VhdlFnScope("or", sl, sl_sl, 2));
    addItem(new VhdlFnScope("xor", sl, sl_sl, 2));
    addItem(new VhdlFnScope("nand", sl, sl_sl, 2));
    addItem(new VhdlFnScope("nor", sl, sl_sl, 2));
    addItem(new VhdlFnScope("xnor", sl, sl_sl, 2));
  }

  void addItem(VhdlScope *element) {
    if(element) {
      string itemClass = element->getItemClass();
      string caseInsensitiveId = boost::to_lower_copy(element->mangled_name());
      //cout << "Declaring " << caseInsensitiveId << endl;
      if ( scopes.find(caseInsensitiveId) == scopes.end() ) {
        scopes[caseInsensitiveId] = element;
      } else {
        cout << "Element " << caseInsensitiveId << " already defined" << endl;
      }
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

  VhdlTypeScope* searchType(string name) {
    string typeName = boost::to_lower_copy("_type_" + escapeName(name));
    //cout << "Lookup for " << typeName << endl;
    if ( scopes.find(typeName) == scopes.end() ) {
      return NULL;
    } else {
      return (VhdlTypeScope*)scopes[typeName];
    }
  }

  VhdlScope* searchFn(string name, VhdlTypeScope *return_type, VhdlTypeScope **params_type, unsigned int params_count) {
    string fnName = "_fn_" + escapeName(name);
    fnName += "_" + escapeName(return_type->to_string());
    for (int i=0; i < params_count; i++) {
      fnName += "_" + escapeName(params_type[i]->to_string());
    }
    //cout << "Lookup for " << typeName << endl;
    if ( scopes.find(fnName) == scopes.end() ) {
      return NULL;
    } else {
      return scopes[fnName];
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
