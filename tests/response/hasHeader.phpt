--TEST--
ServerResponse::hasHeader
--FILE--
<?php
$response = new ServerResponse();
$response->setHeader('Foo-Bar', 'baz');
var_dump($response->hasHeader('Foo-Bar'));
var_dump($response->hasHeader('foo-bar'));
var_dump($response->hasHeader('dib-zim'));
--EXPECT--
bool(true)
bool(true)
bool(false)
