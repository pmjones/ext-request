--TEST--
SapiResponseSender::send (extended without constructor)
--CGI--
--INI--
expose_php=0
--FILE--
<?php
class ExtResponse extends SapiResponse
{
}
$ext = new ExtResponse();
$ext->setHeader('foo', 'bar');
$ext->setCookie('baz', 'dib');
$ext->setContent('content');
(new SapiResponseSender())->send($ext);
echo "\n";
var_dump(headers_list());
--EXPECT--
content
array(3) {
  [0]=>
  string(8) "foo: bar"
  [1]=>
  string(19) "Set-Cookie: baz=dib"
  [2]=>
  string(38) "Content-type: text/html; charset=UTF-8"
}
