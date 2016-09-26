--TEST--
StdRequest::__set
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new StdRequest();
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
string(33) "StdRequest::$method is read-only."
string(16) "RuntimeException"
string(41) "StdRequest::$noSuchProperty is read-only."
