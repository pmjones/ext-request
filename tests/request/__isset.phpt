--TEST--
ServerRequest::__isset
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new ServerRequest();
var_dump(isset($request->method));
var_dump(isset($request->noSuchProperty));
/*
string(16) "RuntimeException"
string(43) "ServerRequest::$noSuchProperty does not exist."
try {
    isset($request->noSuchProperty);
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
*/
--EXPECT--
bool(true)
bool(false)
