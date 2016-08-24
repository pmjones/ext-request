#!/bin/sh

set -ex

php -d extension=modules/request.so `which phpunit` refimpl
