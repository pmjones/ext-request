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
$response->setCookie('cookie1', 'v1&%v2');
$response->setRawCookie('cookie2', 'v3&%v4');
(new ServerResponseSender())->send($response);
var_dump(headers_list());
--EXPECT--
array(2) {
  [0]=>
  string(30) "Set-Cookie: cookie1=v1%26%25v2"
  [1]=>
  string(26) "Set-Cookie: cookie2=v3&%v4"
}
