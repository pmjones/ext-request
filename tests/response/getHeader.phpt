--TEST--
SapiResponse::getHeader
--FILE--
<?php
$response = new SapiResponse();
$response->setHeader('Foo-Bar', 'baz');
var_dump($response->getHeader('Foo-Bar'));
var_dump($response->getHeader('foo-bar'));
var_dump($response->getHeader('dib-zim'));
--EXPECT--
string(3) "baz"
string(3) "baz"
NULL
