#include <boost/test/unit_test.hpp>

#include "fixture/copy_automaton_fixture.hh"
#include "fixture/withdraw_automaton_fixture.hh"

#include "automata_operation.hh"

BOOST_AUTO_TEST_SUITE(AutomatonOperationTest)
    BOOST_AUTO_TEST_SUITE(NonParametric)

        struct CopyAndWithdrawFixture : CopyFixture, WithdrawFixture {
        };

        BOOST_FIXTURE_TEST_CASE(DisjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->CopyFixture::automaton;
            auto withdraw = this->WithdrawFixture::automaton;

            const size_t copyStateCount = copy.states.size();
            const size_t copyInitialStateCount = copy.initialStates.size();
            const size_t withdrawStateCount = withdraw.states.size();
            const size_t withdrawInitialStateCount = withdraw.initialStates.size();

            const auto result = disjunction(std::move(copy), std::move(withdraw));

            BOOST_CHECK_EQUAL(result.states.size(), copyStateCount + withdrawStateCount);
            BOOST_CHECK_EQUAL(result.initialStates.size(), copyInitialStateCount + withdrawInitialStateCount);
        }

    BOOST_AUTO_TEST_SUITE_END() // NonParametric

    BOOST_AUTO_TEST_SUITE(DataParametric)

        struct CopyAndWithdrawFixture : DataParametricCopy, DataParametricWithdrawFixture {
        };

        BOOST_FIXTURE_TEST_CASE(DisjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->DataParametricCopy::automaton;
            auto withdraw = this->DataParametricWithdrawFixture::automaton;

            const size_t copyStateCount = copy.states.size();
            const size_t copyInitialStateCount = copy.initialStates.size();
            const size_t withdrawStateCount = withdraw.states.size();
            const size_t withdrawInitialStateCount = withdraw.initialStates.size();

            const auto result = disjunction(std::move(copy), std::move(withdraw));

            BOOST_CHECK_EQUAL(result.states.size(), copyStateCount + withdrawStateCount);
            BOOST_CHECK_EQUAL(result.initialStates.size(), copyInitialStateCount + withdrawInitialStateCount);
        }

    BOOST_AUTO_TEST_SUITE_END() // DataParametric

    BOOST_AUTO_TEST_SUITE(Parametric)

        struct CopyAndWithdrawFixture : ParametricCopy, ParametricWithdrawFixture {
        };

        BOOST_FIXTURE_TEST_CASE(DisjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->ParametricCopy::automaton;
            auto withdraw = this->ParametricWithdrawFixture::automaton;

            const size_t copyStateCount = copy.states.size();
            const size_t copyInitialStateCount = copy.initialStates.size();
            const size_t withdrawStateCount = withdraw.states.size();
            const size_t withdrawInitialStateCount = withdraw.initialStates.size();

            const auto result = disjunction(std::move(copy), std::move(withdraw));

            BOOST_CHECK_EQUAL(result.states.size(), copyStateCount + withdrawStateCount);
            BOOST_CHECK_EQUAL(result.initialStates.size(), copyInitialStateCount + withdrawInitialStateCount);
        }

    BOOST_AUTO_TEST_SUITE_END() // Parametric

BOOST_AUTO_TEST_SUITE_END() // AutomatonOperationTest
