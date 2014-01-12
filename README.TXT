Emulate many of the macros from UnitTest++, within a simple header file.

Author: David Gobbi <david.gobbi@gmail.com>


Introduction
============

UnitTest.h provides macros that can be used to facilitate the creation
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
      add_test(${TEST_NAME} ${TEST_EXE} ${TEST_NAME})
    endforeach()
