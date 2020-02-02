--TEST--
ServerResponseSender::sendContent __toString()
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
$response = new ServerResponse();
$response->setContent(new Stringable());
(new ServerResponseSender())->send($response);
--EXPECT--
foo
