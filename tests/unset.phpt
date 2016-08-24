--TEST--
PhpRequest unsetter
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$request = new PhpRequest();
try {
    unset($request->method);
} catch( \RuntimeException $e ) {
    echo $e->getMessage();
}
--EXPECT--
PhpRequest is read-only.