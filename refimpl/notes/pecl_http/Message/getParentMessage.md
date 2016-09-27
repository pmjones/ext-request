# http\Message http\Message::getParentMessage()

Retrieve any parent message.
See http\Message::reverse().

## Params:

None.

## Returns:

* http/Message, the parent message.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\BadMethodCallException


## Example:

    <?php
    $message = new http\Message(
        "GET / HTTP/1.0\n".
        "HTTP/1.0 200 Ok"
    );
    var_dump($message->getParentMessage()->toString());
    ?>

Yields:

    string(16) "GET / HTTP/1.0
    "
