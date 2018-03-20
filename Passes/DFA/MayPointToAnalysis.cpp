#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <set>

#define _R 0
#define _M 1

using namespace llvm;
using namespace std;

class MayPointToInfo : public Info {
public:
    typedef set<unsigned> tInfo;
    
private:
    typedef map<unsigned, tInfo> tData;
    
    tData RMap, MMap;
    
public:
    tData * getPtr(unsigned id) {
        assert(id == _R || id == _M && "Wrong ID, should be _R or _M");
        return (id == _R) ? &RMap : &MMap;
    }
    
    void add(unsigned id, unsigned from, unsigned to) {
        tData *ptr = getPtr(id);
        (*ptr)[from].insert(to);
    }
    
    void add(unsigned id, unsigned from, tInfo to) {
        tData *ptr = getPtr(id);
        (*ptr)[from].insert(to.begin(), to.end());
    }
    
    tInfo get(unsigned id, unsigned from) {
        tData *ptr = getPtr(id);
        return (*ptr)[from];
    }
    
    void print() {
        for (auto ptr : {&RMap, &MMap}) {
            string label = (ptr == &RMap) ? "R" : "M";
            for (auto M : *ptr) {
                if (M.second.size() == 0) {
                    continue;
                }
                errs() << label << M.first << "->(";
                for (auto d : M.second) {
                    errs() << "M" << d << "/";
                }
                errs() << ")|";
            }
        }
        errs() << "\n";
    }
    
    static bool equals(MayPointToInfo * info1, MayPointToInfo * info2) {
        return (info1->RMap == info2->RMap) && (info1->MMap == info2->MMap);
    }
    
    static void join(MayPointToInfo * info1, MayPointToInfo * info2, MayPointToInfo * result) {
        for (auto info : {info1, info2}) {
            if (result != info) {
                for (auto d : info->RMap) {
                    result->add(_R, d.first, d.second);
                }
                for (auto d : info->MMap) {
                    result->add(_M, d.first, d.second);
                }
            }
        }
    }
};

