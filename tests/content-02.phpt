--TEST--
PhpRequest::$content (cli)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new PhpRequest();
var_dump($request->content);
--EXPECT--
NULL