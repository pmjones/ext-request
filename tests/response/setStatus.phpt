--TEST--
StdResponse::setStatus
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new StdResponse();
$response->setStatus('500');
var_dump($response->getStatus());
$response->setStatus(401);
var_dump($response->getStatus());
--EXPECT--
int(500)
int(401)
