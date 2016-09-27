# string http\Message\Body::__toString()

String cast handler.

## Params:

None.

## Returns:

* string, the message body.

## Example:

    <?php
    $body = new http\Message\Body;
    $body->append("this\nis\nan\nexample!\n");
    echo $body;
    ?>

Yields:

    this
    is
    an
    example!
