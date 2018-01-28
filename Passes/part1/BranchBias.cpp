#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>

using namespace llvm;
using namespace std;

namespace {
    struct BranchBias : public FunctionPass {
        static char ID;
        BranchBias() : FunctionPass(ID) {}
        
        bool runOnFunction(Function &F) override {
            // Get module and context
            Module *mod = F.getParent();
            LLVMContext &ctxt = mod->getContext();
            
            // Get handle of function
            Function *updateFunc = cast<Function>(mod->getOrInsertFunction("updateBranchInfo", Type::getVoidTy(ctxt), Type::getInt1Ty(ctxt)));
            Function *printFunc = cast<Function>(mod->getOrInsertFunction("printOutBranchInfo", Type::getVoidTy(ctxt)));
            
            for (BasicBlock &BB : F) {
                // Get builder
                IRBuilder<> Builder(&BB);
                
                // Insert update function
                BranchInst *terminator = dyn_cast<BranchInst>(BB.getTerminator());
                if (terminator && terminator->isConditional()) {
                    vector<Value *> args = {terminator->getCondition()};
                    Builder.SetInsertPoint(BB.getTerminator());
                    Builder.CreateCall(updateFunc, args);
                }
                
                // Insert print function
                for (Instruction &I : BB) {
                    if (I.getOpcode() == 1) { // "ret"
                        Builder.SetInsertPoint(&I);
                        Builder.CreateCall(printFunc);
                    }
                }
            }
            
            return false; // Function is not modified
        }
    }; // end of struct BranchBias
}  // end of anonymous namespace

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Profiling Branch Bias",
                                               false /* Only looks at CFG */,
                                               false /* Analysis Pass */);
