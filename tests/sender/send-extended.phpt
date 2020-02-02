--TEST--
ServerResponseSender::send (extended without constructor)
--CGI--
--INI--
expose_php=0
--FILE--
<?php
class ExtResponse extends ServerResponse
{
}
$ext = new ExtResponse();
$ext->setHeader('foo', 'bar');
$ext->setCookie('baz', 'dib');
$ext->setContent('content');
(new ServerResponseSender())->send($ext);
echo PHP_EOL;
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
