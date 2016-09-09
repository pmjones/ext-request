--TEST--
PhpResponse::getStatus
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new PhpResponse();
var_dump($response->getStatus());
--EXPECT--
int(200)
