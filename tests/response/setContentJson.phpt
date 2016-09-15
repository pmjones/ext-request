--TEST--
PhpResponse::setContentJson
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--EXTENSIONS--
json
--FILE--
<?php
$response = new PhpResponse();
$response->setContentJson(array('foo' => 'bar'));
var_dump($response->getContent());
var_dump($response->getHeaders()['Content-Type'][0]);
--EXPECT--
string(13) "{"foo":"bar"}"
string(16) "application/json"
