--TEST--
SapiRequest::__get
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new SapiRequest($GLOBALS);
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
string(44) "SapiRequest::$noSuchProperty does not exist."
string(16) "RuntimeException"
string(34) "SapiRequest::$accept is read-only."
string(16) "RuntimeException"
string(34) "SapiRequest::$method is read-only."
