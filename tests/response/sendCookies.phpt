--TEST--
ServerResponse::sendCookies
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$response->setCookie('cookie1', 'value1');
$response->setRawCookie('cookie2', 'value2');
$response->send();
var_dump(headers_list());
// it appears EXPECTHEADERS can't handle duplicate headers
--EXPECTHEADERS--
Set-Cookie: cookie2=value2
--EXPECT--
array(2) {
  [0]=>
  string(26) "Set-Cookie: cookie1=value1"
  [1]=>
  string(26) "Set-Cookie: cookie2=value2"
}
