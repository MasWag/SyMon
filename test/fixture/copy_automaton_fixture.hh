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
      state = std::make_shared<NonParametricTAState<int>>(false);
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
    automaton.states[0]->next[0][0] =
      {{}, {}, {}, {}, {}, automaton.states[0]};
    automaton.states[0]->next[0][1] =
            {{NonSymbolic::SCMaker(0) == "y"}, // stringConstraints
       {}, // numConstraints
       {{},  // stringUpdates
        {{0, 1}}}, // numberUpdates
       {0}, // resetVars
       {}, // guard
       automaton.states[1]};

    // #### FROM STATE 1 ####
    automaton.states[1]->next[0].resize(3);
    {
      std::vector<NonSymbolic::NumberConstraint<int>> vec;
      vec.push_back({NonSymbolic::NCMakerVar<int>(0) != NonSymbolic::NCMakerVar<int>(1)});
      automaton.states[1]->next[0][0] =
              {{NonSymbolic::SCMaker(0) == "x"}, // stringConstraints
         std::move(vec), // numConstraints
         {}, // Updates
         {}, // resetVars
         {{ConstraintMaker(0) < 3}}, // guard
         automaton.states[1]};
    }
    automaton.states[1]->next[0][1] =
            {{NonSymbolic::SCMaker(0) != "x"}, // stringConstraints
       {}, // numConstraints
       {}, // Updates
       {}, // resetVars
       {{ConstraintMaker(0) < 3}}, // guard
       automaton.states[1]};
    {
      std::vector<NonSymbolic::NumberConstraint<int>> vec;
      vec.push_back({NonSymbolic::NCMakerVar<int>(0) == NonSymbolic::NCMakerVar<int>(1)});
      automaton.states[1]->next[0][2] =
              {{NonSymbolic::SCMaker(0) == "x"}, // stringConstraints
         std::move(vec), // numConstraints
         {}, // Updates
         {}, // resetVars
         {{ConstraintMaker(0) < 3}}, // guard
         automaton.states[2]};
    }

    // #### FROM STATE 2 ####
    automaton.states[2]->next[0].resize(4);
    {
      std::vector<NonSymbolic::NumberConstraint<int>> vec;
      vec.push_back({NonSymbolic::NCMakerVar<int>(0) == NonSymbolic::NCMakerVar<int>(1)});
      automaton.states[2]->next[0][0] =
              {{NonSymbolic::SCMaker(0) == "x"}, // stringConstraints
         std::move(vec), // numConstraints
         {}, // Updates
         {}, // resetVars
         {{ConstraintMaker(0) <= 5}}, // guard
         automaton.states[2]};
    }
    automaton.states[2]->next[0][1] =
            {{NonSymbolic::SCMaker(0) != "x"}, // stringConstraints
       {}, // numConstraints
       {}, // Updates
       {}, // resetVars
       {{ConstraintMaker(0) <= 5}}, // guard
       automaton.states[2]};
    {
      std::vector<NonSymbolic::NumberConstraint<int>> vec;
      vec.push_back({NonSymbolic::NCMakerVar<int>(0) != NonSymbolic::NCMakerVar<int>(1)});
      automaton.states[2]->next[0][2] =
              {{NonSymbolic::SCMaker(0) == "x"}, // stringConstraints
         std::move(vec), // numConstraints
         {}, // Updates
         {}, // resetVars
         {{ConstraintMaker(0) < 3}}, // guard
         automaton.states[1]};
    }
    automaton.states[2]->next[0][3] =
      {{}, // stringConstraints
       {}, // numConstraints
       {}, // Updates
       {}, // resetVars
       {{ConstraintMaker(0) > 5}}, // guard
       automaton.states[3]};
  }

  NonParametricTA<int> automaton;
  std::unique_ptr<Signature> signature;
};
