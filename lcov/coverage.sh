#!/bin/sh

#
# Shell script to build HTML coverage information overview by using lcov.
#
# Supposed to run under Linux.
# We need gcc >=3.4 for gcov and lcov to work properly.
#
# Parameters
# $1 Name of the test, e.g. netlib (needs netlib.test, netlib.solu)

if (test $# != 1)                                              
then 
  echo "usage: coverage.sh <test_suite>"
  exit
fi

TEST_SUITE=$1

# binaries & paths
LCOV=lcov
GENHTML=genhtml

OBJ_DIR=obj/O.linux.x86.gnu.gcov.static/lib
SRC_DIR=src
CHECK_DIR=check

#--------------------------------------------------------------------------
# end setup
#--------------------------------------------------------------------------

# Check that both lcov and genhtml are available
DUMMY=`which $LCOV`

if (test -z $DUMMY)
then
  echo "program lcov ($LCOV) not found; please install it"
  exit 1
fi

DUMMY=`which $GENHTML`

if (test -z $DUMMY)
then
  echo "program genhtml ($GENHTML) not found; please install it"
  exit 1
fi

# Determine base directory
BASE_DIR=`cd ..; pwd`

# Check existence of test suite before doing anything.
if !(test -f $BASE_DIR/$CHECK_DIR/$TEST_SUITE.test)
then
  echo "$TEST_SUITE is not a valid (check) test suite"
  exit 1
fi

# Check existence of a .gcno file to ensure proper gcov (version >=3.4).
if !(test -f $BASE_DIR/$OBJ_DIR/spxsolve.gcno)
then
  echo "object files in $OBJ_DIR seem not to be compiled with gcc >= 3.4 and gcov activated (no .gcno files)"
  exit 1
fi

# We need to ensure that there is a link to "src" in $OBJ_DIR, otherwise
# gcov and lcov will get confused.
if !(test -L $BASE_DIR/$OBJ_DIR/src)
then
  ln --symbolic $BASE_DIR/$SRC_DIR $BASE_DIR/$OBJ_DIR/src
fi

# Reset counters, ie remove data files
#
# This command is supposed to do the job, but only removes .da files while
# GCC >3.4 uses .gcda files
# $LCOV --zerocounters --directory $OBJ_DIR
echo Removing .gcda files
rm -f $OBJ_DIR/*.gcda

# Create coverage data by running the test suite.
cd ..
make COMP=gnu OPT=gcov TEST=$TEST_SUITE ALGO="1 2 3 4 5 6 7 8 9 10 11 12 13 14 15" check
cd lcov

# Build HTML files.
$LCOV --capture -directory $BASE_DIR/$OBJ_DIR --output-file $TEST_SUITE.info --test-name $TEST_SUITE
$GENHTML $TEST_SUITE.info --output-directory $TEST_SUITE --title "$TEST_SUITE LP suite" --show-details 