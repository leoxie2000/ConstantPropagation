#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constant.h"
#include "llvm/Support/Compiler.h"
#include "llvm/IR/Instruction.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/ADT/APInt.h"
#include <set>
#include <map>
#include <iostream>


using namespace llvm;
namespace{
//initializing the pass
struct ConstantPropagationPass : public FunctionPass {
    int getInt (Value *val) {
	    if (llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(val)) {
		    return CI->getSExtValue();
	    }
	    return 0;
    }
    static char ID;
    ConstantPropagationPass() : FunctionPass(ID) {}

    //main function to be called on
    virtual bool runOnFunction (Function &F) {
        bool change = false;
        std::set<Instruction*> InstructionList;
        //adding all instructions into a list 
        for(inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i) {
            InstructionList.insert(&*i);
        }
        //continue looping until no instructions left
        while(!InstructionList.empty()){
            Instruction *I = *InstructionList.begin();
            InstructionList.erase(InstructionList.begin());
            //No need to replace if no instructions used in future
            if (!I->use_empty()) {
                //Case binary operation
                if ((*I).isBinaryOp()) {
                    Value *lhs = (*I).getOperand(0);	// grab the operators
                    Value *rhs = (*I).getOperand(1);
                    if (isa<ConstantInt>(lhs) & isa<ConstantInt>(rhs)) {	// see if both are constants
                        int opResult = 0;
                        //case switch based on opcode
                        switch ((*I).getOpcode()) {		// Look for known opcodes
                            case Instruction::Add:
                                opResult = getInt(lhs) + getInt(rhs);
                                break;
                            case Instruction::Sub:
                                opResult = getInt(lhs) - getInt(rhs);
                                break;
                            case Instruction::Mul:
                                opResult = getInt(lhs) * getInt(rhs);
                                break;
                            case Instruction::SDiv:
                                opResult = getInt(lhs) / /* */ getInt(rhs);
                                break;
                        
                            default:
                            llvm_unreachable ("Unknown or uncoded inst. in binary op.");
                            break;
                        }
                        //replace everything with the value
                        Value* V = llvm::ConstantInt::get(lhs->getType(), opResult);
                        //add everything relevant to the set
                        for (Value::use_iterator UI = I->use_begin(), UE = I->use_end(); UI != UE; ++UI)
                            InstructionList.insert(cast<Instruction>(*UI));

                        (*I).replaceAllUsesWith(V);
                        I->getParent()->getInstList().erase(I);
                        change = true;

                    }

                }
                //same as binary, except there's only one side
                else if ((*I).isUnaryOp()) {
                    Value *lhs = (*I).getOperand(0);
                    if (isa<ConstantInt>(lhs)) {	// see if left is constants
                        int opResult = 0;
                        switch ((*I).getOpcode()) {	
                            case Instruction::FNeg:
                                opResult = -getInt(lhs);
                                break;
                            default:
                                llvm_unreachable ("Unknown or uncoded inst. in binary op.");
                                break;
                        }
                        Value* V = llvm::ConstantInt::get(lhs->getType(), opResult);
                        for (Value::use_iterator UI = I->use_begin(), UE = I->use_end(); UI != UE; ++UI)
                            InstructionList.insert(cast<Instruction>(*UI));
                        (*I).replaceAllUsesWith(V);
                        I->getParent()->getInstList().erase(I);
                        change = true;

                    }

                    
                }
                //case comparison (relational)
                else if (I->getOpcode() == Instruction::ICmp) {
                    Value *lhs = (*I).getOperand(0);
                    Value *rhs = (*I).getOperand(1);
                    if (isa<ConstantInt>(lhs) & isa<ConstantInt>(rhs)) {	// see if both are constants

                    //case into icmpinst type 
                        ICmpInst *cc = dyn_cast<ICmpInst>(I);
                        bool opResult;
                        CmpInst::Predicate op = cc->getSignedPredicate();
                        switch (op) {	
                            case CmpInst::ICMP_EQ: //case equal
                                opResult = (lhs == rhs);
                                break;
                            case CmpInst::ICMP_NE: //case not equal
                                opResult = (lhs != rhs);
                                break;
                            case CmpInst::ICMP_SGT: //case signed greater
                                opResult = (lhs > rhs);
                                break;
                            case CmpInst::ICMP_SGE: //case signed greater or equal
                                opResult = (lhs >= rhs);
                                break;
                            case CmpInst::ICMP_SLT://case signed less
                                opResult = (lhs < rhs);
                                break;
                            case CmpInst::ICMP_SLE://case signed less or equal
                                opResult = (lhs <= rhs);
                                break;

                            default:
                                llvm_unreachable ("Unknown or uncoded inst. in Comp op.");
                                break;
                        }
                        Value* V = ConstantInt::get(lhs->getType(), APInt(1, opResult));
                        for (Value::use_iterator UI = I->use_begin(), UE = I->use_end(); UI != UE; ++UI)
                            InstructionList.insert(cast<Instruction>(*UI));
                        (*I).replaceAllUsesWith(V);
                        I->getParent()->getInstList().erase(I);
                        change = true;
                    }

                    

                    
                }
            }
            
        }
        return change;
    }
};
}

char ConstantPropagationPass::ID = 0;
//
static llvm::RegisterPass<ConstantPropagationPass> X("constantpropagationpass", "ConstantPropagation Pass",
			false,  /* looks at CFG, true changed CFG */
			false); /* analysis pass, true means analysis needs to run again */
static void registerConstantPropagationPass(const llvm::PassManagerBuilder &,
                         llvm::legacy::PassManagerBase &PM) {
  PM.add(new ConstantPropagationPass());
}
static llvm::RegisterStandardPasses
  RegisterMyPass(llvm::PassManagerBuilder::EP_EarlyAsPossible,
                 registerConstantPropagationPass);