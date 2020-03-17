--TEST--
SapiResponse::__construct (extended without calling parent)
--FILE--
<?php
class ExtResponse extends SapiResponse
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
  ["version":"SapiResponse":private]=>
  NULL
  ["code":"SapiResponse":private]=>
  NULL
  ["headers":"SapiResponse":private]=>
  NULL
  ["cookies":"SapiResponse":private]=>
  NULL
  ["content":"SapiResponse":private]=>
  NULL
  ["callbacks":"SapiResponse":private]=>
  NULL
}
NULL
