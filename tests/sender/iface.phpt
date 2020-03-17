--TEST--
SapiResponseSender::send with custom SapiResponseInterface
--CGI--
--INI--
expose_php=0
--FILE--
<?php
include "../MySapiResponse.inc";
$response = new MySapiResponse();
$response->setCode(400);
$response->setContent('foo');
$response->setHeader('Foo', 'bar');
$response->setCookie('cookie1', 'v1&%v2');
$response->addHeaderCallback(function($response) {
    $response->addHeader('Baz', 'zim');
});
(new SapiResponseSender())->send($response);
echo "\n";
var_dump(http_response_code());
var_dump(headers_list());
--EXPECTHEADERS--
foo: bar
baz: zim
--EXPECT--
foo
int(400)
array(4) {
  [0]=>
  string(8) "foo: bar"
  [1]=>
  string(8) "baz: zim"
  [2]=>
  string(30) "Set-Cookie: cookie1=v1%26%25v2"
  [3]=>
  string(38) "Content-type: text/html; charset=UTF-8"
}
