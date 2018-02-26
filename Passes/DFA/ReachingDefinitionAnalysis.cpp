#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>
#include <set>

using namespace llvm;
using namespace std;

class ReachingInfo : public Info {
private:
    set<unsigned> data;
    
public:
    void add(unsigned d) {
        data.insert(d);
    }
    
    void print() {
        for (auto d : data) {
            errs() << d << '|';
        }
        errs() << '\n';
    }
    
    static bool equals(ReachingInfo * info1, ReachingInfo * info2) {
        return info1->data == info2->data;
    }
    
    static void join(ReachingInfo * info1, ReachingInfo * info2, ReachingInfo * result) {
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

class ReachingDefinitionAnalysis : public DataFlowAnalysis<ReachingInfo, true> {
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
                              std::vector<ReachingInfo *> & Infos) {
        int category = 2; // Default to second category
        string name = I->getOpcodeName();
        if (categoryMap.count(name) > 0) {
            category = categoryMap[name];
        } else if (I->isBinaryOp()) {
            category = 1;
        }
        
        unsigned myNode = instrToIndex(I);
        
        // Join all incoming info
        ReachingInfo *outInfo = new ReachingInfo();
        for (unsigned node : IncomingEdges) {
            ReachingInfo *incomingInfo = edgeToInfo({node, myNode});
            ReachingInfo::join(outInfo, incomingInfo, outInfo);
        }
        
        // Add self to the result
        if (category == 1) {
            outInfo->add(myNode);
        } else if (category == 3) {
            unsigned firstNonPhiNode = instrToIndex(I->getParent()->getFirstNonPHI());
            for (unsigned i = myNode; i < firstNonPhiNode; i++) {
                outInfo->add(i);
            }
        }
        
        // Copy output to every edge
        for (unsigned i = 0; i < OutgoingEdges.size(); i++) {
            Infos.push_back(outInfo);
        }
    }
    
public:
    ReachingDefinitionAnalysis (ReachingInfo &bottom, ReachingInfo &initialState) : DataFlowAnalysis(bottom, initialState) {}
};

namespace {
    
    struct ReachingDefinitionAnalysisPass : public FunctionPass {
        static char ID;
        ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}
        
        bool runOnFunction(Function &F) override {
            ReachingInfo bottom, initialState;
            ReachingDefinitionAnalysis RDA(bottom, initialState);
            RDA.runWorklistAlgorithm(&F);
            RDA.print();
            return false; // Function is not modified
        }
    }; // end of struct ReachingDefinitionAnalysisPass
}  // end of anonymous namespace

char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cse231-reaching", "Reaching Definition Analysis",
                                               false /* Only looks at CFG */,
                                               false /* Analysis Pass */);

