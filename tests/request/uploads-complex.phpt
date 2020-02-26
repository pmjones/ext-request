--TEST--
ServerRequest::$uploads (complex)
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$_FILES = [
  'foo' => [
    'name' => [
      'bar' => [
        'bar-a' => '',
        'bar-b' => '',
        'bar-c' => '',
      ],
      'baz' => [
        'baz-a' => '',
        'baz-b' => '',
        'baz-c' => '',
      ],
      'qux' => [
        'qux-a' => '',
        'qux-b' => '',
        'qux-c' => '',
      ],
    ],
    'type' => [
      'bar' => [
        'bar-a' => '',
        'bar-b' => '',
        'bar-c' => '',
      ],
      'baz' => [
        'baz-a' => '',
        'baz-b' => '',
        'baz-c' => '',
      ],
      'qux' => [
        'qux-a' => '',
        'qux-b' => '',
        'qux-c' => '',
      ],
    ],
    'tmp_name' => [
      'bar' => [
        'bar-a' => '',
        'bar-b' => '',
        'bar-c' => '',
      ],
      'baz' => [
        'baz-a' => '',
        'baz-b' => '',
        'baz-c' => '',
      ],
      'qux' => [
        'qux-a' => '',
        'qux-b' => '',
        'qux-c' => '',
      ],
    ],
    'error' => [
      'bar' => [
        'bar-a' => 4,
        'bar-b' => 4,
        'bar-c' => 4,
      ],
      'baz' => [
        'baz-a' => 4,
        'baz-b' => 4,
        'baz-c' => 4,
      ],
      'qux' => [
        'qux-a' => 4,
        'qux-b' => 4,
        'qux-c' => 4,
      ],
    ],
    'size' => [
      'bar' => [
        'bar-a' => 0,
        'bar-b' => 0,
        'bar-c' => 0,
      ],
      'baz' => [
        'baz-a' => 0,
        'baz-b' => 0,
        'baz-c' => 0,
      ],
      'qux' => [
        'qux-a' => 0,
        'qux-b' => 0,
        'qux-c' => 0,
      ],
    ],
  ],
];
$request = new ServerRequest($GLOBALS);
$output = var_export($request->uploads, true);
$output = str_replace(' ' . PHP_EOL, "\n", $output);
echo $output;
--EXPECTF--
array (
  'foo' =>
  array (
    'bar' =>
    array (
      'bar-a' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
      'bar-b' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
      'bar-c' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
    ),
    'baz' =>
    array (
      'baz-a' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
      'baz-b' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
      'baz-c' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
    ),
    'qux' =>
    array (
      'qux-a' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
      'qux-b' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
      'qux-c' =>
      array (
        'error' => 4,
        'name' => '',
        'size' => 0,
        'tmp_name' => '',
        'type' => '',
      ),
    ),
  ),
)
