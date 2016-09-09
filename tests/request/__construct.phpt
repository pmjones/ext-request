--TEST--
PhpRequest::__construct
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new PhpRequest();
var_dump(get_class($request));
--EXPECT--
string(10) "PhpRequest"