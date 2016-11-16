--TEST--
ServerRequest - auto_globals_jit
--SKIPIF--
<?php if (
    ! extension_loaded('request')
) {
    die('skip ');
} ?>
--INI--
auto_globals_jit=1
--POST--
foo=bar&baz=bat
--FILE--
<?php
$var = '_SERVER';
var_dump(isset($$var));
// this will not error if $_SERVER is not defined, but return null - so we want it to error out here
try {
    $request = new ServerRequest();
    var_dump($request->server);
} catch( Exception $e ) {
    var_dump(true);
}
--EXPECT--
bool(false)
bool(true)
