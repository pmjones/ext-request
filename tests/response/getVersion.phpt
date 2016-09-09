--TEST--
PhpResponse::getVersion
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new PhpResponse();
var_dump($response->getVersion());
--EXPECT--
string(3) "1.1"