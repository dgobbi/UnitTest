/*=========================================================================

Emulate many of the macros from UnitTest++, within a simple header file.
This header file is placed in the public domain.


Introduction
============

This header file provides macros that can be used to facilitate the creation
of unit tests for a C++ project.  It is designed to be portable.  Simply
include this header from any CPP file.

Some important differences as compared to UnitTest++ are:
1. It does not provide exception checks.
2. It does not provide time checks.
These two features could easily be added.


Core Macros
===========

Define a test block.  This causes the instantiation of a UnitTest object.

    TEST(name)
    {
      // test code
    }

Check a condition.  This can be called within a test block.  If the condition
is false, the test is marked as "failed" and diagnostic information is printed.

    CHECK(condition)

Create a main() function that will run the tests when the test executable is
run.  This macro must be called in one (and only one) of the cpp files that
are linked into the executable.  Because it generates a main() function, the
executable must have no other main() function.

    TEST_MAIN()


More Macros
===========

Check equality.  These macros check to see if their arguments are equal,
and fail if they aren't.  For the array checks, the types "a" and "b" are
indexed using the [i] operator, or [i][j] for the 2D arrays.

    CHECK_EQUAL(a, b)
    CHECK_ARRAY_EQUAL(a, b, size)
    CHECK_ARRAY2D_EQUAL(a, b, size_i, size_j)

Check if floating-point values are close to each other, that is, if their
difference is less than the given tolerance.

    CHECK_CLOSE(a, b, tolerance)
    CHECK_ARRAY_CLOSE(a, b, size, tolerance)
    CHECK_ARRAY2D_CLOSE(a, b, size_i, size_j, tolerance)

Define a suite, which provides a namespace for a group of related tests.

    SUITE(name)
    {
    // one or more TEST definitions
    }

Define a test that is a subclass of a the specified fixture class.  The
fixture class can be any simple C++ class that you define, and members of
the fixture class can be used within the test.  The same fixture class can
be used for many tests, so this is a convenient way of sharing code between
tests (just put the shared code in a method of the fixture class).

    TEST_FIXTURE(fixture, name)
    {
      // test code where "this" is an instance of "fixture".
    }


Running Tests
=============

If you run the test executable with no arguments, then it will run through
all of the tests and print pass/fail information to stdout.  If any errors
are encountered, they will be printed to stderr, and the executable will
return a nonzero falue to indicate failure.  If any tests are part of a suite,
then the name of the test will be prefixed by the suite name.

    ./TestEvents
    Events-Constructor: [Passed]
    Events-DescriptorSpecificity: [Passed]
    Events-EventMatching: [Passed]

A single test can be run by passing the name of the test to the executable.
If the test is within a suite, then the name must include the suite.  When
used this way, the name of the test is not printed.

    ./TestEvents Events-DescriptorSpecificity
    # returns 0 if success, prints error and returns 1 if failed

It is also possible to list all of the tests without running them by using
the "--list" option.

    ./TestEvents --list
    Events-Constructor
    Events-DescriptorSpecificity
    Events-EventMatching

When a test fails, the failure condition will be printed along with the
line number within the test program.

    Failed CHECK(!w.IsExpired()) TestEvents.cpp:216 [UnitTest]


Adding to CMake
===============

To use the tests from cmake, the tests must be listed in the CMakeLists.txt.
First, you have to list the names of all the tests and the executable that
runs them.  Then, you need to add a "foreach" loop that calls add_test() for
each test.  If you ever add or remove tests from the executable, then be sure
to run it on the command-line with the "--list" option to get an updated list
of the tests that it provides.

    # Modify this code as necessary for your tests.
    set(TESTS
      test1
      test2
      test3
    )
    set(TEST_EXE Tests)
    set(TEST_SRCS Test.cxx)
    set(TEST_LIBS )

    # This block creates the tests, leave it as-is.
    add_executable(${TEST_EXE} ${TEST_SRCS})
    target_link_libraries(${TEST_EXE} ${TEST_LIBS})
    foreach(TEST_NAME ${TESTS})
      add_test(NAME ${TEST_NAME} COMMAND ${TEST_EXE} ${TEST_NAME})
    endforeach()

=========================================================================*/

#ifndef UNITTEST_H
#define UNITTEST_H

#include <math.h>
#include <string.h>
#include <vector>
#include <iostream>

//! The base class for unit tests.
class UnitTest
{
public:
  //! Virtual destructor.
  virtual ~UnitTest() {}

  //! A method to get the name of the test.
  const char *GetTestName();

  //! A method to get the suite the test belongs to.
  const char *GetSuiteName();

  //! A static method to run one test by name.
  static int RunTest(const char *name);

  //! A static method to run all the unit tests.
  static int RunAllTests();

  //! A static method to print all test names to stdout.
  static void ListAllTests();

protected:
  //! Create a unit test and register it with the test driver.
  UnitTest(const char *suite, const char *name);