class MayPointToAnalysis : public DataFlowAnalysis<MayPointToInfo, true> {
private:
    void flowfunction(Instruction * I,
                      std::vector<unsigned> & IncomingEdges,
                      std::vector<unsigned> & OutgoingEdges,
                      std::vector<MayPointToInfo *> & Infos) {
        unsigned myIdx = instrToIndex(I);
        
        // Join all incoming info
        MayPointToInfo *outInfo = new MayPointToInfo();
        for (unsigned nodeIdx : IncomingEdges) {
            MayPointToInfo *incomingInfo = edgeToInfo({nodeIdx, myIdx});
            MayPointToInfo::join(outInfo, incomingInfo, outInfo);
        }
        
        if (isa<AllocaInst>(I)) {
            outInfo->add(_R, myIdx, myIdx);
            
        } else if (isa<BitCastInst>(I)) {
            Instruction* instr = (Instruction *)(((CastInst *)I)->getOperand(0));
            unsigned instrIdx = instrToIndex(instr);
            if (instrIdx != UINT_MAX) {
                MayPointToInfo::tInfo info = outInfo->get(_R, instrIdx);
                outInfo->add(_R, myIdx, info);
            }
            
        } else if (isa<GetElementPtrInst>(I)) {
            Instruction *instr = (Instruction *)(((GetElementPtrInst *)I)->getPointerOperand());
            unsigned instrIdx = instrToIndex(instr);
            if (instrIdx != UINT_MAX) {
                MayPointToInfo::tInfo info = outInfo->get(_R, instrIdx);
                outInfo->add(_R, myIdx, info);
            }
            
        } else if (isa<LoadInst>(I)) {
            if (isa<PointerType>(I->getType())) {
                Instruction *instr = (Instruction *)(((LoadInst *)I)->getPointerOperand());
                unsigned instrIdx = instrToIndex(instr);
                if (instrIdx != UINT_MAX) {
                    MayPointToInfo::tInfo info = outInfo->get(_R, instrIdx);
                    for (auto idx : info) {
                        MayPointToInfo::tInfo MInfo = outInfo->get(_M, idx);
                        outInfo->add(_R, myIdx, MInfo);
                    }
                }
            }
            
        } else if (isa<StoreInst>(I)) {
            Instruction *ptrInstr = (Instruction *)(((StoreInst *)I)->getPointerOperand());
            Instruction *valInstr = (Instruction *)(((StoreInst *)I)->getValueOperand());
            unsigned ptrInstrIdx = instrToIndex(ptrInstr);
            unsigned valInstrIdx = instrToIndex(valInstr);
            if (!isa<Constant>(valInstr) && ptrInstrIdx != UINT_MAX && valInstrIdx != UINT_MAX) {
                MayPointToInfo::tInfo ptrInfo = outInfo->get(_R, ptrInstrIdx);
                MayPointToInfo::tInfo valInfo = outInfo->get(_R, valInstrIdx);
                for (auto ptrIdx : ptrInfo) {
                    outInfo->add(_M, ptrIdx, valInfo);
                }
            }
            
        } else if (isa<SelectInst>(I)) {
            Instruction *trueInstr = (Instruction *)(((SelectInst *)I)->getTrueValue());
            Instruction *falseInstr = (Instruction *)(((SelectInst *)I)->getFalseValue());
            unsigned trueInstrIdx = instrToIndex(trueInstr);
            unsigned falseInstrIdx = instrToIndex(falseInstr);
            if (trueInstrIdx != UINT_MAX && falseInstrIdx != UINT_MAX) {
                MayPointToInfo::tInfo trueInfo = outInfo->get(_R, trueInstrIdx);
                MayPointToInfo::tInfo falseInfo = outInfo->get(_R, falseInstrIdx);
                outInfo->add(_R, myIdx, trueInfo);
                outInfo->add(_R, myIdx, falseInfo);
            }
            
        } else if (isa<PHINode>(I)) {
            unsigned firstNonPhiIdx = instrToIndex(I->getParent()->getFirstNonPHI());
            for (unsigned i = myIdx; i < firstNonPhiIdx; i++) {
                PHINode *instr = (PHINode *)indexToInstr(i);
                for (unsigned j = 0; j < instr->getNumIncomingValues(); j++) {
                    Instruction *operandInstr = (Instruction *)instr->getIncomingValue(j);
                    unsigned instrIdx = instrToIndex(operandInstr);
                    if (instrIdx != UINT_MAX) {
                        MayPointToInfo::tInfo info = outInfo->get(_R, instrIdx);
                        outInfo->add(_R, i, info);
                    }
                }
            }
        }
        
        // Copy output to every edge
        for (unsigned i = 0; i < OutgoingEdges.size(); i++) {
            Infos.push_back(outInfo);
        }
    }
    
public:
    MayPointToAnalysis (MayPointToInfo &bottom, MayPointToInfo &initialState) : DataFlowAnalysis(bottom, initialState) {}
};

namespace {
    
    struct MayPointToAnalysisPass : public FunctionPass {
        static char ID;
        MayPointToAnalysisPass() : FunctionPass(ID) {}
        
        bool runOnFunction(Function &F) override {
            MayPointToInfo bottom, initialState;
            MayPointToAnalysis MPTA(bottom, initialState);
            MPTA.runWorklistAlgorithm(&F);
            MPTA.print();
            return false; // Function is not modified
        }
    }; // end of struct MayPointToAnalysisPass
}  // end of anonymous namespace

char MayPointToAnalysisPass::ID = 0;
static RegisterPass<MayPointToAnalysisPass> X("cse231-maypointto", "May Point To Analysis",
                                            false /* Only looks at CFG */,
                                            false /* Analysis Pass */);


