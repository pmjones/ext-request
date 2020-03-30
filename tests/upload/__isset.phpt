--TEST--
SapiUpload::__isset
--FILE--
<?php
$upload = new SapiUpload('foo', 'text/plain', 123, '/tmp/foo.txt', 0);
var_dump(isset($upload->name));
var_dump(isset($upload->noSuchProperty));
--EXPECT--
bool(true)
bool(false)
