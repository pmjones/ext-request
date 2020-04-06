--TEST--
SapiRequest::$uploads (trivial)
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$_FILES = [
  'foo1' => [
    'error' => 4,
    'name' => '',
    'size' => 0,
    'tmp_name' => '',
    'type' => '',
  ],
  'foo2' => [
    'error' => 4,
    'name' => '',
    'size' => 0,
    'tmp_name' => '',
    'type' => '',
  ],
  'foo3' => [
    'error' => 4,
    'name' => '',
    'size' => 0,
    'tmp_name' => '',
    'type' => '',
  ],
  'bar' => [
    'name' => [
      0 => '',
      1 => '',
      2 => '',
    ],
    'type' => [
      0 => '',
      1 => '',
      2 => '',
    ],
    'tmp_name' => [
      0 => '',
      1 => '',
      2 => '',
    ],
    'error' => [
      0 => 4,
      1 => 4,
      2 => 4,
    ],
    'size' => [
      0 => 0,
      1 => 0,
      2 => 0,
    ],
  ],
  'baz' => [
    'name' => [
      'baz1' => '',
      'baz2' => '',
      'baz3' => '',
    ],
    'type' => [
      'baz1' => '',
      'baz2' => '',
      'baz3' => '',
    ],
    'tmp_name' => [
      'baz1' => '',
      'baz2' => '',
      'baz3' => '',
    ],
    'error' => [
      'baz1' => 4,
      'baz2' => 4,
      'baz3' => 4,
    ],
    'size' => [
      'baz1' => 0,
      'baz2' => 0,
      'baz3' => 0,
    ],
  ],
];
$request = new SapiRequest($GLOBALS);
var_dump($request->uploads);
--EXPECTF--
array(5) {
  ["foo1"]=>
  object(SapiUpload)#%d (5) {
    ["name"]=>
    string(0) ""
    ["type"]=>
    string(0) ""
    ["size"]=>
    int(0)
    ["tmpName"]=>
    string(0) ""
    ["error"]=>
    int(4)
  }
  ["foo2"]=>
  object(SapiUpload)#%d (5) {
    ["name"]=>
    string(0) ""
    ["type"]=>
    string(0) ""
    ["size"]=>
    int(0)
    ["tmpName"]=>
    string(0) ""
    ["error"]=>
    int(4)
  }
  ["foo3"]=>
  object(SapiUpload)#%d (5) {
    ["name"]=>
    string(0) ""
    ["type"]=>
    string(0) ""
    ["size"]=>
    int(0)
    ["tmpName"]=>
    string(0) ""
    ["error"]=>
    int(4)
  }
  ["bar"]=>
  array(3) {
    [0]=>
    object(SapiUpload)#%d (5) {
      ["name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmpName"]=>
      string(0) ""
      ["error"]=>
      int(4)
    }
    [1]=>
    object(SapiUpload)#%d (5) {
      ["name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmpName"]=>
      string(0) ""
      ["error"]=>
      int(4)
    }
    [2]=>
    object(SapiUpload)#%d (5) {
      ["name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmpName"]=>
      string(0) ""
      ["error"]=>
      int(4)
    }
  }
  ["baz"]=>
  array(3) {
    ["baz1"]=>
    object(SapiUpload)#%d (5) {
      ["name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmpName"]=>
      string(0) ""
      ["error"]=>
      int(4)
    }
    ["baz2"]=>
    object(SapiUpload)#%d (5) {
      ["name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmpName"]=>
      string(0) ""
      ["error"]=>
      int(4)
    }
    ["baz3"]=>
    object(SapiUpload)#%d (5) {
      ["name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmpName"]=>
      string(0) ""
      ["error"]=>
      int(4)
    }
  }
}
