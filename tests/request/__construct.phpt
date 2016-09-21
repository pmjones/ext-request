--TEST--
StdRequest::__construct
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new StdRequest();
var_dump(get_class($request));
--EXPECT--
string(10) "StdRequest"
