--TEST--
SapiResponse::getCookie
--FILE--
<?php
$response = new SapiResponse();
$response->setCookie('cookie1', 'v1&%v2');
var_dump($response->getCookie('cookie1'));
var_dump($response->getCookie('cookie2'));
--EXPECT--
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
NULL
