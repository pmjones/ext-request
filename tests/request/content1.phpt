--TEST--
ServerRequest::$content (cgi)
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--POST_RAW--
Content-Type: application/x-www-form-urlencoded
a=1&b=ZYX
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new ServerRequest();
var_dump($request->content);
try {
    $request->content = 'foo';
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage(), $request->content);
}
--EXPECT--
string(9) "a=1&b=ZYX"
string(16) "RuntimeException"
string(37) "ServerRequest::$content is read-only."
string(9) "a=1&b=ZYX"
