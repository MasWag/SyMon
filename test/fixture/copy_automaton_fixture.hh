#pragma once

#include <sstream>

#include "automaton.hh"
#include "signature.hh"

/*
  @brief This automaton accepts "copy" behavior.
*/
struct CopyFixture {
    CopyFixture() {
        // Construct signature
        std::stringstream sigStream;
        sigStream << "update\t0\t1";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton
        automaton.states.resize(4);
        for (auto &state: automaton.states) {
            state = std::make_shared<NonParametricTAState<int> >(false);
        }
        automaton.initialStates = {automaton.states.front()};
        automaton.states[0]->isMatch = false;
        automaton.states[1]->isMatch = false;
        automaton.states[2]->isMatch = false;
        automaton.states[3]->isMatch = true;

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 0;
        automaton.numberVariableSize = 1;

        // #### FROM STATE 0 ####
        automaton.states[0]->next[0].resize(2);

        // Self loop at state 0
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;

            automaton.states[0]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[0]
            };
        }

        // Transition from state 0 to state 1 (when input is "y")
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == "y");

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;

            NonSymbolic::Update update;
            update.numberUpdate.emplace_back(VariableID{0}, VariableID{1});

            std::vector<VariableID> resetVars = {VariableID{0}};

            automaton.states[0]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                {}, // Empty guard
                automaton.states[1]
            };
        }

        // #### FROM STATE 1 ####
        automaton.states[1]->next[0].resize(3);

        // First self-loop at state 1 (x0 == "x" && x0 != x1)
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == "x");

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            numConstraints.push_back(NonSymbolic::NCMakerVar<int>(0) != NonSymbolic::NCMakerVar<int>(1));

            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                std::move(guard),
                automaton.states[1]
            };
        }

        // Second self-loop at state 1 (x0 != "x")
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) != "x");

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[1]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                std::move(guard),
                automaton.states[1]
            };
        }

        // Transition from state 1 to state 2 (x0 == "x" && x0 == x1)
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == "x");

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            numConstraints.push_back(NonSymbolic::NCMakerVar<int>(0) == NonSymbolic::NCMakerVar<int>(1));

            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[1]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                std::move(guard),
                automaton.states[2]
            };
        }

        // #### FROM STATE 2 ####
        automaton.states[2]->next[0].resize(4);

        // First self-loop at state 2 (x0 == "x" && x0 == x1)
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == "x");

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            numConstraints.push_back(NonSymbolic::NCMakerVar<int>(0) == NonSymbolic::NCMakerVar<int>(1));

            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) <= 5);

            automaton.states[2]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                std::move(guard),
                automaton.states[2]
            };
        }

        // Second self-loop at state 2 (x0 != "x")
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) != "x");

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) <= 5);

            automaton.states[2]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                std::move(guard),
                automaton.states[2]
            };
        }

        // Transition from state 2 to state 1 (x0 == "x" && x0 != x1)
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            stringConstraints.push_back(NonSymbolic::SCMaker(0) == "x");

            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            numConstraints.push_back(NonSymbolic::NCMakerVar<int>(0) != NonSymbolic::NCMakerVar<int>(1));

            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[2]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                std::move(guard),
                automaton.states[1]
            };
        }

        // Transition from state 2 to state 3 (clock > 5)
        {
            std::vector<NonSymbolic::StringConstraint> stringConstraints;
            std::vector<NonSymbolic::NumberConstraint<int> > numConstraints;
            NonSymbolic::Update update;
            std::vector<VariableID> resetVars;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) > 5);

            automaton.states[2]->next[0][3] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                std::move(guard),
                automaton.states[3]
            };
        }
    }

    NonParametricTA<int> automaton;
    std::unique_ptr<Signature> signature;
};


struct DataParametricCopy {
    DataParametricTA automaton;
    std::unique_ptr<Signature> signature;

    DataParametricCopy() {
        // Construct signature
        std::stringstream sigStream;
        sigStream << "update\t0\t1";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton
        automaton.states.resize(4);
        for (auto &state: automaton.states) {
            state = std::make_shared<DataParametricTAState>(false);
        }
        automaton.initialStates = {automaton.states.front()};
        automaton.states[0]->isMatch = false;
        automaton.states[1]->isMatch = false;
        automaton.states[2]->isMatch = false;
        automaton.states[3]->isMatch = true;

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 0;
        automaton.numberVariableSize = 1;

        // #### FROM STATE 0 ####
        automaton.states[0]->next[0].resize(2);

        // Self loop at state 0
        automaton.states[0]->next[0][0] = {{}, {}, {}, {}, {}, automaton.states[0]};

        // Transition from state 0 to state 1
        {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "y");

            std::vector<NumberConstraint> numConstraints;

            Update update;
            update.numberUpdate.emplace_back(VariableID{0}, Variable(1));

            automaton.states[0]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                {VariableID{0}}, // resetVars
                {}, // guard
                automaton.states[1]
            };
        }

        // #### FROM STATE 1 ####
        automaton.states[1]->next[0].resize(4);

        // First self-loop at state 1 (x0 == 'x' && x0 > x1)
        {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) > Variable(1));

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[1]
            };
        }

        // Second self-loop at state 1 (x0 == 'x' && x0 < x1)
        {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) < Variable(1));

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[1]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[1]
            };
        }

        // Third self-loop at state 1 (x0 != 'x')
        {
            using namespace Symbolic;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) != "x");

            std::vector<NumberConstraint> numConstraints;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[1]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[1]
            };
        }

        // Transition from state 1 to state 2 (x0 == 'x' && x0 == x1)
        {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) == Variable(1));

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[1]->next[0][3] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[2]
            };
        }

        // #### FROM STATE 2 ####
        automaton.states[2]->next[0].resize(5);

        // First self-loop at state 2 (x0 == 'x' && x0 == x1)
        {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) == Variable(1));

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 5);

            automaton.states[2]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[2]
            };
        }

        // Second self-loop at state 2 (x0 != 'x')
        {
            using namespace Symbolic;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) != "x");

            std::vector<NumberConstraint> numConstraints;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 5);

            automaton.states[2]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[2]
            };
        }

        // First transition from state 2 to state 1 (x0 == 'x' && x0 > x1)
        {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) > Variable(1));

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[2]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[1]
            };
        }

        // Second transition from state 2 to state 1 (x0 == 'x' && x0 < x1)
        {
            using namespace Symbolic;
            using namespace Parma_Polyhedra_Library;

            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) < Variable(1));

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) < 3);

            automaton.states[2]->next[0][3] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[1]
            };
        }

        // Transition from state 2 to state 3
        {
            using namespace Symbolic;

            std::vector<StringConstraint> stringConstraints;
            std::vector<NumberConstraint> numConstraints;

            std::vector<TimingConstraint> guard;
            guard.push_back(ConstraintMaker(0) > 5);

            automaton.states[2]->next[0][4] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                {}, // update
                {}, // resetVars
                std::move(guard),
                automaton.states[3]
            };
        }
    }
};

