#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <llvm-14/llvm/IR/BasicBlock.h>
#include <llvm-14/llvm/IR/Instruction.h>
#include <llvm-14/llvm/Pass.h>
#include <llvm-14/llvm/Support/Casting.h>
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/CallGraph.h"
using namespace llvm;

namespace {
  struct DAEPass : public ModulePass {
    static char ID;
    DAEPass() : ModulePass(ID) {}

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
    void findUsesOfFunctionArgument(Function &F, Argument &Arg){
        for(BasicBlock &BB : F){
          for(Instruction &I : BB){
            for(Use &U : I.operands()){
              Value *Val = U.get();
                if(Val == &Arg){
                  if(findIsArgumentDead(F, I.getOperand(1))){
                    dbgs() << Arg << " is Dead " << "\n";
                  }
                  else{
                    dbgs() << Arg << " is not Dead " << "\n";
                  }
                }
            }
          }
        }
    }

    virtual bool runOnModule(Module &M) {
      CallGraph CG = CallGraph(M);

      for (auto IT = df_begin(&CG), EI = df_end(&CG); IT != EI; IT++){
        Function *F = IT->getFunction();
        if(F && !F->empty()){
          dbgs() << "Visiting Function: " << F->getName() << "\n";
          for(Argument *Arg = F->arg_begin(); Arg != F->arg_end(); Arg++){
            findUsesOfFunctionArgument(*F, *Arg);
          }
        }
      }
      return false;
    }
  };
}

char DAEPass::ID = 0;

static RegisterPass<DAEPass> X("deadae", "Dead Argument Elimination Pass",
                                    false, false);
