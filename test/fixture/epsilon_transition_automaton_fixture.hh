#pragma once

#include <cstring>
#include <cerrno>
#include <iostream>
#include <sstream>
#include "../src/automaton.hh"
#include "../src/automaton_parser.hh"


class AutomatonFixture {
    private:
        template<typename TAType, typename BoostTAType>
        static TAType parseDotTA(const std::string& dotStr) {
            std::istringstream taStream(dotStr);
            TAType TA;
            BoostTAType BoostTA;
            parseBoostTA(taStream, BoostTA);
            convBoostTA(BoostTA, TA);
            return TA;
        }
        std::string dotString;

    public:
        AutomatonFixture(std::string dotStr):dotString(dotStr) {}

        auto makeBooleanTA() const {
            using TAType = NonParametricTA<int>;
            using BoostTAType = NonParametricBoostTA<int>;
            return parseDotTA<TAType, BoostTAType>(dotString);
        }

        auto makeDataParametricTA() const {
            using TAType = DataParametricTA;
            using BoostTAType = DataParametricBoostTA;
            return parseDotTA<TAType, BoostTAType>(dotString);
        }
};

namespace EpsilonTransitionAutomatonFixture {
    const char DOT_EPS1[] = R"DOT(digraph G {
        graph [
            clock_variable_size = 1
            string_variable_size = 1
            number_variable_size = 0
            parameter_size = 0
        ]
        0 [init=1][match=0]
        1 [init=0][match=0]
        2 [init=0][match=0]
        3 [init=0][match=0]
        4 [init=0][match=1]
        0 -> 0 [label=0]
        0 -> 1 [label=0][s_constraints="{x1 == 'a'}"][reset="{0}"]
        1 -> 2 [label=127][s_constraints="{x0 == 'z'}"][guard="{x0 == 2}"]
        2 -> 3 [label=127][s_constraints="{x0 == 'z'}"][guard="{x0 == 3}"]
        3 -> 4 [label=0][s_constraints="{x1 == 'b'}"][guard="{x0 > 4}"]

    })DOT";
    inline const AutomatonFixture FIXTURE1{DOT_EPS1};

    //Epsilon transition with no guard
    const char DOT_EPS2[] = R"DOT(digraph G {
        graph [
            clock_variable_size = 1
            string_variable_size = 1
            number_variable_size = 0
            parameter_size = 0
        ]
        0 [init=1][match=0]
        1 [init=0][match=0]
        2 [init=0][match=0]
        3 [init=0][match=1]
        0 -> 0 [label=0]
        0 -> 1 [label=0][s_constraints="{x1 == 'a'}"][reset="{0}"]
        1 -> 2 [label=127][s_constraints="{x0 == 'z'}"]
        2 -> 3 [label=0][s_constraints="{x1 == 'b', x0 == 'z'}"][guard="{x0 > 4}"]
    })DOT";
    inline const AutomatonFixture FIXTURE2{DOT_EPS2};

    // 2 clock variables
    const char DOT_EPS3[] = R"DOT(digraph G {
        graph [
            clock_variable_size = 2
            string_variable_size = 1
            number_variable_size = 0
            parameter_size = 0
        ]
        0 [init=1][match=0]
        1 [init=0][match=0]
        2 [init=0][match=0]
        3 [init=0][match=0]
        4 [init=0][match=0]
        5 [init=0][match=1]
        0 -> 0 [label=0]
        0 -> 1 [label=0][s_constraints="{x1 == 'a'}"][reset="{0, 1}"]
        1 -> 3 [label=0][s_constraints="{x1 == 'b'}"][reset="{0}"]
        0 -> 2 [label=0][s_constraints="{x1 == 'b'}"][reset="{0, 1}"]
        2 -> 3 [label=0][s_constraints="{x1 == 'a'}"][reset="{1}"]
        3 -> 4 [label=127][guard="{x0 == 2, x1 == 1}"]
        4 -> 5 [label=0][s_constraints="{x1 == 'c'}"]
    })DOT";
    inline const AutomatonFixture FIXTURE3{DOT_EPS3};

    //Epsilon transition at last to accept state
    const char DOT_EPS4[] = R"DOT(digraph G {
        graph [
            clock_variable_size = 1
            string_variable_size = 1
            number_variable_size = 0
            parameter_size = 0
        ]
        0 [init=1][match=0]
        1 [init=0][match=0]
        2 [init=0][match=1]
        0 -> 0 [label=0]
        0 -> 1 [label=0][s_constraints="{x1 == 'b'}"][reset="{0}"]
        1 -> 2 [label=127][guard="{x0 == 3}"]

    })DOT";
    inline const AutomatonFixture FIXTURE4{DOT_EPS4};

} // namespace EpsilonTransitionAutomatonFixture