struct ParametricCopy {
    ParametricTA automaton;
    std::unique_ptr<Signature> signature;

    ParametricCopy() {
        using namespace Parma_Polyhedra_Library;
        using namespace Symbolic;

        // Construct signature
        std::stringstream sigStream;
        sigStream << "update\t0\t1";
        signature = std::make_unique<Signature>(sigStream);

        // Construct automaton
        automaton.states.resize(4);
        for (auto &state: automaton.states) {
            state = std::make_shared<PTAState>(false);
        }
        automaton.initialStates = {automaton.states.front()};
        automaton.states[0]->isMatch = false;
        automaton.states[1]->isMatch = false;
        automaton.states[2]->isMatch = false;
        automaton.states[3]->isMatch = true;

        automaton.clockVariableSize = 1;
        automaton.stringVariableSize = 0;
        automaton.numberVariableSize = 1;
        automaton.parameterSize = 0; // No parameters for this example

        // Create parametric timing constraints
        ParametricTimingConstraint lessThan3 = ParametricTimingConstraint(1);
        lessThan3.add_constraint(Variable(0) < 3);

        ParametricTimingConstraint lessThanEqual5 = ParametricTimingConstraint(1);
        lessThanEqual5.add_constraint(Variable(0) <= 5);

        ParametricTimingConstraint greaterThan5 = ParametricTimingConstraint(1);
        greaterThan5.add_constraint(Variable(0) > 5);

        // #### FROM STATE 0 ####
        automaton.states[0]->next[0].resize(2);

        // Self loop at state 0
        {
            std::vector<StringConstraint> stringConstraints;
            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[0]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                ParametricTimingConstraint(1), // Empty constraint
                automaton.states[0]
            };
        }

        // Transition from state 0 to state 1 (when input is "y")
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "y");

            std::vector<NumberConstraint> numConstraints;

            Update update;
            update.numberUpdate.emplace_back(VariableID{0}, NumberExpression(Variable(1)));

            std::vector<VariableID> resetVars = {VariableID{0}};

            automaton.states[0]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                ParametricTimingConstraint(1), // Empty constraint
                automaton.states[1]
            };
        }

        // #### FROM STATE 1 ####
        automaton.states[1]->next[0].resize(4);

        // First self-loop at state 1 (x0 == "x" && x0 > x1)
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) > Variable(1));

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThan3,
                automaton.states[1]
            };
        }

        // Second self-loop at state 1 (x0 == "x" && x0 < x1)
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) < Variable(1));

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThan3,
                automaton.states[1]
            };
        }

        // Third self-loop at state 1 (x0 != "x")
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) != "x");

            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThan3,
                automaton.states[1]
            };
        }

        // Transition from state 1 to state 2 (x0 == "x" && x0 == x1)
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) == Variable(1));

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[1]->next[0][3] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThan3,
                automaton.states[2]
            };
        }

        // #### FROM STATE 2 ####
        automaton.states[2]->next[0].resize(5);

        // First self-loop at state 2 (x0 == "x" && x0 == x1)
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) == Variable(1));

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[2]->next[0][0] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThanEqual5,
                automaton.states[2]
            };
        }

        // Second self-loop at state 2 (x0 != "x")
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) != "x");

            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[2]->next[0][1] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThanEqual5,
                automaton.states[2]
            };
        }

        // First transition from state 2 to state 1 (x0 == "x" && x0 > x1)
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) > Variable(1));

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[2]->next[0][2] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThan3,
                automaton.states[1]
            };
        }

        // Second transition from state 2 to state 1 (x0 == "x" && x0 < x1)
        {
            std::vector<StringConstraint> stringConstraints;
            stringConstraints.push_back(SCMaker(0) == "x");

            std::vector<NumberConstraint> numConstraints;
            numConstraints.push_back(Variable(0) < Variable(1));

            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[2]->next[0][3] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                lessThan3,
                automaton.states[1]
            };
        }

        // Transition from state 2 to state 3
        {
            std::vector<StringConstraint> stringConstraints;
            std::vector<NumberConstraint> numConstraints;
            Update update;
            std::vector<VariableID> resetVars;

            automaton.states[2]->next[0][4] = {
                std::move(stringConstraints),
                std::move(numConstraints),
                std::move(update),
                std::move(resetVars),
                greaterThan5,
                automaton.states[3]
            };
        }
    }
};