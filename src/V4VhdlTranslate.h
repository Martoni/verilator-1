#ifndef _V4VHDLTRANSLATE_H_
#define _V4VHDLTRANSLATE_H_

#include "V4VhdlSymtable.h"
#include "vhdl_parser/vhdlBaseVisitor.h"
#include "V3Ast.h"
#include <iostream>
#include <algorithm>
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

      VhdlTypeScope *type = m_scopeTable->searchType(typeName);
      if (!type) {
        cout << "Error: " << typeName << " does not name a type" << endl;
        exit(-1);
      }

      m_scopeTable->addItem(new VhdlVarScope(portName, type));
      FileLine *flPort = new FileLine(m_filename, port->value->getLine());
      AstPort *portp = new AstPort(flPort, pinNumber++, portName);
      m_currentModule->addStmtp(portp);

      FileLine *flType = new FileLine(m_filename, port->value->getLine());
      AstBasicDType *dtypep = new AstBasicDType(flType, AstBasicDTypeKwd::BIT);
      if (ctx->subtype_indication()->constraint()) {
        if (ctx->subtype_indication()->constraint()->index_constraint()) {
          dtypep->rangep(visitIndex_constraint(ctx->subtype_indication()->constraint()->index_constraint()));
        }
      }
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
      VhdlTypeScope *type = m_scopeTable->searchType(typeName);
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
    FileLine *fl = new FileLine(m_filename, 0); // TODO fix This
    AstAssignW *assign = new AstAssignW(fl, visitTarget(ctx->target()), visitConditional_waveforms(ctx->conditional_waveforms()));
    m_currentModule->addStmtp(assign);
    return NULL;
  }

  virtual antlrcpp::Any visitConditional_waveforms(vhdlParser::Conditional_waveformsContext *ctx) override {
    AstNode *node = visitWaveform(ctx->waveform());
    if (ctx->WHEN()) {
      FileLine *fl = new FileLine(m_filename, 0);
      if (ctx->ELSE()) { // TODO fix mistranslation when ELSE is missing (condition dropped)
        node = new AstCond(fl, visitCondition(ctx->condition()), node, (AstNode*)visitConditional_waveforms(ctx->conditional_waveforms()));
      }
    }
    return (AstNode*) node;
  }

  virtual antlrcpp::Any visitTarget(vhdlParser::TargetContext *ctx) override {
    if (ctx->name()) {
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

  virtual antlrcpp::Any visitIndex_constraint(vhdlParser::Index_constraintContext *ctx) override {
      FileLine *fl = new FileLine(m_filename, 0);
      auto range = ctx->discrete_range()[0]->range_decl()->explicit_range();
      return new AstRange(fl, (AstNode*)visitSimple_expression(range->simple_expression()[0]), (AstNode*)visitSimple_expression(range->simple_expression()[1]));
  }

  void resolveType(AstNode* node) {
    string opName = node->name();
    string typeName = ((VhdlVarScope*)m_scopeTable->searchItem(opName))->getType()->to_string();
    cout << opName << " of type " << typeName << endl;
  }

  virtual antlrcpp::Any visitExpression(vhdlParser::ExpressionContext *ctx) override {
    AstNode * previousNode = visitRelation(ctx->relation()[0]);
    for (int i = 0; i < ctx->logical_operator().size(); ++i) {
      cout << i << " " << ctx->logical_operator()[i] << endl;
      if (ctx->logical_operator()[i]->AND()) {
        FileLine *fl = new FileLine(m_filename, 0);
        resolveType((AstNode*)visitRelation(ctx->relation()[i]));
        resolveType((AstNode*)visitRelation(ctx->relation()[i+1]));
        previousNode = new AstAnd(fl, previousNode, visitRelation(ctx->relation()[i+1]));
      } else if (ctx->logical_operator()[i]->OR()) {
        FileLine *fl = new FileLine(m_filename, 0);
        previousNode = new AstOr(fl, previousNode, visitRelation(ctx->relation()[i+1]));
      } else if (ctx->logical_operator()[i]->NAND()) {
        FileLine *fl = new FileLine(m_filename, 0);
        AstNode* and_op = new AstAnd(fl, previousNode, visitRelation(ctx->relation()[i+1]));
        FileLine *fl_not = new FileLine(m_filename, 0);
        previousNode = new AstNot(fl_not, and_op);
      } else if (ctx->logical_operator()[i]->NOR()) {
        FileLine *fl = new FileLine(m_filename, 0);
        AstNode* or_op = new AstOr(fl, previousNode, visitRelation(ctx->relation()[i+1]));
        FileLine *fl_not = new FileLine(m_filename, 0);
        previousNode = new AstNot(fl_not, or_op);
      } else if (ctx->logical_operator()[i]->XOR()) {
        FileLine *fl = new FileLine(m_filename, 0);
        previousNode = new AstXor(fl, previousNode, visitRelation(ctx->relation()[i+1]));
      } else if (ctx->logical_operator()[i]->XNOR()) {
        FileLine *fl = new FileLine(m_filename, 0);
        previousNode = new AstXnor(fl, previousNode, visitRelation(ctx->relation()[i+1]));
      }
    }
    return (AstNode*) previousNode;
  }

  virtual antlrcpp::Any visitRelation(vhdlParser::RelationContext *ctx) override {
    if (!ctx->relational_operator())
      return visitShift_expression(ctx->shift_expression()[0]);
    else if (ctx->relational_operator()->EQ()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstEq(fl, visitShift_expression(ctx->shift_expression()[0]), visitShift_expression(ctx->shift_expression()[1]));
    } else if (ctx->relational_operator()->NEQ()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstNeq(fl, visitShift_expression(ctx->shift_expression()[0]), visitShift_expression(ctx->shift_expression()[1]));
    } else if (ctx->relational_operator()->LOWERTHAN()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstLt(fl, visitShift_expression(ctx->shift_expression()[0]), visitShift_expression(ctx->shift_expression()[1]));
    } else if (ctx->relational_operator()->LE()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstLte(fl, visitShift_expression(ctx->shift_expression()[0]), visitShift_expression(ctx->shift_expression()[1]));
    } else if (ctx->relational_operator()->GREATERTHAN()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstGt(fl, visitShift_expression(ctx->shift_expression()[0]), visitShift_expression(ctx->shift_expression()[1]));
    } else if (ctx->relational_operator()->GE()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstGte(fl, visitShift_expression(ctx->shift_expression()[0]), visitShift_expression(ctx->shift_expression()[1]));
    }
  }

  virtual antlrcpp::Any visitShift_expression(vhdlParser::Shift_expressionContext *ctx) override {
    return visitSimple_expression(ctx->simple_expression()[0]);
  }

  virtual antlrcpp::Any visitSimple_expression(vhdlParser::Simple_expressionContext *ctx) override {
    AstNode * prevNode = visitTerm(ctx->term()[0]);
    for (int i = 0; i < ctx->adding_operator().size(); ++i) {
      if (ctx->adding_operator()[i]->PLUS()) {
        FileLine *fl = new FileLine(m_filename, 0);
        prevNode = new AstAdd(fl, prevNode, visitTerm(ctx->term()[i + 1]));
      } else if (ctx->adding_operator()[i]->MINUS()) {
        FileLine *fl = new FileLine(m_filename, 0);
        prevNode = new AstSub(fl, prevNode, visitTerm(ctx->term()[i + 1]));
      } else if (ctx->adding_operator()[i]->AMPERSAND()) {
        FileLine *fl = new FileLine(m_filename, 0);
        prevNode = new AstConcat(fl, prevNode, visitTerm(ctx->term()[i + 1]));
      }
    }
    return (AstNode*) prevNode;
  }

  virtual antlrcpp::Any visitTerm(vhdlParser::TermContext *ctx) override {
    AstNode * prevNode = visitFactor(ctx->factor()[0]);
    for (int i = 0; i < ctx->multiplying_operator().size(); ++i) {
      if (ctx->multiplying_operator()[i]->MUL()) {
        FileLine *fl = new FileLine(m_filename, 0);
        prevNode = new AstMul(fl, prevNode, visitFactor(ctx->factor()[i + 1]));
      } else if (ctx->multiplying_operator()[i]->DIV()) {
        FileLine *fl = new FileLine(m_filename, 0);
        prevNode = new AstDiv(fl, prevNode, visitFactor(ctx->factor()[i + 1]));
      } else if (ctx->multiplying_operator()[i]->MOD()) {
        FileLine *fl = new FileLine(m_filename, 0);
        prevNode = new AstModDiv(fl, prevNode, visitFactor(ctx->factor()[i + 1]));
      } else if (ctx->multiplying_operator()[i]->REM()) {
        FileLine *fl = new FileLine(m_filename, 0);
        prevNode = new AstModDivS(fl, prevNode, visitFactor(ctx->factor()[i + 1]));
      }
    }
    return (AstNode*) prevNode;
  }

  virtual antlrcpp::Any visitFactor(vhdlParser::FactorContext *ctx) override {
    if (ctx->DOUBLESTAR()) {
      FileLine *fl = new FileLine(m_filename, 0);
      // TODO implement
    } else if (ctx->ABS()) {
      FileLine *fl = new FileLine(m_filename, 0);
      // TODO implemeAnt
    } else if (ctx->NOT()) {
      FileLine *fl = new FileLine(m_filename, 0);
      return (AstNode*) new AstNot(fl, visitPrimary(ctx->primary()[0]));
    }
    else
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

    // Handle numeric literals
    if (ctx->literal()->numeric_literal()) {
      return (AstNode*) visitNumeric_literal(ctx->literal()->numeric_literal());

    // Handle char literal
    } else if (ctx->literal()->enumeration_literal()->CHARACTER_LITERAL()) {
      FileLine *fl = new FileLine(m_filename, 0);
      uint32_t constValue = interpretEnumerationLiteral(ctx->literal()->enumeration_literal()->CHARACTER_LITERAL()->getText());
      FileLine *flNumber = new FileLine(m_filename, 0);
      const V3Number value(flNumber, 1, constValue);
      return (AstNode*) new AstConst(fl, value);

      // Special case for VarRef coming from here instead of name
    } else if (ctx->literal()->enumeration_literal()->identifier()){
      FileLine *fl = new FileLine(m_filename, 0);
      string targetName = ctx->literal()->enumeration_literal()->identifier()->value->getText();
      if (!m_scopeTable->searchItem(targetName)) {
        cout << "Error: could not find " << targetName << endl;
        exit(-1);
      }
      return (AstNode*) new AstVarRef(fl, targetName, false);
    }
  }

  virtual antlrcpp::Any visitNumeric_literal(vhdlParser::Numeric_literalContext *ctx) override {
    if (ctx->physical_literal()) {
      cout << "Error: VHDL physical literal not supported" << endl;
      exit(-1);
    } else if (ctx->abstract_literal()) {
      if (ctx->abstract_literal()->INTEGER()) {
        FileLine *fl = new FileLine(m_filename, 0);
        uint32_t constValue = stoul(ctx->abstract_literal()->INTEGER()->getText());
        FileLine *flNumber = new FileLine(m_filename, 0);
        const V3Number value(flNumber, 32, constValue);
        return (AstNode*) new AstConst(fl, value);
        // TODO fix this with correct values
      } else if (ctx->abstract_literal()->BASE_LITERAL()) {
        auto basestr = ctx->abstract_literal()->BASE_LITERAL()->getText();
        basestr.erase(remove(basestr.begin(), basestr.end(), '_'), basestr.end());
        FileLine *fl = new FileLine(m_filename, 0);
        uint32_t constValue = 0;
        FileLine *flNumber = new FileLine(m_filename, 0);
        const V3Number value(flNumber, 32, constValue);
        return (AstNode*) new AstConst(fl, value);
        cout << basestr << endl;

      }
    }
  }

  virtual antlrcpp::Any visitProcess_statement(vhdlParser::Process_statementContext *ctx) override {
    FileLine *flSenTree = new FileLine(m_filename, 0);
    AstSenTree *senList = new AstSenTree(flSenTree, NULL);
    if (ctx->sensitivity_list()) { // Create refs for sensitivity list
      for (auto sigName : ctx->sensitivity_list()->name()) {
        FileLine *flSigRef = new FileLine(m_filename, 0);
        AstNode *sigRef = new AstVarRef(flSigRef, sigName->identifier()->getText(), false);
        FileLine *flSenItem = new FileLine(m_filename, 0);
        senList->addSensesp(new AstSenItem(flSenItem, AstEdgeType::ET_ANYEDGE, sigRef));
      }
    }

    AstBegin *content = NULL;
    if (ctx->process_statement_part()) {
      FileLine *flProcessStmts = new FileLine(m_filename, 0);
      content = new AstBegin(flProcessStmts, "", NULL);
      for (auto stmt : ctx->process_statement_part()->sequential_statement()) {
        content->addStmtsp(visitSequential_statement(stmt));
      }
    }

    FileLine *flProcess = new FileLine(m_filename, 0);
    AstAlways *process = new AstAlways(flProcess, VAlwaysKwd::ALWAYS, senList, content);
    m_currentModule->addStmtp(process);
    return NULL;
  }

  virtual antlrcpp::Any visitSignal_assignment_statement(vhdlParser::Signal_assignment_statementContext *ctx) override {
    FileLine *flAssign = new FileLine(m_filename, 0);
    AstAssignDly *assign = new AstAssignDly(flAssign, visitTarget(ctx->target()), visitWaveform(ctx->waveform()));
    return (AstNode *) assign;
  }

  virtual antlrcpp::Any visitIf_statement(vhdlParser::If_statementContext *ctx) override {
    // Visit the chain in reverse order to recreate it
    AstNode *elsep = NULL;

    // Latest block of code is the else
    if (ctx->ELSE())
      elsep = visitSequence_of_statements(ctx->sequence_of_statements()[ctx->sequence_of_statements().size() - 1]);

    // By default take the first then statement
    AstNode *thenp = visitSequence_of_statements(ctx->sequence_of_statements()[0]);

    for(int ifc = ctx->ELSIF().size(); ifc >= 1 ; --ifc) {
      cout << ifc << endl;
      thenp = visitSequence_of_statements(ctx->sequence_of_statements()[ifc]);
      FileLine *flElsif = new FileLine(m_filename, 0);
      elsep = new AstIf(flElsif, visitCondition(ctx->condition()[ifc]), thenp, elsep);
      thenp = visitSequence_of_statements(ctx->sequence_of_statements()[ifc - 1]);
    }

    FileLine *flIf = new FileLine(m_filename, 0);
    AstIf *last_if = new AstIf(flIf, visitCondition(ctx->condition()[0]), thenp, elsep);
    return (AstNode *)last_if;
  }

private:
  string m_filename;
  AstNetlist *m_rootp;
  long pinNumber;
  AstModule *m_currentModule;
  VhdlScopeTable *m_scopeTable;
};

#endif
