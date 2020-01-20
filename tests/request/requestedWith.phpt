--TEST--
ServerRequest::$xhr
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';

$request = new ServerRequest($GLOBALS);
var_dump($request->requestedWith);

$_SERVER['HTTP_X_REQUESTED_WITH'] = 'xmlHttpRequest';
$request = new ServerRequest($GLOBALS);
var_dump($request->requestedWith);
--EXPECT--
NULL
string(14) "xmlHttpRequest"
