--TEST--
StdResponse::setVersion
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new StdResponse();
$response->setVersion('1.0');
var_dump($response->getVersion());
--EXPECT--
string(3) "1.0"