  //! This method is overridden to run the test.
  virtual void operator() () = 0;

  //! A list of all registered tests.
  static std::vector<UnitTest *> *Tests;

  //! A boolean that is set if any test fails.
  static bool TestFailed;

private:
  const char *UnitTestSuite;
  const char *UnitTestName;

  friend class UnitTestInitializer;
};

//! An internal class that initializes the list of unit tests.
static class UnitTestInitializer
{
public:
  UnitTestInitializer();
  ~UnitTestInitializer();
} unitTestSchwarzCounter;

// Constructor adds the test to the list of tests.
inline UnitTest::UnitTest(const char *suite, const char *name)
  : UnitTestSuite(suite), UnitTestName(name)
{
  UnitTest::Tests->push_back(this);
}

// Get the name of the suite.
inline const char *UnitTest::GetSuiteName()
{
  return UnitTestSuite;
}

// Get the name of the test.
inline const char *UnitTest::GetTestName()
{
  return UnitTestName;
}

// Run one of the tests by name.
inline int UnitTest::RunTest(const char *test)
{
  // Look for a hyphen, denoting suite-test.
  size_t suiteLen = 0;
  const char *suite = "";
  const char *cp = test;
  while (*cp != '\0' && *cp != '-') { cp++; }
  if (*cp == '-')
  {
    suite = test;
    suiteLen = cp - test;
  }
  // The 'stest' is the remainder, after the hyphen.
  const char *stest = test + suiteLen + (suiteLen > 0 ? 1 : 0);
  UnitTest::TestFailed = false;
  for (size_t i = 0; i < UnitTest::Tests->size(); i++)
  {
    UnitTest *t = UnitTest::Tests->at(i);
    // First check that the suite name matches.
    const char *tsuite = t->GetSuiteName();
    if ((suiteLen == 0 && *tsuite == '\0') ||
        (suiteLen > 0 && strncmp(tsuite, suite, suiteLen) == 0 &&
         tsuite[suiteLen] == '\0'))
    {
      // Check that the test name maches.
      if (strcmp(t->GetTestName(), stest) == 0)
      {
        (*t)();
        return UnitTest::TestFailed;
      }
    }
  }
  std::cerr << "Unknown test \"" << test << "\" for file " << __FILE__ << "\n";
  return 1;
}

// Run all of the tests in the list.
inline int UnitTest::RunAllTests()
{
  bool anyFailed = false;
  for (size_t i = 0; i < UnitTest::Tests->size(); i++)
  {
    UnitTest::TestFailed = false;
    UnitTest *t = UnitTest::Tests->at(i);
    const char *suite = t->GetSuiteName();
    const char *name = t->GetTestName();
    std::cout << suite << (suite[0] == 0 ? "" : "-") << name << ": ";
    std::cout.flush();
    (*t)();
    std::cout << (UnitTest::TestFailed ? "[Failed]" : "[Passed]")
              << std::endl;
    anyFailed |= UnitTest::TestFailed;
  }
  UnitTest::TestFailed = anyFailed;
  return UnitTest::TestFailed;
}

// List the tests to stdout.
inline void UnitTest::ListAllTests()
{
  for (size_t i = 0; i < UnitTest::Tests->size(); i++)
  {
    UnitTest *t = UnitTest::Tests->at(i);
    const char *suite = t->GetSuiteName();
    const char *name = t->GetTestName();
    std::cout << suite << (suite[0] == 0 ? "" : "-") << name << "\n";
  }
}

namespace SuiteNamespace{
// The default suite prefix is empty.
inline const char *GetSuiteName() { return ""; }
}

#define CHECK_WITH_MESSAGE(t, m) \
if (!(t)) \
{ \
  std::cerr << "Failed " << m << " " \
            << __FILE__ << ":" << __LINE__ << " [UnitTest]\n"; \
  std::cerr.flush(); \
  UnitTest::TestFailed = true; \
}

