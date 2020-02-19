--TEST--
ServerRequest - variables_order
--INI--
variables_order=G
--POST--
foo=bar&baz=bat
--FILE--
<?php
// Currently if a variable is missing from variables_order, it's not available
$_SERVER['HTTP_HOST'] = 'example.com';
var_dump($_POST);
$request = new ServerRequest($GLOBALS);
var_dump($request->input);
--EXPECT--
array(0) {
}
array(0) {
}
