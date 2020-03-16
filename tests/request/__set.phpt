--TEST--
SapiRequest::__set
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new SapiRequest($GLOBALS);
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
string(34) "SapiRequest::$method is read-only."
string(16) "RuntimeException"
string(44) "SapiRequest::$noSuchProperty does not exist."
