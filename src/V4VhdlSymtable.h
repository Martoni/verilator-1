#ifndef _V4VHDLSYMTABLE_H_
#define _V4VHDLSYMTABLE_H_

#include <vector>
#include <map>
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
    cout << "Declaring " << name << endl;
  }

  string getName() {
    return m_name;
  }
};

class VhdlScopeTable {
private:
  map<string, VhdlScope*> scopes;

public:
  void addItem(VhdlScope *element) {
    if(element)
      scopes[element->getName()] = element;
  }

  VhdlScope* searchItem(string name) {
    if ( scopes.find(name) == scopes.end() ) {
      return NULL;
    } else {
      return scopes[name];
    }
  }

};

#endif
