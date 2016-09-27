# void http\Env\Response::__construct()

Create a new env response message instance.

## Params:

None.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\UnexpectedValueException

## Example:

    <?php
    $res = new http\Env\Response;
    $res->setContentType("text/plain");
    $res->getBody()->append("Hello world!\n");
    $res->send(STDOUT);
    ?>

Yields:

    HTTP/1.1 200 OK
    Accept-Ranges: bytes
    Content-Type: text/plain
    ETag: "b2a9e441"

    Hello world!
