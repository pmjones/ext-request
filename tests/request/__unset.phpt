--TEST--
ServerRequest::__unset
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new ServerRequest($GLOBALS);
try {
    unset($request->method);
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
--EXPECT--
string(16) "RuntimeException"
string(36) "ServerRequest::$method is read-only."
