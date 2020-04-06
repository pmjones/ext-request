--TEST--
SapiUpload::__get
--FILE--
<?php
$upload = new SapiUpload('foo', 'text/plain', 123, '/tmp/foo.txt', 0);
var_dump($upload->name);
var_dump($upload->type);
var_dump($upload->size);
var_dump($upload->tmpName);
var_dump($upload->error);
--EXPECT--
string(3) "foo"
string(10) "text/plain"
int(123)
string(12) "/tmp/foo.txt"
int(0)
