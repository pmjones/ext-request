--TEST--
ServerRequest::$uploads (trivial)
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
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
$request = new ServerRequest();
var_dump($request->uploads);
--EXPECTF--
array(5) {
  ["foo1"]=>
  array(5) {
    ["error"]=>
    int(4)
    ["name"]=>
    string(0) ""
    ["size"]=>
    int(0)
    ["tmp_name"]=>
    string(0) ""
    ["type"]=>
    string(0) ""
  }
  ["foo2"]=>
  array(5) {
    ["error"]=>
    int(4)
    ["name"]=>
    string(0) ""
    ["size"]=>
    int(0)
    ["tmp_name"]=>
    string(0) ""
    ["type"]=>
    string(0) ""
  }
  ["foo3"]=>
  array(5) {
    ["error"]=>
    int(4)
    ["name"]=>
    string(0) ""
    ["size"]=>
    int(0)
    ["tmp_name"]=>
    string(0) ""
    ["type"]=>
    string(0) ""
  }
  ["bar"]=>
  array(3) {
    [0]=>
    array(5) {
      ["error"]=>
      int(4)
      ["name"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmp_name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
    }
    [1]=>
    array(5) {
      ["error"]=>
      int(4)
      ["name"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmp_name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
    }
    [2]=>
    array(5) {
      ["error"]=>
      int(4)
      ["name"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmp_name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
    }
  }
  ["baz"]=>
  array(3) {
    ["baz1"]=>
    array(5) {
      ["error"]=>
      int(4)
      ["name"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmp_name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
    }
    ["baz2"]=>
    array(5) {
      ["error"]=>
      int(4)
      ["name"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmp_name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
    }
    ["baz3"]=>
    array(5) {
      ["error"]=>
      int(4)
      ["name"]=>
      string(0) ""
      ["size"]=>
      int(0)
      ["tmp_name"]=>
      string(0) ""
      ["type"]=>
      string(0) ""
    }
  }
}
