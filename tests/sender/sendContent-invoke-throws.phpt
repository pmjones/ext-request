--TEST--
ServerResponseSender::sendContent __invoke() that throws
--CGI--
--INI--
expose_php=0
--FILE--
<?php

class Invokable
{
    public function __invoke(ServerResponse $response)
    {
        echo "foo\n";
        throw new RuntimeException("failure");
        return "bar";
    }
}
$response = new ServerResponse();
$response->setContent(new Invokable());
try {
    (new ServerResponseSender())->send($response);
} catch (Throwable $e) {
    echo $e->getMessage();
}
--EXPECT--
foo
failure
