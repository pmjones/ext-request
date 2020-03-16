--TEST--
SapiResponseSender::sendContent __invoke()
--CGI--
--INI--
expose_php=0
--FILE--
<?php

class Invokable
{
    public function __invoke(SapiResponse $response)
    {
        echo count($response->getHeaders() ?? []);
        echo 'foo';
        return 'bar';
    }
}
$response = new SapiResponse();
$response->setContent(new Invokable());
(new SapiResponseSender())->send($response);
--EXPECT--
0foobar
