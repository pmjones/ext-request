--TEST--
SapiRequest::$files
--POST_RAW--
Content-Type: multipart/form-data; boundary=---------------------------20896060251896012921717172737
-----------------------------20896060251896012921717172737
Content-Disposition: form-data; name="MAX_FILE_SIZE"

1
-----------------------------20896060251896012921717172737
Content-Disposition: form-data; name="file1"; filename="file1.txt"
Content-Type: text/plain-file1

1
-----------------------------20896060251896012921717172737--
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$request = new SapiRequest($GLOBALS);
var_dump($request->files);
var_dump($request->files == $_FILES);
--EXPECTF--
array(1) {
  ["file1"]=>
  array(5) {
    ["name"]=>
    string(9) "file1.txt"
    ["type"]=>
    string(16) "text/plain-file1"
    ["tmp_name"]=>
    string(%d) "%s"
    ["error"]=>
    int(0)
    ["size"]=>
    int(1)
  }
}
bool(true)
