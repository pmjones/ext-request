--TEST--
PhpRequest setter
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new PhpRequest();
try {
    $request->undefinedProp = 'foo';
} catch( \RuntimeException $e ) {
    echo $e->getMessage();
}
--EXPECT--
PhpRequest is read-only.