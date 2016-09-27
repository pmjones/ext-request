#!/bin/sh

set -ex

export NO_INTERACTION=1
export REPORT_EXIT_STATUS=1
export TEST_PHP_EXECUTABLE=`which php`
export PHPUNIT=`which phpunit`

case $1 in
coverage)
    phpize
    ./configure CFLAGS="--coverage -fprofile-arcs -ftest-coverage" LDFLAGS="--coverage"
    make clean all
    lcov --directory . --zerocounters
    lcov --directory . --capture --compat-libtool --initial --output-file coverage.info
    $TEST_PHP_EXECUTABLE -d extension=modules/request.so $PHPUNIT refimpl
    $TEST_PHP_EXECUTABLE run-tests.php -d extension=request.so -d extension_dir=modules -n ./tests/
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
refimpl)
    phpize
    ./configure
    make clean all
    $TEST_PHP_EXECUTABLE -d extension=modules/request.so $PHPUNIT refimpl
    ;;
*)
    $TEST_PHP_EXECUTABLE -d extension=modules/request.so $PHPUNIT refimpl $@
    ;;
esac
