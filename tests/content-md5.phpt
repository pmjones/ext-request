--TEST--
PhpRequest::$contentMd5
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTP_CONTENT_MD5' => 'foobar',
];
$request = new PhpRequest();
var_dump($request->contentMd5);

unset($_SERVER['HTTP_CONTENT_MD5']);
$request = new PhpRequest();
var_dump($request->contentMd5);
--EXPECT--
string(6) "foobar"
NULL
