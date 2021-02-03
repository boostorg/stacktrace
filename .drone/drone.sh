#!/bin/bash

# Copyright 2020 Rene Rivera, Sam Darwin
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

set -e
set -x
export TRAVIS_BUILD_DIR=$(pwd)
export DRONE_BUILD_DIR=$(pwd)
export TRAVIS_BRANCH=$DRONE_BRANCH
export TRAVIS_EVENT_TYPE=$DRONE_BUILD_EVENT
export VCS_COMMIT_ID=$DRONE_COMMIT
export GIT_COMMIT=$DRONE_COMMIT
export REPO_NAME=$DRONE_REPO
export USER=$(whoami)
export CC=${CC:-gcc}
export PATH=~/.local/bin:/usr/local/bin:$PATH

if [ "$DRONE_JOB_BUILDTYPE" == "boost" ]; then

echo '==================================> INSTALL'

BOOST_BRANCH=develop && [ "$TRAVIS_BRANCH" == "master" ] && BOOST_BRANCH=master || true
IGNORE_COVERAGE='*/filesystem/src/* */thread/src/pthread/* */thread/src/future.cpp */timer/src/* operators.hpp'
BOOST_LIBS_FOLDER=$(basename $REPO_NAME)
UBSAN_OPTIONS=print_stacktrace=1
LSAN_OPTIONS=verbosity=1:log_threads=1
BOOST=$HOME/boost-local
git clone -b $BOOST_BRANCH --depth 10 https://github.com/boostorg/boost.git $BOOST
cd $BOOST
git submodule update --init --depth 10 --jobs 2 tools/build tools/boostdep tools/inspect libs/filesystem libs/interprocess
python tools/boostdep/depinst/depinst.py --git_args "--depth 10 --jobs 2" $BOOST/libs/filesystem
echo "Testing $BOOST/libs/$BOOST_LIBS_FOLDER moved from $TRAVIS_BUILD_DIR, branch $BOOST_BRANCH"
rm -rf $BOOST/libs/$BOOST_LIBS_FOLDER || true
cp -rp $TRAVIS_BUILD_DIR $BOOST/libs/$BOOST_LIBS_FOLDER
python tools/boostdep/depinst/depinst.py --git_args "--depth 10 --jobs 2" $BOOST_LIBS_FOLDER
git status
./bootstrap.sh
./b2 headers
./b2 -j4 variant=debug tools/inspect/build
echo "using gcc ;" >> ~/user-config.jam
echo "using clang ;" >> ~/user-config.jam
echo "using clang : 3.8 : clang++-3.8 ;" >> ~/user-config.jam
echo "using clang : 4 : clang++-4.0 ;" >> ~/user-config.jam
echo "using clang : 5 : clang++-5.0 ;" >> ~/user-config.jam
echo "using clang : 6 : clang++-6.0 ;" >> ~/user-config.jam
echo "using clang : 7 : clang++-7.0 ;" >> ~/user-config.jam
echo "using clang : 8 : clang++-8 ;" >> ~/user-config.jam
echo "using clang : libc++ : clang++-libc++ ;" >> ~/user-config.jam
cd $BOOST/libs/$BOOST_LIBS_FOLDER/test/

echo '==================================> SCRIPT'

sh -c "../../../b2 -j2 $B2_ARGS"
../../../dist/bin/inspect .. -license -copyright -crlf -end -path_name -tab -ascii -apple_macro -deprecated_macro -minmax -unnamed -assert_macro

echo '==================================> AFTER_SUCCESS'

mkdir -p $TRAVIS_BUILD_DIR/coverals
find ../../../bin.v2/ -name "*.gcda" -exec cp "{}" $TRAVIS_BUILD_DIR/coverals/ \;
find ../../../bin.v2/ -name "*.gcno" -exec cp "{}" $TRAVIS_BUILD_DIR/coverals/ \;
find ../../../bin.v2/ -name "*.da" -exec cp "{}" $TRAVIS_BUILD_DIR/coverals/ \;
find ../../../bin.v2/ -name "*.no" -exec cp "{}" $TRAVIS_BUILD_DIR/coverals/ \;
wget https://github.com/linux-test-project/lcov/archive/v1.14.zip
unzip v1.14.zip
LCOV="`pwd`/lcov-1.14/bin/lcov --gcov-tool $GCOVTOOL"
mkdir -p ~/.local/bin
echo -e '#!/bin/bash\nexec llvm-cov gcov "$@"' > ~/.local/bin/gcov_for_clang.sh
chmod 755 ~/.local/bin/gcov_for_clang.sh
echo "$LCOV --directory $TRAVIS_BUILD_DIR/coverals --base-directory `pwd` --capture --output-file $TRAVIS_BUILD_DIR/coverals/coverage.info"
$LCOV --directory $TRAVIS_BUILD_DIR/coverals --base-directory `pwd` --capture --output-file $TRAVIS_BUILD_DIR/coverals/coverage.info
cd $BOOST
$LCOV --remove $TRAVIS_BUILD_DIR/coverals/coverage.info "/usr*" "*/$BOOST_LIBS_FOLDER/test/*" $IGNORE_COVERAGE "*/$BOOST_LIBS_FOLDER/tests/*" "*/$BOOST_LIBS_FOLDER/examples/*" "*/$BOOST_LIBS_FOLDER/example/*" -o $TRAVIS_BUILD_DIR/coverals/coverage.info
OTHER_LIBS=`grep "submodule .*" .gitmodules | sed 's/\[submodule\ "\(.*\)"\]/"\*\/boost\/\1\.hpp" "\*\/boost\/\1\/\*" "\*\/libs\/\1\/src\/\*"/g'| sed "/\"\*\/boost\/$BOOST_LIBS_FOLDER\/\*\"/d" | sed ':a;N;$!ba;s/\n/ /g'`
echo $OTHER_LIBS
eval "$LCOV --remove $TRAVIS_BUILD_DIR/coverals/coverage.info $OTHER_LIBS -o $TRAVIS_BUILD_DIR/coverals/coverage.info"
cd $TRAVIS_BUILD_DIR
gem install coveralls-lcov || echo "ERROR. Failed to install coveralls-lcov"
coveralls-lcov coverals/coverage.info

fi
