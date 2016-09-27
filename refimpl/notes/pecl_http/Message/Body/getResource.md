# resource http\Message\Body::getResource()

Retrieve the underlying stream resource.

## Params:

None.

## Returns:

* resource, the underlying stream.

## Example:

    <?php
    $body = new http\Message\Body;
    var_dump($body->getResource());
    ?>

Yields:

    resource(5) of type (stream)
