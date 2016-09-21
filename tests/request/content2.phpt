--TEST--
StdRequest::$content (cli)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new StdRequest();
var_dump($request->content);
--EXPECT--
NULL
