#ifndef _V4VHDLTRANSLATE_H_
#define _V4VHDLTRANSLATE_H_

#include "vhdl_parser/vhdlBaseVisitor.h"
#include "V3Ast.h"
#include <iostream>
using namespace std;

class VhdlTranslateVisitor: public vhdlBaseVisitor{
public:
  VhdlTranslateVisitor(const string &filename, AstNetlist *root) {
    m_filename = filename;
    m_rootp = root;
    pinNumber = 0;
  }

  virtual antlrcpp::Any visitEntity_declaration(vhdlParser::Entity_declarationContext *ctx) override {
    FileLine *fl = new FileLine(m_filename, ctx->identifier().front()->value->getLine());
    AstModule *modulep = new AstModule(fl, ctx->identifier().at(0)->value->getText());
    m_currentModule = modulep;
    m_rootp->addModulep(modulep);
    cout << "Declaration " << ctx->identifier().at(0)->value->getText() << " At line " << ctx->identifier().front()->value->getLine() << endl;
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArchitecture_body(vhdlParser::Architecture_bodyContext *ctx) override {
    cout << "Architecture " << ctx->identifier().at(0)->value->getText() << " of " << ctx->identifier().at(1)->value->getText() <<" At line " << ctx->identifier().front()->value->getLine() << endl;
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInterface_port_declaration(vhdlParser::Interface_port_declarationContext *ctx) override {
    for(auto port : ctx->identifier_list()->id_lst) {
      string portName = port->value->getText();
      FileLine *flPort = new FileLine(m_filename, port->value->getLine());
      AstPort *portp = new AstPort(flPort, pinNumber++, portName);
      m_currentModule->addStmtp(portp);

      FileLine *flType = new FileLine(m_filename, port->value->getLine());
      AstNodeDType *dtypep = new AstBasicDType(flType, AstBasicDTypeKwd::BIT);
      m_currentModule->addStmtp(dtypep);

      FileLine *flVar = new FileLine(m_filename, port->value->getLine());
      AstVar *varp;
      if (ctx->signal_mode()->IN())
        varp = new AstVar(flVar, AstVarType::INPUT, portName, dtypep);
      else if(ctx->signal_mode()->OUT())
        varp = new AstVar(flVar, AstVarType::OUTPUT, portName, dtypep);
      else if(ctx->signal_mode()->INOUT())
        varp = new AstVar(flVar, AstVarType::INOUT, portName, dtypep);
      else if(ctx->signal_mode()->BUFFER())
        varp = new AstVar(flVar, AstVarType::OUTPUT, portName, dtypep);
      else if(ctx->signal_mode()->LINKAGE())
        varp = new AstVar(flVar, AstVarType::INOUT, portName, dtypep);

      m_currentModule->addStmtp(varp);
    }
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSignal_declaration(vhdlParser::Signal_declarationContext *ctx) override {
    for(auto sig : ctx->identifier_list()->id_lst) {
      string sigName = sig->value->getText();
      cout << "Signal " << sigName << endl;
    }
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstant_declaration(vhdlParser::Constant_declarationContext *ctx) override {
    for(auto cst : ctx->identifier_list()->id_lst) {
      string cstName = cst->value->getText();
      cout << "Constant " << cstName << endl;
    }
    return visitChildren(ctx);
  }

private:
  string m_filename;
  AstNetlist *m_rootp;
  long pinNumber;
  AstModule *m_currentModule;

};

#endif
