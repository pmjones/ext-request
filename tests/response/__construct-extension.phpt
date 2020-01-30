--TEST--
ServerResponse::__construct without calling parent
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
--EXPECT--
object(ExtResponse)#1 (6) {
  ["version":"ServerResponse":private]=>
  string(3) "1.1"
  ["code":"ServerResponse":private]=>
  int(200)
  ["headers":"ServerResponse":private]=>
  NULL
  ["cookies":"ServerResponse":private]=>
  NULL
  ["content":"ServerResponse":private]=>
  NULL
  ["callbacks":"ServerResponse":private]=>
  NULL
}
