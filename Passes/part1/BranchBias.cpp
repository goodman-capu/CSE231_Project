#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct BranchBias : public FunctionPass {
        static char ID;
        BranchBias() : FunctionPass(ID) {}
        
        bool runOnFunction(Function &F) override {
            
            return false; // Function is not modified
        }
    }; // end of struct BranchBias
}  // end of anonymous namespace

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Profiling Branch Bias",
                                               false /* Only looks at CFG */,
                                               false /* Analysis Pass */);
