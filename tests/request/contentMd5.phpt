--TEST--
SapiRequest::$contentMd5
--FILE--
<?php
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTP_CONTENT_MD5' => 'foobar',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->contentMd5);

unset($_SERVER['HTTP_CONTENT_MD5']);
$request = new SapiRequest($GLOBALS);
var_dump($request->contentMd5);
--EXPECT--
string(6) "foobar"
NULL
