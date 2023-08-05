#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <algorithm>
using namespace llvm;

#define DEBUG_TYPE "deadae"

namespace {
  struct DAEPass : public ModulePass {
    static char ID;
    DAEPass() : ModulePass(ID) {}

void deleteUnusedLoads(CallInst* callInst) {

  Function* calledFunction = callInst->getCalledFunction();
  if (!calledFunction) {
    return ; 
  }
  for (unsigned i = 0; i < callInst->arg_size(); ++i) {
    Value* argValue = callInst->getArgOperand(i);
    if (!isa<Constant>(argValue)) {
      if(argValue->getNumUses() == 1){
        Instruction* inst = dyn_cast<Instruction>(argValue->uses().begin()->get());
        if(LoadInst* load = dyn_cast<LoadInst>(inst)){
          load->eraseFromParent();
        }
      }
    }
  }

  return ;
}

void removeArgumentFromFunction(Function* F, std::vector<int>& argIndexesToRemove) {
  

  FunctionType* funcType = F->getFunctionType();
  std::vector<Type*> paramTypes = funcType->params();

  const AttributeList &PAL = F->getAttributes();


  std::sort(argIndexesToRemove.begin(), argIndexesToRemove.end(), std::greater<int>() );
  for(int argIndexToRemove : argIndexesToRemove){
     paramTypes.erase(paramTypes.begin() + argIndexToRemove);
  }
  

  FunctionType* newFuncType = FunctionType::get(funcType->getReturnType(), paramTypes, funcType->isVarArg());

  AttrBuilder RAttrs(F->getContext(), PAL.getRetAttrs());
  AttributeSet FnAttrs =
      PAL.getFnAttrs().removeAttribute(F->getContext(), Attribute::AllocSize);
  AttributeSet RetAttrs = AttributeSet::get(F->getContext(), RAttrs);

  SmallVector<AttributeSet, 8> ArgAttrVec;
  for(auto I = F->arg_begin(), E = F->arg_end(); I != E; ++I){
    if(std::find(argIndexesToRemove.begin(), argIndexesToRemove.end(), I->getArgNo()) == argIndexesToRemove.end() ){
      ArgAttrVec.push_back(PAL.getParamAttrs(I->getArgNo()));
    }
  }

  AttributeList NewPAL =
      AttributeList::get(F->getContext(), FnAttrs, RetAttrs, ArgAttrVec);


  Function* newFunction = Function::Create(newFuncType, F->getLinkage(), F->getAddressSpace());
  newFunction->copyAttributesFrom(F);
  newFunction->setComdat(F->getComdat());
  newFunction->setAttributes(NewPAL);



  F->getParent()->getFunctionList().insert(F->getIterator(), newFunction);
  newFunction->takeName(F);



  for (User* U : F->users()) {
    if (auto callInst = dyn_cast<CallInst>(U)) {
      const AttributeList &CallPAL = callInst->getAttributes();
      ArgAttrVec.clear();
      std::vector<Value*> newArgs;
      for (unsigned i = 0; i < callInst->arg_size(); ++i) {
        if (std::find(argIndexesToRemove.begin(), argIndexesToRemove.end(), i) == argIndexesToRemove.end() ) {
          newArgs.push_back(callInst->getArgOperand(i));
          ArgAttrVec.push_back(CallPAL.getAttributes(i));
        }
      }
      SmallVector<OperandBundleDef, 1> OpBundles;
      callInst->getOperandBundlesAsDefs(OpBundles);
      AttributeSet FnAttrs = CallPAL.getFnAttrs().removeAttribute(
        F->getContext(), Attribute::AllocSize);
      AttributeList NewCallPAL =
        AttributeList::get(F->getContext(), FnAttrs, CallPAL.getRetAttrs(), ArgAttrVec);
      CallInst* newCallInst = CallInst::Create(newFuncType, newFunction, newArgs, OpBundles, "", callInst);
      newCallInst->setTailCallKind(callInst->getTailCallKind());
      newCallInst->setCallingConv(F->getCallingConv());
      newCallInst->setAttributes(NewCallPAL);
      newCallInst->copyMetadata(*callInst, {LLVMContext::MD_prof, LLVMContext::MD_dbg});

      

        deleteUnusedLoads(callInst);

      callInst->replaceAllUsesWith(newCallInst);
      newCallInst->takeName(callInst);
      callInst->eraseFromParent();
    }
  }

  newFunction->getBasicBlockList().splice(newFunction->begin(), F->getBasicBlockList());
  
  unsigned ArgI = 0;
  for(auto I = F->arg_begin(), E = F->arg_end(), I2 = newFunction->arg_begin(); I != E; ++I, ++ArgI){
    if(std::find(argIndexesToRemove.begin(), argIndexesToRemove.end(), ArgI) == argIndexesToRemove.end() ){
      I->replaceAllUsesWith(&*I2);
      I2->takeName(&*I);
      ++I2;
    } else {
      if(!I->getType()->isX86_AMXTy())
        I->replaceAllUsesWith(PoisonValue::get(I->getType()));
    }
  }

  F->eraseFromParent();

}


    bool findIsArgumentDead(Function &F, Value *Arg){
      for(BasicBlock &BB : F){
        for(Instruction &I : BB){
          for(Use &U : I.operands()){
            Value *Val = U.get();
            if(Val == Arg){
              if(isa<LoadInst>(I)){
                return false;
              }
            }
          }
        }
      }
      return true;
    }
    bool isDead(Function &F, Argument &Arg){
        for(BasicBlock &BB : F){
          for(Instruction &I : BB){
            for(Use &U : I.operands()){
              Value *Val = U.get();
                if(Val == &Arg){
                  if(findIsArgumentDead(F, I.getOperand(1))){
                    errs() << Arg << " is Dead " << "\n";
                    return true;
                  }
                  else{
                    errs() << Arg << " is not Dead " << "\n";
                    return false;
                  }
                }
            }
          }
        }
        return false;
    }

    virtual bool runOnModule(Module &M) {
 
      CallGraph CG = CallGraph(M);
      for (auto IT = df_begin(&CG), EI = df_end(&CG); IT != EI; IT++){
        Function *F = IT->getFunction();
        if(F && !F->empty() && F->getName() != "main"){
          errs() << "Visiting Function: " << F->getName() << "\n";
           std::vector<int> dead;
          for(Argument *Arg = F->arg_begin(); Arg != F->arg_end(); Arg++){
             if(isDead(*F, *Arg)){
              dead.push_back(Arg->getArgNo());
             }
          }
          removeArgumentFromFunction(F, dead);
          
        }
      }
      
      return true;
    }
  };
}

char DAEPass::ID = 0;

static RegisterPass<DAEPass> X("deadae", "Dead Argument Elimination Pass",
                                    false, false);
