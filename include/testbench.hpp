#ifndef SHOAL_INCLUDE_TESTBENCH_H_
#define SHOAL_INCLUDE_TESTBENCH_H_

#ifdef DEBUG
#include <sstream>
#define CHECK_STATE(state, stateInt, caseInt) \
    case caseInt: { \
        if(stateInt == caseInt) \
            return state; \
        else{ \
            std::stringstream sstm; \
            sstm << "Error - State Mismatch: Expected " << stateInt << " Received " << caseInt; \
            std::string result = sstm.str(); \
            return result; \
        } \
    }
#endif

#endif // SHOAL_INCLUDE_TESTBENCH_H_