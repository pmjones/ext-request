--TEST--
SapiResponseSender::sendContent __toString()
--CGI--
--INI--
expose_php=0
--FILE--
<?php
class Stringable
{
    public function __toString() : string
    {
        return 'foo';
    }
}
$response = new SapiResponse();
$response->setContent(new Stringable());
(new SapiResponseSender())->send($response);
--EXPECT--
foo
