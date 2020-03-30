--TEST--
SapiRequest::$uploads (complex)
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$_FILES = [
  'foo' => [
    'name' => 'foo_name',
    'type' => 'foo_type',
    'tmp_name' => 'foo_tmp_name',
    'error' => 4,
    'size' => 0,
  ],
  'dib' => [
    'name' => [
      'dib1' => [
        'dib1a' => 'dib1a_name',
        'dib1b' => 'dib1b_name',
        'dib1c' => 'dib1c_name',
      ],
      'dib2' => [
        'dib2a' => 'dib2a_name',
        'dib2b' => 'dib2b_name',
        'dib2c' => 'dib2c_name',
      ],
      'dib3' => [
        'dib3a' => 'dib3a_name',
        'dib3b' => 'dib3b_name',
        'dib3c' => 'dib3c_name',
      ],
    ],
    'type' => [
      'dib1' => [
        'dib1a' => 'dib1a_type',
        'dib1b' => 'dib1b_type',
        'dib1c' => 'dib1c_type',
      ],
      'dib2' => [
        'dib2a' => 'dib2a_type',
        'dib2b' => 'dib2b_type',
        'dib2c' => 'dib2c_type',
      ],
      'dib3' => [
        'dib3a' => 'dib3a_type',
        'dib3b' => 'dib3b_type',
        'dib3c' => 'dib3c_type',
      ],
    ],
    'tmp_name' => [
      'dib1' => [
        'dib1a' => 'dib1a_tmp_name',
        'dib1b' => 'dib1b_tmp_name',
        'dib1c' => 'dib1c_tmp_name',
      ],
      'dib2' => [
        'dib2a' => 'dib2a_tmp_name',
        'dib2b' => 'dib2b_tmp_name',
        'dib2c' => 'dib2c_tmp_name',
      ],
      'dib3' => [
        'dib3a' => 'dib3a_tmp_name',
        'dib3b' => 'dib3b_tmp_name',
        'dib3c' => 'dib3c_tmp_name',
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
$request = new SapiRequest($GLOBALS);
var_dump($request->uploads);
--EXPECTF--
array(2) {
  ["foo"]=>
  object(SapiUpload)#%d (5) {
    ["name"]=>
    string(8) "foo_name"
    ["type"]=>
    string(8) "foo_type"
    ["size"]=>
    int(0)
    ["tmpName"]=>
    string(12) "foo_tmp_name"
    ["error"]=>
    int(4)
  }
  ["dib"]=>
  array(3) {
    ["dib1"]=>
    array(3) {
      ["dib1a"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib1a_name"
        ["type"]=>
        string(10) "dib1a_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib1a_tmp_name"
        ["error"]=>
        int(4)
      }
      ["dib1b"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib1b_name"
        ["type"]=>
        string(10) "dib1b_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib1b_tmp_name"
        ["error"]=>
        int(4)
      }
      ["dib1c"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib1c_name"
        ["type"]=>
        string(10) "dib1c_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib1c_tmp_name"
        ["error"]=>
        int(4)
      }
    }
    ["dib2"]=>
    array(3) {
      ["dib2a"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib2a_name"
        ["type"]=>
        string(10) "dib2a_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib2a_tmp_name"
        ["error"]=>
        int(4)
      }
      ["dib2b"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib2b_name"
        ["type"]=>
        string(10) "dib2b_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib2b_tmp_name"
        ["error"]=>
        int(4)
      }
      ["dib2c"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib2c_name"
        ["type"]=>
        string(10) "dib2c_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib2c_tmp_name"
        ["error"]=>
        int(4)
      }
    }
    ["dib3"]=>
    array(3) {
      ["dib3a"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib3a_name"
        ["type"]=>
        string(10) "dib3a_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib3a_tmp_name"
        ["error"]=>
        int(4)
      }
      ["dib3b"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib3b_name"
        ["type"]=>
        string(10) "dib3b_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib3b_tmp_name"
        ["error"]=>
        int(4)
      }
      ["dib3c"]=>
      object(SapiUpload)#%d (5) {
        ["name"]=>
        string(10) "dib3c_name"
        ["type"]=>
        string(10) "dib3c_type"
        ["size"]=>
        int(0)
        ["tmpName"]=>
        string(14) "dib3c_tmp_name"
        ["error"]=>
        int(4)
      }
    }
  }
}
