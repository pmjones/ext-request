# http\Message\Body http\Message\Body::addPart(http\Message $part)

Add a part to a multipart body.

## Params:

* http\Message $part  
  The message part.

## Returns:

* http\Message\Body, self.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\RuntimeException

## Example:

    <?php
    $multi = new http\Message\Body;
    $multi->addPart(new http\Message("Content-type: text/plain\n\nHello part 1!"));
    $multi->addPart(new http\Message("Content-type: text/plain\n\nHello part 2!"));
    
    echo $multi;
    ?>

Yields:

    --8a72b190.3fe908df
    Content-Type: text/plain
    Content-Length: 13

    Hello part 1!
    --8a72b190.3fe908df
    Content-Type: text/plain
    Content-Length: 13

    Hello part 2!
    --8a72b190.3fe908df--
