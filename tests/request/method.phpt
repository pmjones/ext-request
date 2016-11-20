--TEST--
ServerRequest::$method
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
unset($_SERVER['REQUEST_METHOD']); // not sure why this was set here
$_SERVER['HTTP_HOST'] = 'example.com';

$request = new ServerRequest();
var_dump($request->method);

$_SERVER['REQUEST_METHOD'] = 'GET';
$request = new ServerRequest();
var_dump($request->method);

$_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE'] = 'PATCH';
$request = new ServerRequest();
var_dump($request->method);

$_SERVER['REQUEST_METHOD'] = 'POST';
$request = new ServerRequest();
var_dump($request->method);
--EXPECT--
string(0) ""
string(3) "GET"
string(3) "GET"
string(5) "PATCH"
