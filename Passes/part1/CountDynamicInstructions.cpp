#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

using namespace llvm;
using namespace std;

namespace {
    struct CountDynamicInstructions : public FunctionPass {
        static char ID;
        CountDynamicInstructions() : FunctionPass(ID) {}
        
        bool runOnFunction(Function &F) override {
            // Get module and context
            Module *mod = F.getParent();
            LLVMContext &ctxt = mod->getContext();
            
            // Get handle of function
            Function *updateFunc = cast<Function>(mod->getOrInsertFunction("updateInstrInfo", Type::getVoidTy(ctxt), Type::getInt32Ty(ctxt), Type::getInt32PtrTy(ctxt), Type::getInt32PtrTy(ctxt)));
            Function *printFunc = cast<Function>(mod->getOrInsertFunction("printOutInstrInfo", Type::getVoidTy(ctxt)));
            
            for (BasicBlock &BB : F) {
                // Get builder
                IRBuilder<> Builder(&BB);
                
                // Prepare arguments
                unordered_map<int, int> Imap;
                for (Instruction &I : BB) {
                    int Opcode = I.getOpcode();
                    if (Imap.find(Opcode) != Imap.end()) {
                        Imap[Opcode]++;
                    } else {
                        Imap[Opcode] = 1;
                    }
                }
                
                ConstantInt *num = ConstantInt::get(Type::getInt32Ty(ctxt), Imap.size());
                vector<Constant *> keys, values;
                for (auto item : Imap) {
                    keys.push_back(ConstantInt::get(Type::getInt32Ty(ctxt), item.first));
                    values.push_back(ConstantInt::get(Type::getInt32Ty(ctxt), item.second));
                }
                ArrayType *typeKey = ArrayType::get(Type::getInt32Ty(ctxt), Imap.size());
                GlobalVariable *gKeys = new GlobalVariable(*mod, typeKey, true, GlobalVariable::InternalLinkage, ConstantArray::get(typeKey, keys), "global keys");
                ArrayType *typeVal = ArrayType::get(Type::getInt32Ty(ctxt), Imap.size());
                GlobalVariable *gValues = new GlobalVariable(*mod, typeVal, true, GlobalVariable::InternalLinkage, ConstantArray::get(typeVal, values), "global values");
                
                Value* indexs[2] = {ConstantInt::get(Type::getInt32Ty(ctxt), 0), ConstantInt::get(Type::getInt32Ty(ctxt), 0)};
                vector<Value *> args = {num, Builder.CreateInBoundsGEP(gKeys, indexs), Builder.CreateInBoundsGEP(gValues, indexs)};
                
                // Insert update function
                Builder.SetInsertPoint(BB.getTerminator());
                Builder.CreateCall(updateFunc, args);
                
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
    }; // end of struct CountDynamicInstructions
}  // end of anonymous namespace

char CountDynamicInstructions::ID = 0;
static RegisterPass<CountDynamicInstructions> X("cse231-cdi", "Collecting Dynamic Instruction Counts",
                                                false /* Only looks at CFG */,
                                                false /* Analysis Pass */);
