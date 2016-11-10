# static string http\Env::negotiateContentType(array $supported[, array &$result)

Negotiate the client's preferred MIME content type.

> ***NOTE:***  
> The first elemement of $supported content types serves as a default if no content-type matches.

## Params:

* array $supported  
  List of supported MIME content types.
* Optional reference array &$result  
  Out parameter recording negotiation results.
  
## Returns:

* NULL, if negotiation fails.
* string, the negotiated content type.

## Example:

A client indicates his accepted MIME content types by sending an Accept
header. The static http\Env class provides a facility to negotiate the
client's preferred content type:

    <?php
    $_SERVER["HTTP_ACCEPT"] = implode(",", array(
        "text/html",
        "text/plain",
        "text/*;q=0.9",
        "*/*;q=0.1",
        "application/xml;q=0"
    ));
    $supported = array(
        "text/html", 
        "text/plain",
        "application/json", 
        "application/xml"
    );
    $preferred = http\Env::negotiateContentType($supported, $ranking);
    var_dump($preferred, $ranking);
    ?>

Running this script should give the following output:

    string(9) "text/html"
    array(3) {
        'text/html' =>
        float(0.99)
        'text/plain' =>
        float(0.98)
        'application/json' =>
        float(0.1)
    }



