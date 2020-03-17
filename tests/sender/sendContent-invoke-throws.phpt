--TEST--
SapiResponseSender::sendContent __invoke() that throws
--CGI--
--INI--
expose_php=0
--FILE--
<?php

class Invokable
{
    public function __invoke(SapiResponse $response)
    {
        echo "foo\n";
        throw new RuntimeException("failure");
        return "bar";
    }
}
$response = new SapiResponse();
$response->setContent(new Invokable());
try {
    (new SapiResponseSender())->send($response);
} catch (Throwable $e) {
    echo $e->getMessage();
}
--EXPECT--
foo
failure
