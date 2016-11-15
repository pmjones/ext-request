--TEST--
ServerRequest::$uploads (complex)
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
$request = new ServerRequest();
var_dump($request->uploads);
--EXPECTF--
array(1) {
  ["dib"]=>
  array(3) {
    ["dib1"]=>
    array(3) {
      ["dib1a"]=>
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
      ["dib1b"]=>
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
      ["dib1c"]=>
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
    ["dib2"]=>
    array(3) {
      ["dib2a"]=>
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
      ["dib2b"]=>
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
      ["dib2c"]=>
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
    ["dib3"]=>
    array(3) {
      ["dib3a"]=>
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
      ["dib3b"]=>
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
      ["dib3c"]=>
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
}
