# bool http\Response::__invoke(string $data[, int $ob_flags = 0])

Output buffer handler.
Appends output data to the body.

## Params:

* string $data  
  The data output.
* Optional int $ob_flags = 0  
  Output buffering flags passed from the output buffering control layer.

## Returns:

* bool, success.

## Example:

    <?php
    $res = new http\Env\Response;
    $res->setContentType("text/plain");

    ob_start($res);
    // yeah, well
    // not the greatest application logic...
    echo "Hello world!\n";

    $res->send();
    ?>

Yields:

    Accept-Ranges: bytes
    X-Powered-By: PHP/5.5.5
    Content-Type: text/plain
    ETag: "b2a9e441"

    Hello world!
