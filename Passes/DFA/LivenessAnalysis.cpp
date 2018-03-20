#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>
#include <set>

using namespace llvm;
using namespace std;

class LivenessInfo : public Info {
private:
    set<unsigned> data;
    
public:
    LivenessInfo() : Info() {}
    
    LivenessInfo(const LivenessInfo &other) : Info((const Info &)other) {
        data = other.data;
    }
    
    void add(unsigned d) {
        data.insert(d);
    }
    
    void remove(unsigned d) {
        data.erase(d);
    }
    
    void print() {
        for (auto d : data) {
            errs() << d << '|';
        }
        errs() << '\n';
    }
    
    static bool equals(LivenessInfo * info1, LivenessInfo * info2) {
        return info1->data == info2->data;
    }
    
    static void join(LivenessInfo * info1, LivenessInfo * info2, LivenessInfo * result) {
        if (result != info1) {
            for (auto d : info1->data) {
                result->add(d);
            }
        }
        if (result != info2) {
            for (auto d : info2->data) {
                result->add(d);
            }
        }
    }
};

class LivenessAnalysis : public DataFlowAnalysis<LivenessInfo, false> {
private:
    unordered_map<string, int> categoryMap = {
        {"br", 2},
        {"switch", 2},
        {"alloca", 1},
        {"load", 1},
        {"store", 2},
        {"getelementptr", 1},
        {"icmp", 1},
        {"fcmp", 1},
        {"phi", 3},
        {"select", 1},
    };
    
    void flowfunction(Instruction * I,
                      std::vector<unsigned> & IncomingEdges,
                      std::vector<unsigned> & OutgoingEdges,
                      std::vector<LivenessInfo *> & Infos) {
        int category = 2; // Default to second category
        string name = I->getOpcodeName();
        if (categoryMap.count(name) > 0) {
            category = categoryMap[name];
        } else if (I->isBinaryOp()) {
            category = 1;
        }
        
        unsigned myIdx = instrToIndex(I);
        
        // Join all incoming info
        LivenessInfo *outInfo = new LivenessInfo();
        for (unsigned nodeIdx : IncomingEdges) {
            LivenessInfo *incomingInfo = edgeToInfo({nodeIdx, myIdx});
            LivenessInfo::join(outInfo, incomingInfo, outInfo);
        }
        
        if (category == 1 || category == 2) {
            // Add all operands
            for (unsigned i = 0; i < I->getNumOperands(); i++) {
                Instruction *operandInstr = (Instruction *)I->getOperand(i);
                unsigned operandIdx = instrToIndex(operandInstr);
                if (operandIdx != UINT_MAX) {
                    outInfo->add(operandIdx);
                }
            }
            
            if (category == 1) {
                outInfo->remove(myIdx);
            }
            
            // Copy output to every edge
            for (unsigned i = 0; i < OutgoingEdges.size(); i++) {
                Infos.push_back(new LivenessInfo(*outInfo));
            }
        } else { // PHI node
            unsigned firstNonPhiIdx = instrToIndex(I->getParent()->getFirstNonPHI());
            for (unsigned i = myIdx; i < firstNonPhiIdx; i++) {
                outInfo->remove(i);
            }
            
            for (unsigned k = 0; k < OutgoingEdges.size(); k++) {
                Instruction *outInstrK = indexToInstr(OutgoingEdges[k]);
                LivenessInfo *outInfoK = new LivenessInfo(*outInfo);

                for (unsigned i = myIdx; i < firstNonPhiIdx; i++) {
                    PHINode *instr = (PHINode *)indexToInstr(i);
                    // Iterate over incoming values
                    for (unsigned j = 0; j < instr->getNumIncomingValues(); j++) {
                        Instruction *operandInstr = (Instruction *)instr->getIncomingValue(j);
                        Instruction *blockLabel = instr->getIncomingBlock(j)->getTerminator();
                        unsigned operandIdx = instrToIndex(operandInstr);
                        if (operandIdx != UINT_MAX && outInstrK == blockLabel) {
                            outInfoK->add(operandIdx);
                        }
                    }
                }

                Infos.push_back(outInfoK);
            }
        }
    }
    
public:
    LivenessAnalysis (LivenessInfo &bottom, LivenessInfo &initialState) : DataFlowAnalysis(bottom, initialState) {}
};

namespace {
    
    struct LivenessAnalysisPass : public FunctionPass {
        static char ID;
        LivenessAnalysisPass() : FunctionPass(ID) {}
        
        bool runOnFunction(Function &F) override {
            LivenessInfo bottom, initialState;
            LivenessAnalysis LA(bottom, initialState);
            LA.runWorklistAlgorithm(&F);
            LA.print();
            return false; // Function is not modified
        }
    }; // end of struct LivenessAnalysisPass
}  // end of anonymous namespace

char LivenessAnalysisPass::ID = 0;
static RegisterPass<LivenessAnalysisPass> X("cse231-liveness", "Liveness Analysis",
                                            false /* Only looks at CFG */,
                                            false /* Analysis Pass */);

