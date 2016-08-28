--TEST--
PhpRequest::__set
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new PhpRequest();
try {
    $request->method = 'PATCH';
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
try {
    $request->noSuchProperty = 'foo';
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
--EXPECT--
string(16) "RuntimeException"
string(24) "PhpRequest is read-only."
string(16) "RuntimeException"
string(24) "PhpRequest is read-only."