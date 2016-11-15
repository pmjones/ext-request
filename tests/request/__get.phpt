--TEST--
ServerRequest::__get
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
try {
    $request->noSuchProperty;
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
--EXPECT--
bool(true)
string(16) "RuntimeException"
string(46) "ServerRequest::$noSuchProperty does not exist."
