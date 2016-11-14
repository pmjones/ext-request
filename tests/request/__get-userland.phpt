--TEST--
ServerRequest::__get userland indirect modification
--SKIPIF--
<?php if (
    ! getenv('TEST_USERLAND_REQUEST')
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
--EXPECTF--

Notice: Indirect modification of overloaded property ServerRequest::$accept has no effect in %s/__get-userland.php on line 5

Notice: Indirect modification of overloaded property ServerRequest::$method has no effect in %s/__get-userland.php on line 13
