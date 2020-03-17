--TEST--
SapiRequest::__get extension indirect modification
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new SapiRequest($GLOBALS);
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
string(16) "RuntimeException"
string(34) "SapiRequest::$accept is read-only."
string(16) "RuntimeException"
string(34) "SapiRequest::$method is read-only."
