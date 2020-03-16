--TEST--
SapiRequest::$method
--FILE--
<?php
unset($_SERVER['REQUEST_METHOD']); // not sure why this was set here
$_SERVER['HTTP_HOST'] = 'example.com';

$request = new SapiRequest($GLOBALS);
var_dump($request->method);

$_SERVER['REQUEST_METHOD'] = 'GET';
$request = new SapiRequest($GLOBALS);
var_dump($request->method);

$_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE'] = 'PATCH';
$request = new SapiRequest($GLOBALS);
var_dump($request->method);

$_SERVER['REQUEST_METHOD'] = 'POST';
$request = new SapiRequest($GLOBALS);
var_dump($request->method);
--EXPECT--
NULL
string(3) "GET"
string(3) "GET"
string(5) "PATCH"
