--TEST--
ServerRequest::__get extension indirect modification
--SKIPIF--
<?php if (
    ! extension_loaded('request')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new ServerRequest();
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
string(36) "ServerRequest::$accept is read-only."
string(16) "RuntimeException"
string(36) "ServerRequest::$method is read-only."
