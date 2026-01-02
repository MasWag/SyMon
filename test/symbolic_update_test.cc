#include <boost/test/unit_test.hpp>
#include "symbolic_number_constraint.hh"
#include "symbolic_update.hh"

BOOST_AUTO_TEST_SUITE(SymbolicDataConstraintsTest)

  BOOST_AUTO_TEST_SUITE(EvalTest)

    BOOST_AUTO_TEST_CASE(stringConstantEqPass) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      Symbolic::StringValuation stringEnv = {"Alice"};
      std::vector<Symbolic::StringConstraint> stringConstraints = {Symbolic::SCMaker(0) == "Alice"};
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      BOOST_TEST(Symbolic::eval(stringConstraints, stringEnv, numberConstraints, numEnv));
    }

    BOOST_AUTO_TEST_CASE(stringConstantNeFail) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      Symbolic::StringValuation stringEnv = {"Alice"};
      std::vector<Symbolic::StringConstraint> stringConstraints = {Symbolic::SCMaker(0) != "Alice"};
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      BOOST_TEST(!Symbolic::eval(stringConstraints, stringEnv, numberConstraints, numEnv));
    }

    BOOST_AUTO_TEST_CASE(stringConstantEqFail) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      Symbolic::StringValuation stringEnv = {"Alice"};
      std::vector<Symbolic::StringConstraint> stringConstraints = {Symbolic::SCMaker(0) == "Bob"};
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      BOOST_TEST(!Symbolic::eval(stringConstraints, stringEnv, numberConstraints, numEnv));
    }

    BOOST_AUTO_TEST_CASE(stringConstantNePass) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      Symbolic::StringValuation stringEnv = {"Alice"};
      std::vector<Symbolic::StringConstraint> stringConstraints = {Symbolic::SCMaker(0) != "Bob"};
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      BOOST_TEST(Symbolic::eval(stringConstraints, stringEnv, numberConstraints, numEnv));
    }

    BOOST_AUTO_TEST_CASE(stringVectorEqPass) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      Symbolic::StringValuation stringEnv = {std::vector<std::string>{"Alice"}};
      std::vector<Symbolic::StringConstraint> stringConstraints = {Symbolic::SCMaker(0) == "Bob"};
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      BOOST_TEST(Symbolic::eval(stringConstraints, stringEnv, numberConstraints, numEnv));
    }

    BOOST_AUTO_TEST_CASE(stringVectorNeNonIncludePass) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      Symbolic::StringValuation stringEnv = {std::vector<std::string>{"Alice"}};
      std::vector<Symbolic::StringConstraint> stringConstraints = {Symbolic::SCMaker(0) != "Bob"};
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      BOOST_TEST(Symbolic::eval(stringConstraints, stringEnv, numberConstraints, numEnv));
    }

    BOOST_AUTO_TEST_CASE(stringVectorEqFail) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      Symbolic::StringValuation stringEnv = {std::vector<std::string>{"Alice"}};
      std::vector<Symbolic::StringConstraint> stringConstraints = {Symbolic::SCMaker(0) == "Alice"};
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      BOOST_TEST(!Symbolic::eval(stringConstraints, stringEnv, numberConstraints, numEnv));
    }

    BOOST_AUTO_TEST_CASE(stringVectorNeIncludePass) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      Symbolic::StringValuation stringEnv = {std::vector<std::string>{"Alice"}};
      std::vector<Symbolic::StringConstraint> stringConstraints = {Symbolic::SCMaker(0) != "Alice"};
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      BOOST_TEST(Symbolic::eval(stringConstraints, stringEnv, numberConstraints, numEnv));
    }

  BOOST_AUTO_TEST_SUITE_END() // EvalTest
  BOOST_AUTO_TEST_SUITE(UpdateTest)

    BOOST_AUTO_TEST_CASE(substitution) {
      using namespace Parma_Polyhedra_Library::IO_Operators;
      using namespace Parma_Polyhedra_Library;
      Symbolic::Update update;
      update.stringUpdate.clear();
      update.numberUpdate.resize(1);
      Parma_Polyhedra_Library::Variable x(0), y(1);
      update.numberUpdate[0] = {1, Parma_Polyhedra_Library::Linear_Expression(x)};
      Parma_Polyhedra_Library::Constraint_System cs;
      cs.insert(0 < x);
      cs.insert(x < 1);
      cs.insert(1 < y);
      cs.insert(y < 2);
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv(cs);
      Symbolic::StringValuation stringEnv;
      update.execute(stringEnv, numEnv);

      Parma_Polyhedra_Library::Constraint_System expectedCs;
      expectedCs.insert(0 < x);
      expectedCs.insert(x < 1);
      expectedCs.insert(x == y);
      Parma_Polyhedra_Library::NNC_Polyhedron expectedNumEnv(expectedCs);
      BOOST_TEST((numEnv == expectedNumEnv));
    }


    BOOST_AUTO_TEST_CASE(incrementByConstant) {
      using namespace Parma_Polyhedra_Library::IO_Operators;
      using namespace Parma_Polyhedra_Library;
      Symbolic::Update update;
      update.stringUpdate.clear();
      update.numberUpdate.resize(1);
      Parma_Polyhedra_Library::Variable x(0);
      update.numberUpdate[0] = {0, Parma_Polyhedra_Library::Linear_Expression(x) + 1};
      Parma_Polyhedra_Library::Constraint_System cs;
      cs.insert(0 < x);
      cs.insert(x < 1);
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv(cs);
      Symbolic::StringValuation stringEnv;
      update.execute(stringEnv, numEnv);

      Parma_Polyhedra_Library::Constraint_System expectedCs;
      expectedCs.insert(1 < x);
      expectedCs.insert(x < 2);
      Parma_Polyhedra_Library::NNC_Polyhedron expectedNumEnv(expectedCs);
      BOOST_TEST((numEnv == expectedNumEnv));
    }

    BOOST_AUTO_TEST_CASE(incrementByVariable) {
      using namespace Parma_Polyhedra_Library::IO_Operators;
      using namespace Parma_Polyhedra_Library;
      Symbolic::Update update;
      update.stringUpdate.clear();
      update.numberUpdate.resize(1);
      Parma_Polyhedra_Library::Variable x(0), y(1);
      update.numberUpdate[0] = {0, x + y};
      Parma_Polyhedra_Library::Constraint_System cs;
      cs.insert(0 < x);
      cs.insert(x < 1);
      cs.insert(1 < y);
      cs.insert(y < 2);
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv(cs);
      Symbolic::StringValuation stringEnv;
      update.execute(stringEnv, numEnv);

      Parma_Polyhedra_Library::Constraint_System expectedCs;
      expectedCs.insert(1 < y);
      expectedCs.insert(y < 2);
      expectedCs.insert(y < x);
      expectedCs.insert(x < 3);
      expectedCs.insert(x < 1 + y);
      Parma_Polyhedra_Library::NNC_Polyhedron expectedNumEnv(expectedCs);
      // std::cout << numEnv << std::endl << expectedNumEnv << std::endl;
      BOOST_TEST((numEnv == expectedNumEnv));
    }

    BOOST_AUTO_TEST_CASE(stringExecute) {
      std::vector<std::pair<VariableID, Symbolic::StringAtom>> stringUpdate = {
        {0, Symbolic::StringAtom{VariableID{1}}},
        {1, Symbolic::StringAtom{std::string{"Charlie"}}},

        {0, Symbolic::StringAtom{VariableID{1}}}
      };
      Symbolic::StringValuation stringEnv = {std::string("Alice"), std::string("Bob")};
      std::vector<std::pair<VariableID, Symbolic::NumberExpression>> numUpdate;
      Symbolic::NumberValuation numEnv;
      Symbolic::Update update = {stringUpdate, numUpdate};
      update.execute(stringEnv, numEnv);
      BOOST_CHECK_EQUAL(std::get<std::string>(stringEnv[0]), "Charlie");
      BOOST_CHECK_EQUAL(std::get<std::string>(stringEnv[1]), "Charlie");
    }

    BOOST_AUTO_TEST_CASE(stringExecute2) {
      Parma_Polyhedra_Library::NNC_Polyhedron numEnv;
      std::vector<Symbolic::NumberConstraint> numberConstraints;
      std::vector<std::pair<VariableID, Symbolic::NumberExpression>> numUpdate;

      Symbolic::StringValuation stringEnv = {std::vector<std::string>{}, std::vector<std::string>{}};
      std::vector<Symbolic::StringConstraint> stringConstraints1 = {
        Symbolic::SCMaker(0) != "Alice",
        Symbolic::SCMaker(0) != "Bob",
        Symbolic::SCMaker(0) != "Charlie",
        Symbolic::SCMaker(1) != "Alice",
        Symbolic::SCMaker(1) != "Bob"
      };
      BOOST_TEST(Symbolic::eval(stringConstraints1, stringEnv, numberConstraints, numEnv));
    
      std::vector<std::pair<VariableID, Symbolic::StringAtom>> stringUpdate = {
        {1, Symbolic::StringAtom{VariableID{0}}},
        {0, Symbolic::StringAtom{std::string{"Charlie"}}},
      };
      Symbolic::Update update = {stringUpdate, numUpdate};
      update.execute(stringEnv, numEnv);
      // After the update, stringEnv[0] is "Charlie", and stringEnv[1] is disabled for "Alice", "Bob", "Charlie"
      BOOST_CHECK_EQUAL(std::get<std::string>(stringEnv[0]), "Charlie");
      BOOST_CHECK_EQUAL(std::get<std::vector<std::string>>(stringEnv[1]).size(), 3);

      std::vector<Symbolic::StringConstraint> stringConstraints2 = {Symbolic::SCMaker(1) == "Charlie"};
      BOOST_TEST(!Symbolic::eval(stringConstraints2, stringEnv, numberConstraints, numEnv));
    
      std::vector<Symbolic::StringConstraint> stringConstraints3 = {
        Symbolic::SCMaker(0) != "Alice",
        Symbolic::SCMaker(0) != "Bob",
        Symbolic::SCMaker(0) == "Charlie",
        Symbolic::SCMaker(0) != "David"
      };
      BOOST_TEST(Symbolic::eval(stringConstraints3, stringEnv, numberConstraints, numEnv));

      std::vector<Symbolic::StringConstraint> stringConstraints4 = {Symbolic::SCMaker(1) == "David"};
      // stringEnv[1] is assumed to be "David"
      BOOST_TEST(Symbolic::eval(stringConstraints4, stringEnv, numberConstraints, numEnv));

      std::vector<Symbolic::StringConstraint> stringConstraints5 = {Symbolic::SCMaker(1) == "Eve"};
      // stringEnv[1] is assumed to be "David" above, so it cannot be "Eve"
      BOOST_TEST(!Symbolic::eval(stringConstraints5, stringEnv, numberConstraints, numEnv));
    
    }
  BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
