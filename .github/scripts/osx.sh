#!/usr/bin/env bash

set -e -o pipefail

# config
export PHP_VERSION=${PHP_VERSION:-"7.4"}
export NO_INTERACTION=1
export REPORT_EXIT_STATUS=1
export TEST_PHP_EXECUTABLE=${TEST_PHP_EXECUTABLE:-`which php`}

function install_brew_packages() (
    set -x
    brew install php@$PHP_VERSION
)

function install() (
    set -x
    phpize
    ./configure
    make
)

function script() (
    set -x
    make test
)

install_brew_packages
install
script
