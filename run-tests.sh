#!/bin/sh

set -ex

export NO_INTERACTION=1
export REPORT_EXIT_STATUS=1
export TEST_PHP_EXECUTABLE=`which php`

case $1 in
coverage)
    phpize
    ./configure CFLAGS="--coverage -fprofile-arcs -ftest-coverage" LDFLAGS="--coverage"
    make clean all
    lcov --directory . --zerocounters
    lcov --directory . --capture --compat-libtool --initial --output-file coverage.info
    $TEST_PHP_EXECUTABLE run-tests.php -d auto_prepend_file=`pwd`/userland/tests/prepend.php -n ./tests/
    $TEST_PHP_EXECUTABLE run-tests.php -d extension=modules/request.so -n ./tests/
    lcov --no-checksum --directory . --capture --compat-libtool --output-file coverage.info
    lcov --remove coverage.info "/usr*" \
        --remove coverage.info "*/.phpenv/*" \
        --remove coverage.info "/home/travis/build/include/*" \
        --compat-libtool --output-file coverage.info
    genhtml coverage.info --output-directory reports
    ;;
valgrind)
    phpize
    ./configure
    make clean all
    make test TEST_PHP_ARGS=-m
    ;;
userland)
    phpize
    ./configure
    make clean all
    $TEST_PHP_EXECUTABLE run-tests.php -d auto_prepend_file=`pwd`/userland/tests/prepend.php -n ./tests/
    ;;
*)
    $TEST_PHP_EXECUTABLE run-tests.php -d auto_prepend_file=`pwd`/userland/tests/prepend.php -n ./tests/ $@
    ;;
esac
