#!/bin/sh

set -ex

case $1 in
coverage)
    export NO_INTERACTION=1
    export REPORT_EXIT_STATUS=1
    export TEST_PHP_EXECUTABLE=`which php`
    phpize
    ./configure --enable-handlebars CFLAGS="--coverage -fprofile-arcs -ftest-coverage" LDFLAGS="--coverage"
    make clean all
    lcov --directory . --zerocounters
    lcov --directory . --capture --compat-libtool --initial --output-file coverage.info
    php run-tests.php -d extension=request.so -d extension_dir=modules -n ./tests/
    lcov --no-checksum --directory . --capture --compat-libtool --output-file coverage.info
    lcov --remove coverage.info "/usr*" \
        --remove coverage.info "*/.phpenv/*" \
        --remove coverage.info "/home/travis/build/include/*" \
        --compat-libtool --output-file coverage.info
    genhtml coverage.info --output-directory reports
    ;;
valgrind)
    TEST_PHP_ARGS=-m make test
    ;;
*)
    php -d extension=modules/request.so `which phpunit` refimpl $@
    ;;
esac
