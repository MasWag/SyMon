#pragma once

#include <boost/lexical_cast.hpp>
#include <sstream>

#include "automaton.hh"
#include "signature.hh"
#include "parametric_timing_constraint_helper.hh"

namespace NonParametric {
    /*
    @brief Automaton with a guard restricting time to non-integer values.
    */
    struct NonIntegerTimestampFixture {
        NonParametricTA<double, double> automaton;
        std::unique_ptr<Signature> signature;
        NonIntegerTimestampFixture() {
            // Construct signature
            std::stringstream sigStream;
            sigStream << "event" << 0 << 1;
            signature = std::make_unique<Signature>(sigStream);

            // Construct automaton with 2 states
            automaton.states.resize(2);
            for (auto &state: automaton.states) {
                state = std::make_shared<NonParametricTAState<double, double> >(false);
            }
            automaton.initialStates = {automaton.states[0]};
            automaton.states[0]->isMatch = false;  // Initial state is not final
            automaton.states[1]->isMatch = true;   // Last state is final

            automaton.clockVariableSize = 1;
            automaton.stringVariableSize = 0;
            automaton.numberVariableSize = 0;

            // Transition from state 0 to state 0 anytime
            {
                std::vector<NonSymbolic::StringConstraint> stringConstraints;
                std::vector<NonSymbolic::NumberConstraint<double> > numConstraints;
                NonSymbolic::Update<double> update;
                std::vector<VariableID> resetVars = {VariableID{0}};
                
                automaton.states[0]->next[0].resize(2);
                automaton.states[0]->next[0][0] = {
                    std::move(stringConstraints),
                    std::move(numConstraints),
                    std::move(update),
                    std::move(resetVars),
                    {}, // Empty guard
                    automaton.states[0]
                };
            }
            // Transition from state 0 to state 1 on event
            {
                std::vector<NonSymbolic::StringConstraint> stringConstraints;
                std::vector<NonSymbolic::NumberConstraint<double> > numConstraints;
                NonSymbolic::Update<double> update;
                std::vector<VariableID> resetVars;
                std::vector<TimingConstraint<double>> guard;
                guard.push_back(ConstraintMaker(0) >= 1.1);
                guard.push_back(ConstraintMaker(0) < 1.2);
                
                automaton.states[0]->next[0][1] = {
                    std::move(stringConstraints),
                    std::move(numConstraints),
                    std::move(update),
                    std::move(resetVars),
                    {std::move(guard)},
                    automaton.states[1]
                };
            }
        }
    };

} // namespace NonParametric

namespace Parametric {
    /*
    @brief Data parametric version of the non-integer timestamp automaton fixture.
    */
    struct DataParametricNonIntegerTimestampFixture {
        DataParametricTA<double> automaton;
        std::unique_ptr<Signature> signature;
        DataParametricNonIntegerTimestampFixture() {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            // Construct signature
            std::stringstream sigStream;
            sigStream << "event" << 0 << 1;
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

            // Transition from state 0 to state 0 anytime
            {
                std::vector<StringConstraint> stringConstraints;
                std::vector<NumberConstraint> numConstraints;
                Update update;
                std::vector<VariableID> resetVars = {VariableID{0}};
                
                automaton.states[0]->next[0].resize(2);
                automaton.states[0]->next[0][0] = {
                    std::move(stringConstraints),
                    std::move(numConstraints),
                    std::move(update),
                    std::move(resetVars),
                    {}, // Empty guard
                    automaton.states[0]
                };
            }
            // Transition from state 0 to state 1 on event
            {
                std::vector<StringConstraint> stringConstraints;
                std::vector<NumberConstraint> numConstraints;
                Update update;
                std::vector<VariableID> resetVars;
                std::vector<TimingConstraint<double>> guard;
                guard.push_back(ConstraintMaker(0) >= 1.1);
                guard.push_back(ConstraintMaker(0) < 1.2);
                
                automaton.states[0]->next[0][1] = {
                    std::move(stringConstraints),
                    std::move(numConstraints),
                    std::move(update),
                    std::move(resetVars),
                    {std::move(guard)},
                    automaton.states[1]
                };
            }
        }
    };

    /*
    @brief Parametric version of the non-integer timestamp automaton fixture.
    */
    struct ParametricNonIntegerTimestampFixture {
        ParametricTA automaton;
        std::unique_ptr<Signature> signature;
        ParametricNonIntegerTimestampFixture() {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            // Construct signature
            std::stringstream sigStream;
            sigStream << "event" << 0 << 1;
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
            automaton.numberVariableSize = 1;
            automaton.parameterSize = 0;

            // Transition from state 0 to state 0 anytime
            {
                std::vector<StringConstraint> stringConstraints;
                std::vector<NumberConstraint> numConstraints;
                Update update;
                std::vector<VariableID> resetVars = {VariableID{0}};
                
                automaton.states[0]->next[0].resize(2);
                automaton.states[0]->next[0][0] = {
                    std::move(stringConstraints),
                    std::move(numConstraints),
                    std::move(update),
                    std::move(resetVars),
                    ParametricTimingConstraint(1), // Empty constraint
                    automaton.states[0]
                };
            }
            // Transition from state 0 to state 1 on event
            {
                std::vector<StringConstraint> stringConstraints;
                std::vector<NumberConstraint> numConstraints;
                Update update;
                std::vector<VariableID> resetVars;
                auto c0 = boost::lexical_cast<ParametricTimingConstraintHelper>("x0 >= 1.1");
                auto c1 = boost::lexical_cast<ParametricTimingConstraintHelper>("x0 < 1.2");
                Parma_Polyhedra_Library::Constraint g0, g1;
                c0.extract(0, g0);
                c1.extract(0, g1);
                ParametricTimingConstraint guard(1);
                guard.add_constraint(g0);
                guard.add_constraint(g1);
                
                automaton.states[0]->next[0][1] = {
                    std::move(stringConstraints),
                    std::move(numConstraints),
                    std::move(update),
                    std::move(resetVars),
                    std::move(guard),
                    automaton.states[1]
                };
            }
        }
    };

} // namespace Parametric
