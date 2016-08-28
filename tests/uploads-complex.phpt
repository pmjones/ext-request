--TEST--
PhpRequest::$uploads (complex)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$_FILES = [
  'dib' => [
    'name' => [
      'dib1' => [
        'dib1a' => '',
        'dib1b' => '',
        'dib1c' => '',
      ],
      'dib2' => [
        'dib2a' => '',
        'dib2b' => '',
        'dib2c' => '',
      ],
      'dib3' => [
        'dib3a' => '',
        'dib3b' => '',
        'dib3c' => '',
      ],
    ],
    'type' => [
      'dib1' => [
        'dib1a' => '',
        'dib1b' => '',
        'dib1c' => '',
      ],
      'dib2' => [
        'dib2a' => '',
        'dib2b' => '',
        'dib2c' => '',
      ],
      'dib3' => [
        'dib3a' => '',
        'dib3b' => '',
        'dib3c' => '',
      ],
    ],
    'tmp_name' => [
      'dib1' => [
        'dib1a' => '',
        'dib1b' => '',
        'dib1c' => '',
      ],
      'dib2' => [
        'dib2a' => '',
        'dib2b' => '',
        'dib2c' => '',
      ],
      'dib3' => [
        'dib3a' => '',
        'dib3b' => '',
        'dib3c' => '',
      ],
    ],
    'error' => [
      'dib1' => [
        'dib1a' => 4,
        'dib1b' => 4,
        'dib1c' => 4,
      ],
      'dib2' => [
        'dib2a' => 4,
        'dib2b' => 4,
        'dib2c' => 4,
      ],
      'dib3' => [
        'dib3a' => 4,
        'dib3b' => 4,
        'dib3c' => 4,
      ],
    ],
    'size' => [
      'dib1' => [
        'dib1a' => 0,
        'dib1b' => 0,
        'dib1c' => 0,
      ],
      'dib2' => [
        'dib2a' => 0,
        'dib2b' => 0,
        'dib2c' => 0,
      ],
      'dib3' => [
        'dib3a' => 0,
        'dib3b' => 0,
        'dib3c' => 0,
      ],
    ],
  ],
];
$request = new PhpRequest();
var_dump($request->uploads);
--EXPECTF--
array(1) {
  ["dib"]=>
  array(3) {
    ["dib1"]=>
    array(3) {
      ["dib1a"]=>
      object(stdClass)#%d (5) {
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
      ["dib1b"]=>
      object(stdClass)#%d (5) {
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
      ["dib1c"]=>
      object(stdClass)#%d (5) {
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
    ["dib2"]=>
    array(3) {
      ["dib2a"]=>
      object(stdClass)#%d (5) {
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
      ["dib2b"]=>
      object(stdClass)#%d (5) {
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
      ["dib2c"]=>
      object(stdClass)#%d (5) {
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
    ["dib3"]=>
    array(3) {
      ["dib3a"]=>
      object(stdClass)#%d (5) {
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
      ["dib3b"]=>
      object(stdClass)#%d (5) {
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
      ["dib3c"]=>
      object(stdClass)#%d (5) {
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
}