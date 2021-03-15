#!/usr/bin/env bash

set -e -o pipefail

# config
export PHP_VERSION=${PHP_VERSION:-"7.4"}
export COVERAGE=${COVERAGE:-false}
export DEBIAN_FRONTEND=noninteractive
export SUDO=sudo

function install_apt_packages() (
    set -x
    ${SUDO} add-apt-repository ppa:ondrej/php
    ${SUDO} apt-get update
    ${SUDO} apt-get install -y composer jq lcov \
        php${PHP_VERSION}-dev \
        php${PHP_VERSION}-cgi \
        php${PHP_VERSION}-bcmath \
        php${PHP_VERSION}-curl \
        php${PHP_VERSION}-gd \
        php${PHP_VERSION}-mbstring \
        php${PHP_VERSION}-xml \
        php${PHP_VERSION}-zip
    ${SUDO} update-alternatives --set php /usr/bin/php${PHP_VERSION}
    ${SUDO} update-alternatives --set php-cgi /usr/bin/php-cgi${PHP_VERSION}
    ${SUDO} update-alternatives --set php-config /usr/bin/php-config${PHP_VERSION}
    ${SUDO} update-alternatives --set phpize /usr/bin/phpize${PHP_VERSION}
    # ${SUDO} update-alternatives --set phpdbg /usr/bin/phpdbg${PHP_VERSION}
    ${SUDO} update-alternatives --config php
    ${SUDO} update-alternatives --config php-cgi
)

install_apt_packages

# source and execute script used in travis
if [[ "${COVERAGE}" = "true" ]]; then
    ./run-tests.sh coverage
else
    ./run-tests.sh normal
fi
