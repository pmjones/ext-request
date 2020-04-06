--TEST--
SapiUpload::__set
--FILE--
<?php
$upload = new SapiUpload('foo', 'text/plain', 123, '/tmp/foo.txt', 0);
try {
    $upload->name = 'bar';
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
try {
    $upload->noSuchProperty = 'foo';
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
--EXPECT--
string(16) "RuntimeException"
string(31) "SapiUpload::$name is read-only."
string(16) "RuntimeException"
string(43) "SapiUpload::$noSuchProperty does not exist."
