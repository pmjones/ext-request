# http\Message http\Message::splitMultipartBody()

Splits the body of a multipart message.
See http\Message::isMultipart() and http\Message\Body::addPart().

## Params:

None.

## Returns:

* http\Message, a message chain of all messages of the multipart body.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\BadMethodCallException
* http\Exception\BadMessageException

## Example:

    <?php
    $body = new http\Message\Body;
    $body->addPart(new http\Message("Content-type: text/plain\n\nHello "));
    $body->addPart(new http\Message("Content-type: text/plain\n\nWorld!"));

    $msg = new http\Message;
    $msg->setHeader("Content-Type", 
        "multipart/mixed; boundary=" . $body->getBoundary());
    $msg->setBody($body);
    var_dump($msg->isMultipart($bnd), $bnd);
    
    $parts = $msg->splitMultipartBody();
    var_dump(count($parts));
    foreach ($parts->reverse() as $part) {
        echo $part->getBody();
    }
    ?>

Yields:

    bool(true)
    string(17) "16658735.3fe37486"
    int(2)
    Hello World!
