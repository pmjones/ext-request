--TEST--
SapiRequest - basic auth
--FILE--
<?php
$_SERVER += [
    'HTTP_HOST' => 'example.com',
    'PHP_AUTH_TYPE' => 'Basic',
    'PHP_AUTH_USER' => 'foo',
    'PHP_AUTH_PW' => 'bar'
];
$request = new SapiRequest($GLOBALS);
var_dump($request->authType);
var_dump($request->authUser);
var_dump($request->authPw);
--EXPECT--
string(5) "Basic"
string(3) "foo"
string(3) "bar"
