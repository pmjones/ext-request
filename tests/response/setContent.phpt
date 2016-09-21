--TEST--
StdResponse::setContent
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new StdResponse();
$response->setContent('foo');
var_dump($response->getContent());
--EXPECT--
string(3) "foo"
