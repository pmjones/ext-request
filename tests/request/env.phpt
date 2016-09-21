--TEST--
StdRequest::$env
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--ENV--
foo=bar
baz=bat
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$_ENV = array(
    'foo' => 'bar'
);
$request = new StdRequest();
var_dump($request->env['foo']);
--EXPECT--
string(3) "bar"