//! A macro that checks a boolean, the test fails if value is false.
#define CHECK(t) \
CHECK_WITH_MESSAGE(t, "CHECK(" #t ")")

//! A macro that causes the test to fail unless the values are equal.
#define CHECK_EQUAL(expected, actual) \
CHECK_WITH_MESSAGE((expected) == (actual), \
  "CHECK_EQUAL(" #expected ", " #actual ")")

//! A macro that causes the test to fail unless the arrays are equal.
#define CHECK_ARRAY_EQUAL(x, y, size) \
{ \
  size_t array_size = (size); \
  bool equal_check = true; \
  for (size_t array_index = 0; array_index < array_size; array_index++) \
  { \
    equal_check &= ((x)[array_index] == (y)[array_index]); \
  } \
  CHECK_WITH_MESSAGE(equal_check, \
    "CHECK_ARRAY_EQUAL(" #x ", " #y ", " #size ")") \
}

//! A macro that causes the test to fail unless the arrays are equal.
#define CHECK_ARRAY2D_EQUAL(x, y, sizex, sizey) \
{ \
  size_t array_sizex = (sizex); \
  size_t array_sizey = (sizey); \
  bool equal_check = true; \
  for (size_t array_idx = 0; array_idx < array_sizex; array_idx++) \
  { \
    for (size_t array_idy = 0; array_idy < array_sizey; array_idy++) \
    { \
      equal_check &= ((x)[array_idx][array_idy] == \
        (y)[array_idx][array_idy]); \
    } \
  } \
  CHECK_WITH_MESSAGE(equal_check, \
    "CHECK_ARRAY2D_EQUAL(" #x ", " #y ", " #sizex ", " #sizey ")") \
}

//! A macro that causes the test to fail unless the values are close.
#define CHECK_CLOSE(x, y, tol) \
CHECK_WITH_MESSAGE(fabs((x) - (y)) < (tol), \
  "CHECK_CLOSE(" #x ", " #y ", " #tol ")")

//! A macro that causes the test to fail unless the arrays are close.
#define CHECK_ARRAY_CLOSE(x, y, size, tol) \
{ \
  double check_tolerance = (tol); \
  size_t array_size = (size); \
  bool equal_check = true; \
  for (size_t array_index = 0; array_index < array_size; array_index++) \
  { \
    equal_check &= (fabs((x)[array_index] - \
      (y)[array_index]) < check_tolerance); \
  } \
  CHECK_WITH_MESSAGE(equal_check, \
    "CHECK_ARRAY_CLOSE(" #x ", " #y ", " #size ", " #tol ")") \
}

//! A macro that causes the test to fail unless the arrays are close.
#define CHECK_ARRAY2D_CLOSE(x, y, sizex, sizey, tol) \
{ \
  double check_tolerance = (tol); \
  size_t array_sizex = (sizex); \
  size_t array_sizey = (sizey); \
  bool equal_check = true; \
  for (size_t array_idx = 0; array_idx < array_sizex; array_idx++) \
  { \
    for (size_t array_idy = 0; array_idy < array_sizey; array_idy++) \
    { \
      equal_check &= (fabs((x)[array_idx][array_idy] - \
            (y)[array_idx][array_idy]) < check_tolerance); \
    } \
  } \
  CHECK_WITH_MESSAGE(equal_check, \
    "CHECK_ARRAY2D_CLOSE(" #x ", " #y ", " #sizex ", " #sizey ", " #tol ")") \
}

//! Use this macro to begin a test suite.
#define SUITE(name) \
namespace name { \
namespace SuiteNamespace { \
inline const char *GetSuiteName() { return #name; } \
} \
} \
namespace name

//! Use this macro to begin a unit test.
#define TEST(name) \
class UnitTest_##name : UnitTest \
{ \
public: \
  UnitTest_##name() : UnitTest(SuiteNamespace::GetSuiteName(), #name) {} \
protected: \
  void operator() (); \
} UnitTest_##name##_Instance; \
void UnitTest_##name::operator() ()

//! Create a test with "fixture" as its base class.
#define TEST_FIXTURE(fixture, name) \
class UnitTest_##name : UnitTest, fixture \
{ \
public: \
  UnitTest_##name() : UnitTest(SuiteNamespace::GetSuiteName(), #name) {} \
protected: \
  void operator() (); \
} UnitTest_##name##_Instance; \
void UnitTest_##name::operator() ()

//! Call this macro to auto-generate a main() function.
#define TEST_MAIN() \
std::vector<UnitTest *> *UnitTest::Tests; \
bool UnitTest::TestFailed; \
static size_t schwarzCounter = 0; \
UnitTestInitializer::UnitTestInitializer() \
{ \
  if (schwarzCounter++ == 0) \
  { \
    UnitTest::Tests = new std::vector<UnitTest *>; \
  } \
} \
UnitTestInitializer::~UnitTestInitializer() \
{ \
  if (--schwarzCounter == 0) \
  { \
    delete UnitTest::Tests; \
  } \
} \
int main(int argc, char *argv[]) \
{ \
  if (argc > 1) \
  { \
    if (argc > 2) \
    { \
      std::cerr << "Too many arguments to test program " << argv[0] << "\n"; \
      return 1; \
    } \
    if (argv[1][0] == '-') \
    { \
      if (strcmp("--list", argv[1]) == 0) \
      { \
        UnitTest::ListAllTests(); \
        return 0; \
      } \
      else \
      { \
        std::cerr << "Unrecognized option \"" << argv[1] \
                  << "\" for test program " << argv[0] << "\n"; \
        return 1; \
      } \
    } \
    return UnitTest::RunTest(argv[1]); \
  } \
  return UnitTest::RunAllTests(); \
}

#endif /* UNITTEST_H */
