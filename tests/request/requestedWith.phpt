--TEST--
ServerRequest::$xhr
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
