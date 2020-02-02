--TEST--
ServerResponse::__construct (extended without calling parent)
--FILE--
<?php
class ExtResponse extends ServerResponse
{
    public function __construct()
    {
    }
}
$ext = new ExtResponse();
var_dump($ext);
var_dump($ext->getHeaders());
--EXPECT--
object(ExtResponse)#1 (6) {
  ["version":"ServerResponse":private]=>
  NULL
  ["code":"ServerResponse":private]=>
  NULL
  ["headers":"ServerResponse":private]=>
  NULL
  ["cookies":"ServerResponse":private]=>
  NULL
  ["content":"ServerResponse":private]=>
  NULL
  ["callbacks":"ServerResponse":private]=>
  NULL
}
NULL
