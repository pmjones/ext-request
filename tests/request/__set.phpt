--TEST--
ServerRequest::__set
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
string(36) "ServerRequest::$method is read-only."
string(16) "RuntimeException"
string(44) "ServerRequest::$noSuchProperty is read-only."
