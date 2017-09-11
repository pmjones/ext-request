--TEST--
ServerRequest return type reflection
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
foreach(['withInput', 'withParam', 'withParams', 'withoutParam', 'withoutParams', 'withUrl'] as $method) {
  $r = new ReflectionMethod("ServerRequest", $method);
  var_dump((string)$r->getReturnType());
}
?>
Done
--EXPECT--
string(13) "ServerRequest"
string(13) "ServerRequest"
string(13) "ServerRequest"
string(13) "ServerRequest"
string(13) "ServerRequest"
string(13) "ServerRequest"
Done
