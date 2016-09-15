--TEST--
PhpResponse::sendContent (string)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new PhpResponse();
$response->setContent('foo');
$response->send();
--EXPECT--
foo