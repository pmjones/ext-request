--TEST--
PhpResponse::setContent
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new PhpResponse();
$response->setContent('foo');
var_dump($response->getContent());
--EXPECT--
string(3) "foo"
