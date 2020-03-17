--TEST--
SapiRequest::$cookie
--COOKIE--
foo=bar; baz=bat
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$request = new SapiRequest($GLOBALS);
var_dump($request->cookie['foo']);
var_dump($request->cookie['baz']);
--EXPECT--
string(3) "bar"
string(3) "bat"
