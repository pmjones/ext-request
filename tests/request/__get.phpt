--TEST--
StdRequest::__get
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new StdRequest();
var_dump(isset($request->method));
try {
    $request->noSuchProperty;
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
try {
    $request->accept[0] = array();
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
function mut(&$method) {
    $method = 'DELETE';
}
try {
    mut($request->method);
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
--EXPECT--
bool(true)
string(16) "RuntimeException"
string(43) "StdRequest::$noSuchProperty does not exist."
string(16) "RuntimeException"
string(33) "StdRequest::$accept is read-only."
string(16) "RuntimeException"
string(33) "StdRequest::$method is read-only."
