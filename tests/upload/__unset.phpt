--TEST--
SapiUpload::__unset
--FILE--
<?php
$upload = new SapiUpload('foo', 'text/plain', 123, '/tmp/foo.txt', 0);
unset($upload->no_such_prop);
try {
    unset($upload->name);
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
--EXPECT--
string(16) "RuntimeException"
string(31) "SapiUpload::$name is read-only."
