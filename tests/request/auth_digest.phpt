--TEST--
ServerRequest - digest auth
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER += [
    'HTTP_HOST' => 'example.com',
    'PHP_AUTH_TYPE' => 'Digest',
    'PHP_AUTH_DIGEST' => implode(',', [
        'nonce="foo"',
        'nc=\'bar\'',
        'cnonce=baz',
        'qop="dib"',
        'username="zim"',
        'uri="gir"',
        'response="irk"',
    ]),
];
$request = new ServerRequest();
var_dump($request->authDigest);

// missing parts
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'PHP_AUTH_TYPE' => 'Digest',
    'PHP_AUTH_DIGEST' => implode(',', [
        'nonce="foo"',
        'nc=\'bar\'',
        'cnonce=baz',
    ]),
];
$request = new ServerRequest();
var_dump($request->authDigest);

--EXPECTF--
array(7) {
  ["nonce"]=>
  string(3) "foo"
  ["nc"]=>
  string(3) "bar"
  ["cnonce"]=>
  string(3) "baz"
  ["qop"]=>
  string(3) "dib"
  ["username"]=>
  string(3) "zim"
  ["uri"]=>
  string(3) "gir"
  ["response"]=>
  string(3) "irk"
}
NULL
