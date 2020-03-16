--TEST--
SapiResponseSender::sendCookies
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new SapiResponse();
$response->setCookie('cookie1', 'v1&%v2');
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
$response->setCookie('empty');
(new SapiResponseSender())->send($response);
var_dump(headers_list());
--EXPECT--
array(5) {
  [0]=>
  string(30) "Set-Cookie: cookie1=v1%26%25v2"
  [1]=>
  string(26) "Set-Cookie: cookie2=v3&%v4"
  [2]=>
  string(122) "Set-Cookie: cookie3=value3; expires=Fri, 13-Feb-2009 23:31:30 GMT; Max-Age=0; path=/path; domain=doma.in; secure; HttpOnly"
  [3]=>
  string(136) "Set-Cookie: cookie4=value4; expires=Fri, 13-Feb-2009 23:31:30 GMT; Max-Age=0; path=/path; domain=doma.in; secure; HttpOnly; SameSite=lax"
  [4]=>
  string(75) "Set-Cookie: empty=deleted; expires=Thu, 01-Jan-1970 00:00:01 GMT; Max-Age=0"
}
