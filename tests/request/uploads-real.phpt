--TEST--
SapiRequest::$uploads (real)
--POST_RAW--
Content-Type: multipart/form-data; boundary=---------------------------20896060251896012921717172737
-----------------------------20896060251896012921717172737
Content-Disposition: form-data; name="MAX_FILE_SIZE"

1
-----------------------------20896060251896012921717172737
Content-Disposition: form-data; name="file1"; filename="file1.txt"
Content-Type: text/plain-file1

1
-----------------------------20896060251896012921717172737
Content-Disposition: form-data; name="file2"; filename="file2.txt"
Content-Type: text/plain-file2

22
-----------------------------20896060251896012921717172737
Content-Disposition: form-data; name="file3"; filename="C:\foo\bar/file3.txt"
Content-Type: text/plain-file3;

3
-----------------------------20896060251896012921717172737--
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$request = new SapiRequest($GLOBALS);
var_dump($request->uploads);
--EXPECTF--
array(3) {
  ["file1"]=>
  object(SapiUpload)#%d (5) {
    ["name"]=>
    string(9) "file1.txt"
    ["type"]=>
    string(16) "text/plain-file1"
    ["size"]=>
    int(1)
    ["tmpName"]=>
    string(%d) "%s"
    ["error"]=>
    int(0)
  }
  ["file2"]=>
  object(SapiUpload)#%d (5) {
    ["name"]=>
    string(9) "file2.txt"
    ["type"]=>
    string(0) ""
    ["size"]=>
    int(0)
    ["tmpName"]=>
    string(0) ""
    ["error"]=>
    int(2)
  }
  ["file3"]=>
  object(SapiUpload)#%d (5) {
    ["name"]=>
    string(9) "file3.txt"
    ["type"]=>
    string(16) "text/plain-file3"
    ["size"]=>
    int(1)
    ["tmpName"]=>
    string(%d) "%s"
    ["error"]=>
    int(0)
  }
}
