--TEST--
ServerResponse::getHeaderCallback
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
$response->setHeaderCallback($func);
var_dump($func === $response->getHeaderCallback());
--EXPECT--
bool(true)
