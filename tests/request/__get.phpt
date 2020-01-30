--TEST--
ServerRequest::__get
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new ServerRequest($GLOBALS);
var_dump(isset($request->method));
try {
    $request->noSuchProperty;
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
--EXPECT--
bool(true)
string(16) "RuntimeException"
string(46) "ServerRequest::$noSuchProperty does not exist."
