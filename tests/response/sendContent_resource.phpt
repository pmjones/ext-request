--TEST--
StdResponse::sendContent (resource)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new StdResponse();
$fh = fopen('php://temp', 'w+');
fwrite($fh, 'foo');
$response->setContent($fh);
$response->send();
--EXPECT--
foo
