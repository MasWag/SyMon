#pragma once

#include <sstream>

#include "automaton.hh"
#include "signature.hh"

namespace NonParametric {

/*
  @brief This automaton is specifically designed to test the timeRestriction operation.
  It has a simple structure with a clear initial and final state.
*/
struct TimeRestrictionFixture {
    TimeRestrictionFixture() {
        // Construct signature
        std::stringstream sigStream;
        sigStream << "event\t0\t0";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton with 2 states
        automaton.states.resize(2);
        for (auto &state: automaton.states) {
            state = std::make_shared<NonParametricTAState<int, double> >(false);
        }
        automaton.initialStates = {automaton.states[0]};
        automaton.states[0]->isMatch = false;  // Initial state is not final
        automaton.states[1]->isMatch = true;   // Last state is final

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 0;
        automaton.numberVariableSize = 0;

        // Transition from state 0 to state 1 on event 'a'
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == "a");
            
            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update<int> update;
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
    }

    NonParametricTA<int, double> automaton;
    std::unique_ptr<Signature> signature;
};

} // namespace NonParametric

namespace NonParametric {
/*
  @brief Data parametric version of the time restriction automaton fixture.
*/
struct DataParametricTimeRestrictionFixture {
    DataParametricTimeRestrictionFixture() {
        using namespace Symbolic;
        using namespace Parma_Polyhedra_Library;

        // Construct signature
        std::stringstream sigStream;
        sigStream << "event\t0\t0";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton with 2 states
        automaton.states.resize(2);
        for (auto &state: automaton.states) {
            state = std::make_shared<DataParametricTAState<double>>(false);
        }
        automaton.initialStates = {automaton.states[0]};
        automaton.states[0]->isMatch = false;  // Initial state is not final
        automaton.states[1]->isMatch = true;   // Last state is final

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
    }

    DataParametricTA<double> automaton;
    std::unique_ptr<Signature> signature;
};

/*
  @brief Parametric version of the time restriction automaton fixture.
*/
struct ParametricTimeRestrictionFixture {
    ParametricTimeRestrictionFixture() {
        using namespace Symbolic;
        using namespace Parma_Polyhedra_Library;

        // Construct signature
        std::stringstream sigStream;
        sigStream << "event\t0\t0";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton with 2 states
        automaton.states.resize(2);
        for (auto &state: automaton.states) {
            state = std::make_shared<PTAState>(false);
        }
        automaton.initialStates = {automaton.states[0]};
        automaton.states[0]->isMatch = false;  // Initial state is not final
        automaton.states[1]->isMatch = true;   // Last state is final

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
    }

    ParametricTA automaton;
    std::unique_ptr<Signature> signature;
};

} // namespace NonParametric
