--TEST--
ServerRequest::$env
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--ENV--
foo=bar
baz=bat
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$_ENV = array(
    'foo' => 'bar'
);
$request = new ServerRequest();
var_dump($request->env['foo']);
--EXPECT--
string(3) "bar"
