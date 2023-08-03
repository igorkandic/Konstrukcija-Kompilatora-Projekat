#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
using namespace llvm;

namespace {
  struct DAEPass : public FunctionPass {
    static char ID;
    DAEPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      errs() << "I saw a function called " << F.getName() << "!\n"; 
      return false;
    }
  };
}

char DAEPass::ID = 0;

static RegisterPass<DAEPass> X("deadae", "Dead Argument Elimination Pass",
                                    false, false);
