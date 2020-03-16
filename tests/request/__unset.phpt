--TEST--
SapiRequest::__unset
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new SapiRequest($GLOBALS);
unset($request->no_such_prop);
try {
    unset($request->method);
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
--EXPECT--
string(16) "RuntimeException"
string(34) "SapiRequest::$method is read-only."
