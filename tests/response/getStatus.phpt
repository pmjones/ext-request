--TEST--
StdResponse::getStatus
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new StdResponse();
var_dump($response->getStatus());
--EXPECT--
int(200)
