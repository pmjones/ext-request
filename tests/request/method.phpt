--TEST--
PhpRequest::$method
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
unset($_SERVER['REQUEST_METHOD']); // not sure why this was set here
$_SERVER['HTTP_HOST'] = 'example.com';

$request = new PhpRequest();
var_dump($request->method, $request->xhr);

$_SERVER['REQUEST_METHOD'] = 'POST';
$request = new PhpRequest();
var_dump($request->method, $request->xhr);

$_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE'] = 'PATCH';
$request = new PhpRequest();
var_dump($request->method, $request->xhr);
--EXPECT--
string(0) ""
bool(false)
string(4) "POST"
bool(false)
string(5) "PATCH"
bool(true)
