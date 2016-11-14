--TEST--
ServerResponse::setCookie
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$response = new ServerResponse();
$response->setCookie('foo', 'bar');
$response->setRawCookie('baz', 'dib');
var_dump($response->getCookies());
--EXPECT--
array(2) {
  ["foo"]=>
  array(7) {
    ["raw"]=>
    bool(false)
    ["value"]=>
    string(3) "bar"
    ["expire"]=>
    int(0)
    ["path"]=>
    string(0) ""
    ["domain"]=>
    string(0) ""
    ["secure"]=>
    bool(false)
    ["httponly"]=>
    bool(false)
  }
  ["baz"]=>
  array(7) {
    ["raw"]=>
    bool(true)
    ["value"]=>
    string(3) "dib"
    ["expire"]=>
    int(0)
    ["path"]=>
    string(0) ""
    ["domain"]=>
    string(0) ""
    ["secure"]=>
    bool(false)
    ["httponly"]=>
    bool(false)
  }
}
