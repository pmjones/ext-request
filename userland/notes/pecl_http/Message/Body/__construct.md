# void http\Message\Body::__construct([resource $stream = NULL])

Create a new message body, optionally referencing $stream.

## Params:

* Optional resource $stream = NULL  
  A stream to be used as message body.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\UnexpectedValueException

## Example:

    <?php
    ob_end_clean();
    
    $body = new http\Message\Body(fopen(__FILE__, "r"));
    $resp = new http\Env\Response;
    $resp->setContentType("text/plain");
    $resp->setBody($body);
    $resp->send();
    ?>

Yields:

    Accept-Ranges: bytes
    X-Powered-By: PHP/5.5.5
    Content-Type: text/plain
    ETag: "138042e-527b5a0a-1b2"
    Last-Modified: Thu, 07 Nov 2013 09:14:50 GMT

    # void http\Message\Body::__construct([resource $stream = NULL])

    Create a new message body, optionally referencing $stream.
    ...
