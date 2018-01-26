#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

using namespace llvm;

namespace {
    struct CountStaticInstructions : public FunctionPass {
        static char ID;
        CountStaticInstructions() : FunctionPass(ID) {}
        
        bool runOnFunction(Function &F) override {
            std::unordered_map<const char *, int> Icounter;
            for (BasicBlock &BB : F) {
                for (Instruction &I : BB) {
                    // errs() << I.getOpcodeName() << '\n';
                    auto Iname = I.getOpcodeName();
                    if (Icounter.find(Iname) != Icounter.end()) {
                        Icounter[Iname] ++;
                    } else {
                        Icounter[Iname] = 1;
                    }
                }
            }
            for (auto item : Icounter) {
                errs() << item.first << '\t' << item.second << '\n';
            }
            return false; // Function is not modified
        }
    }; // end of struct CountStaticInstructions
}  // end of anonymous namespace

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions> X("cse231-csi", "Collecting Static Instruction Counts",
                                               false /* Only looks at CFG */,
                                               false /* Analysis Pass */);
