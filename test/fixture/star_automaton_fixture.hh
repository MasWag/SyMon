#pragma once

#include <sstream>

#include "automaton.hh"
#include "signature.hh"

namespace NonParametric {

/*
  @brief This automaton is specifically designed to test the star operation.
  It has a simple structure with a clear initial and final state.
*/
struct StarAutomatonFixture {
    StarAutomatonFixture() {
        // Construct signature
        std::stringstream sigStream;
        sigStream << "event\t0\t0";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton with 3 states
        automaton.states.resize(3);
        for (auto &state: automaton.states) {
            state = std::make_shared<NonParametricTAState<int> >(false);
        }
        automaton.initialStates = {automaton.states[0]};
        automaton.states[0]->isMatch = false;  // Initial state is not final
        automaton.states[1]->isMatch = false;  // Middle state is not final
        automaton.states[2]->isMatch = true;   // Last state is final

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 0;
        automaton.numberVariableSize = 0;

        // Transition from state 0 to state 1 on event 'a'
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == "a");
            
            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update update;
            std::vector<VariableID> resetVars = {VariableID{0}};
            
            automaton.states[0]->next[0].resize(1);
            automaton.states[0]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[1]
            };
        }

        // Transition from state 1 to state 2 on event 'b'
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == "b");
            
            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;
            
            automaton.states[1]->next[0].resize(1);
            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[2]
            };
        }
    }

    NonParametricTA<int> automaton;
    std::unique_ptr<Signature> signature;
};

} // namespace NonParametric

namespace NonParametric {
/*
  @brief Data parametric version of the star automaton fixture.
*/
struct DataParametricStarAutomaton {
    DataParametricStarAutomaton() {
        using namespace Symbolic;
        using namespace Parma_Polyhedra_Library;

        // Construct signature
        std::stringstream sigStream;
        sigStream << "event\t0\t0";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton with 3 states
        automaton.states.resize(3);
        for (auto &state: automaton.states) {
            state = std::make_shared<DataParametricTAState>(false);
        }
        automaton.initialStates = {automaton.states[0]};
        automaton.states[0]->isMatch = false;  // Initial state is not final
        automaton.states[1]->isMatch = false;  // Middle state is not final
        automaton.states[2]->isMatch = true;   // Last state is final

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 0;
        automaton.numberVariableSize = 0;

        // Transition from state 0 to state 1 on event 'a'
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "a");
            
            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars = {VariableID{0}};
            
            automaton.states[0]->next[0].resize(1);
            automaton.states[0]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[1]
            };
        }

        // Transition from state 1 to state 2 on event 'b'
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "b");
            
            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;
            
            automaton.states[1]->next[0].resize(1);
            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[2]
            };
        }
    }

    DataParametricTA automaton;
    std::unique_ptr<Signature> signature;
};

/*
  @brief Parametric version of the star automaton fixture.
*/
struct ParametricStarAutomaton {
    ParametricStarAutomaton() {
        using namespace Symbolic;
        using namespace Parma_Polyhedra_Library;

        // Construct signature
        std::stringstream sigStream;
        sigStream << "event\t0\t0";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton with 3 states
        automaton.states.resize(3);
        for (auto &state: automaton.states) {
            state = std::make_shared<PTAState>(false);
        }
        automaton.initialStates = {automaton.states[0]};
        automaton.states[0]->isMatch = false;  // Initial state is not final
        automaton.states[1]->isMatch = false;  // Middle state is not final
        automaton.states[2]->isMatch = true;   // Last state is final

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 0;
        automaton.numberVariableSize = 0;
        automaton.parameterSize = 0;

        // Transition from state 0 to state 1 on event 'a'
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "a");
            
            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars = {VariableID{0}};
            
            automaton.states[0]->next[0].resize(1);
            automaton.states[0]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                ParametricTimingConstraint(1), // Empty constraint
                automaton.states[1]
            };
        }

        // Transition from state 1 to state 2 on event 'b'
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "b");
            
            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;
            
            automaton.states[1]->next[0].resize(1);
            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                ParametricTimingConstraint(1), // Empty constraint
                automaton.states[2]
            };
        }
    }

    ParametricTA automaton;
    std::unique_ptr<Signature> signature;
};

} // namespace NonParametric