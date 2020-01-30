--TEST--
ServerResponse::sendContent __invoke()
--CGI--
--INI--
expose_php=0
--FILE--
<?php

class Invokable
{
    public function __invoke(ServerResponse $response)
    {
        echo count($response->getHeaders());
        echo 'foo';
        return 'bar';
    }
}
$response = new ServerResponse();
$response->setContent(new Invokable());
(new ServerResponseSender())->send($response);
--EXPECT--
0foobar
