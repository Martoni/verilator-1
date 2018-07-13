#ifndef _V4VHDLTRANSLATE_H_
#define _V4VHDLTRANSLATE_H_

#include "V4VhdlSymtable.h"
#include "vhdl_parser/vhdlBaseVisitor.h"
#include "V3Ast.h"
#include <iostream>
using namespace std;

class VhdlTranslateVisitor: public vhdlBaseVisitor{
public:
  VhdlTranslateVisitor(const string &filename, AstNetlist *root, VhdlScopeTable *scopeTable) {
    m_filename = filename;
    m_rootp = root;
    pinNumber = 0;
    m_scopeTable = scopeTable;
  }

  virtual antlrcpp::Any visitEntity_declaration(vhdlParser::Entity_declarationContext *ctx) override {
    FileLine *fl = new FileLine(m_filename, ctx->identifier().front()->value->getLine());
    AstModule *modulep = new AstModule(fl, ctx->identifier().at(0)->value->getText());
    m_currentModule = modulep;
    m_rootp->addModulep(modulep);
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArchitecture_body(vhdlParser::Architecture_bodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInterface_port_declaration(vhdlParser::Interface_port_declarationContext *ctx) override {
    for(auto port : ctx->identifier_list()->id_lst) {
      string portName = port->value->getText();

      string typeName = visitSelected_name(ctx->subtype_indication()->selected_name()[0]);
      VhdlScope *type = m_scopeTable->searchType(typeName);
      if (!type) {
        cout << "Error: " << typeName << " does not name a type" << endl;
        exit(-1);
      }

      m_scopeTable->addItem(new VhdlVarScope(portName, type));
      FileLine *flPort = new FileLine(m_filename, port->value->getLine());
      AstPort *portp = new AstPort(flPort, pinNumber++, portName);
      m_currentModule->addStmtp(portp);

      FileLine *flType = new FileLine(m_filename, port->value->getLine());
      AstNodeDType *dtypep = new AstBasicDType(flType, AstBasicDTypeKwd::BIT);

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
      varp->childDTypep(dtypep);


      m_currentModule->addStmtp(varp);
    }
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSignal_declaration(vhdlParser::Signal_declarationContext *ctx) override {
    for(auto sig : ctx->identifier_list()->id_lst) {
      string sigName = sig->value->getText();

      string typeName = visitSelected_name(ctx->subtype_indication()->selected_name()[0]);
      VhdlScope *type = m_scopeTable->searchType(typeName);
      if (!type) {
        cout << "Error: " << typeName << " does not name a type" << endl;
        exit(-1);
      }

      m_scopeTable->addItem(new VhdlVarScope(sigName, type));

      FileLine *flType = new FileLine(m_filename, 0);
      AstNodeDType *dtypep = new AstBasicDType(flType, AstBasicDTypeKwd::BIT);

      FileLine *flVar = new FileLine(m_filename, 0);
      AstVar *varp = new AstVar(flVar, AstVarType::VAR, sigName, dtypep);
      varp->childDTypep(dtypep);

      m_currentModule->addStmtp(varp);

      cout << "Signal " << sigName << endl;
    }
    return NULL;
  }

  virtual antlrcpp::Any visitConstant_declaration(vhdlParser::Constant_declarationContext *ctx) override {
    for(auto cst : ctx->identifier_list()->id_lst) {
      string cstName = cst->value->getText();
      cout << "Constant " << cstName << endl;

      FileLine *flType = new FileLine(m_filename, 0);
      AstNodeDType *dtypep = new AstBasicDType(flType, AstBasicDTypeKwd::BIT);

      FileLine *flVar = new FileLine(m_filename, 0);
      AstVar *varp = new AstVar(flVar, AstVarType::VAR, cstName, dtypep);
      varp->childDTypep(dtypep);

      m_currentModule->addStmtp(varp);

    }
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConditional_signal_assignment(vhdlParser::Conditional_signal_assignmentContext *ctx) override {
    if (!ctx->conditional_waveforms()->WHEN()) {
      cout << "Concurrent assign" << endl;
      FileLine *flRef = new FileLine(m_filename, 0); // TODO fix This
      AstVarRef *source = new AstVarRef(flRef, "A1", false);

      FileLine *fl = new FileLine(m_filename, 0); // TODO fix This
      AstAssignW *assign = new AstAssignW(fl, visitTarget(ctx->target()), visitConditional_waveforms(ctx->conditional_waveforms()));
      m_currentModule->addStmtp(assign);
    }

    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTarget(vhdlParser::TargetContext *ctx) override {
    if (ctx->name()) {
      cout << "Target " << ctx->name()->identifier()->value->getLine() << endl;
      FileLine *fl = new FileLine(m_filename, ctx->name()->identifier()->value->getLine());
      string targetName =  ctx->name()->identifier()->value->getText();
      if (!m_scopeTable->searchItem(targetName)) {
        cout << "Error: could not find " << targetName << endl;
        exit(-1);
      }
      return (AstNode*) new AstVarRef(fl, targetName, true);
    }
  }

  virtual antlrcpp::Any visitWaveform(vhdlParser::WaveformContext *ctx) override {
    return visitExpression(ctx->waveform_element()[0]->expression()[0]);
  }

  virtual antlrcpp::Any visitExpression(vhdlParser::ExpressionContext *ctx) override {
    if (not ctx->logical_operator().size())
      return visitRelation(ctx->relation()[0]);
    else if (ctx->logical_operator()[0]->AND()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstAnd(fl, visitRelation(ctx->relation()[0]), visitRelation(ctx->relation()[1]));
    } else if (ctx->logical_operator()[0]->OR()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstOr(fl, visitRelation(ctx->relation()[0]), visitRelation(ctx->relation()[1]));
    } else if (ctx->logical_operator()[0]->NAND()) {
      FileLine *fl = new FileLine(m_filename, 0);
      AstNode* and_op = new AstAnd(fl, visitRelation(ctx->relation()[0]), visitRelation(ctx->relation()[1]));
      FileLine *fl_not = new FileLine(m_filename, 0);
      return (AstNode*) new AstNot(fl_not, and_op);
    } else if (ctx->logical_operator()[0]->NOR()) {
      FileLine *fl = new FileLine(m_filename, 0);
      AstNode* or_op = new AstOr(fl, visitRelation(ctx->relation()[0]), visitRelation(ctx->relation()[1]));
      FileLine *fl_not = new FileLine(m_filename, 0);
      return (AstNode*) new AstNot(fl_not, or_op);
    } else if (ctx->logical_operator()[0]->XOR()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstXor(fl, visitRelation(ctx->relation()[0]), visitRelation(ctx->relation()[1]));
    } else if (ctx->logical_operator()[0]->XNOR()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstXnor(fl, visitRelation(ctx->relation()[0]), visitRelation(ctx->relation()[1]));
    }
  }

  virtual antlrcpp::Any visitRelation(vhdlParser::RelationContext *ctx) override {
      return visitShift_expression(ctx->shift_expression()[0]);
  }

  virtual antlrcpp::Any visitShift_expression(vhdlParser::Shift_expressionContext *ctx) override {
    return visitSimple_expression(ctx->simple_expression()[0]);
  }

  virtual antlrcpp::Any visitSimple_expression(vhdlParser::Simple_expressionContext *ctx) override {
    return visitTerm(ctx->term()[0]);
  }

  virtual antlrcpp::Any visitTerm(vhdlParser::TermContext *ctx) override {
    return visitFactor(ctx->factor()[0]);
  }

  virtual antlrcpp::Any visitFactor(vhdlParser::FactorContext *ctx) override {
    return visitPrimary(ctx->primary()[0]);
  }

  virtual antlrcpp::Any visitSelected_name(vhdlParser::Selected_nameContext *ctx) override {
    return ctx->identifier()->value->getText();
  }

  uint32_t interpretEnumerationLiteral(string inStr) {
    string character = inStr.substr(1, 1);
    if (character == "0")
      return 0;
    else if (character == "1")
      return 1;
    else
      return 0;
  }

  virtual antlrcpp::Any visitPrimary(vhdlParser::PrimaryContext *ctx) override {
    if (ctx->literal()->enumeration_literal()->CHARACTER_LITERAL()) {
      FileLine *fl = new FileLine(m_filename, 0);
      uint32_t constValue = interpretEnumerationLiteral(ctx->literal()->enumeration_literal()->CHARACTER_LITERAL()->getText());
      FileLine *flNumber = new FileLine(m_filename, 0);
      const V3Number value(flNumber, 1, constValue);
      return (AstNode*) new AstConst(fl, value);
    } else {
      FileLine *fl = new FileLine(m_filename, 0);
      string targetName = ctx->literal()->enumeration_literal()->identifier()->value->getText();
      if (!m_scopeTable->searchItem(targetName)) {
        cout << "Error: could not find " << targetName << endl;
        exit(-1);
      }
      return (AstNode*) new AstVarRef(fl, targetName, false);
    }
  }

private:
  string m_filename;
  AstNetlist *m_rootp;
  long pinNumber;
  AstModule *m_currentModule;
  VhdlScopeTable *m_scopeTable;
};

#endif
