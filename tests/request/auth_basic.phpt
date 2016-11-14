--TEST--
ServerRequest - basic auth
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER += [
    'HTTP_HOST' => 'example.com',
    'PHP_AUTH_TYPE' => 'Basic',
    'PHP_AUTH_USER' => 'foo',
    'PHP_AUTH_PW' => 'bar'
];
$request = new ServerRequest();
var_dump($request->authType);
var_dump($request->authUser);
var_dump($request->authPw);
--EXPECT--
string(5) "Basic"
string(3) "foo"
string(3) "bar"
