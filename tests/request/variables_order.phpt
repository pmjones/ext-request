--TEST--
ServerRequest - variables_order
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--INI--
variables_order=G
--POST--
foo=bar&baz=bat
--FILE--
<?php
// Currently if a variable is missing from variables_order, it's not available
$_SERVER['HTTP_HOST'] = 'example.com';
var_dump($_POST);
$request = new ServerRequest();
var_dump($request->post);
--EXPECT--
array(0) {
}
array(0) {
}
