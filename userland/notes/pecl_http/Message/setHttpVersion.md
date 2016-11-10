# http\Message http\Message::setHttpVersion(string $http_version)

Set the HTTP protocol version of the message.
See http\Message::getHttpVersion().

## Params:

* string $http_version  
  The protocol version, e.g. "1.1", optionally prefixed by "HTTP/".

## Returns:

* http\Message, self.

## Notices:

* If a non-standard version separator is encounted.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\BadHeaderException

## Example:

    <?php
    $message = new http\Message;
    var_dump($message->getHttpVersion());
    $message->setHttpVersion("HTTP/1_0");
    var_dump($message->getHttpVersion());
    ?>

Yields:

    string(3) "1.1"
    Notice: Non-standard version separator '_' in HTTP protocol version 'HTTP/1_0'
    string(3) "1.0"
