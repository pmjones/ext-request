--TEST--
ServerResponse::addHeaderCallback
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
$func = function($response) {};
$response = new ServerResponse();
var_dump(count($response->getHeaderCallbacks()));
var_dump(array() === $response->getHeaderCallbacks());

$response->addHeaderCallback($func);
var_dump(count($response->getHeaderCallbacks()));
var_dump(array($func) === $response->getHeaderCallbacks());

$response->addHeaderCallback($func);
var_dump(count($response->getHeaderCallbacks()));
var_dump(array($func, $func) === $response->getHeaderCallbacks());
--EXPECT--
int(0)
bool(true)
int(1)
bool(true)
int(2)
bool(true)
