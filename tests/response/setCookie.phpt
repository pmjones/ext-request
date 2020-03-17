--TEST--
SapiResponse::setCookie
--FILE--
<?php
$response = new SapiResponse();
var_dump($response->setCookie('cookie1', 'v1&%v2') === $response);
$response->setRawCookie('cookie2', 'v3&%v4');
$response->setCookie('cookie3', 'value3', 1234567890, "/path", "doma.in", true, true);
$response->setCookie('cookie4', 'value4', [
    'expires' => 1234567890,
    'path' => "/path",
    'domain' => "doma.in",
    'secure' => true,
    'httponly' => true,
    'samesite' => 'lax',
]);
$response->setCookie('empty', '');
var_dump($response->getCookies());
$response->unsetCookies();
var_dump($response->getCookies());
$response->setCookie('foo', 'bar');
$response->setCookie('baz', 'dib');
$response->unsetCookie('foo');
var_dump($response->getCookies());
--EXPECT--
bool(true)
array(5) {
  ["cookie1"]=>
  array(8) {
    ["value"]=>
    string(6) "v1&%v2"
    ["expires"]=>
    int(0)
    ["path"]=>
    string(0) ""
    ["domain"]=>
    string(0) ""
    ["secure"]=>
    bool(false)
    ["httponly"]=>
    bool(false)
    ["samesite"]=>
    string(0) ""
    ["url_encode"]=>
    bool(true)
  }
  ["cookie2"]=>
  array(8) {
    ["value"]=>
    string(6) "v3&%v4"
    ["expires"]=>
    int(0)
    ["path"]=>
    string(0) ""
    ["domain"]=>
    string(0) ""
    ["secure"]=>
    bool(false)
    ["httponly"]=>
    bool(false)
    ["samesite"]=>
    string(0) ""
    ["url_encode"]=>
    bool(false)
  }
  ["cookie3"]=>
  array(8) {
    ["value"]=>
    string(6) "value3"
    ["expires"]=>
    int(1234567890)
    ["path"]=>
    string(5) "/path"
    ["domain"]=>
    string(7) "doma.in"
    ["secure"]=>
    bool(true)
    ["httponly"]=>
    bool(true)
    ["samesite"]=>
    string(0) ""
    ["url_encode"]=>
    bool(true)
  }
  ["cookie4"]=>
  array(8) {
    ["value"]=>
    string(6) "value4"
    ["expires"]=>
    int(1234567890)
    ["path"]=>
    string(5) "/path"
    ["domain"]=>
    string(7) "doma.in"
    ["secure"]=>
    bool(true)
    ["httponly"]=>
    bool(true)
    ["samesite"]=>
    string(3) "lax"
    ["url_encode"]=>
    bool(true)
  }
  ["empty"]=>
  array(8) {
    ["value"]=>
    string(0) ""
    ["expires"]=>
    int(0)
    ["path"]=>
    string(0) ""
    ["domain"]=>
    string(0) ""
    ["secure"]=>
    bool(false)
    ["httponly"]=>
    bool(false)
    ["samesite"]=>
    string(0) ""
    ["url_encode"]=>
    bool(true)
  }
}
NULL
array(1) {
  ["baz"]=>
  array(8) {
    ["value"]=>
    string(3) "dib"
    ["expires"]=>
    int(0)
    ["path"]=>
    string(0) ""
    ["domain"]=>
    string(0) ""
    ["secure"]=>
    bool(false)
    ["httponly"]=>
    bool(false)
    ["samesite"]=>
    string(0) ""
    ["url_encode"]=>
    bool(true)
  }
}
