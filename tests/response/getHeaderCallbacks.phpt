--TEST--
ServerResponse::getHeaderCallbacks
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$func = function($response) {};
$response->setHeaderCallbacks(array($func, $func));
var_dump(array($func, $func) === $response->getHeaderCallbacks());
$response->setHeaderCallbacks(array($func));
var_dump(array($func) === $response->getHeaderCallbacks());
--EXPECT--
bool(true)
bool(true)
