# void http\Message::rewind()

Implements Iterator.

## Params:

None.

## Example:

    <?php
    $types = ["message", "request", "response"];
    $message = new http\Message("GET / HTTP/1.1\n".
        "HTTP/1.1 200\n".
        "GET / HTTP/1.0\n".
        "HTTP/1.0 426"
    );
    foreach ($message as $key => $msg) {
        printf("Key %d is a %8s: %s\n",
            $key, $types[$msg->getType()],
            $msg->getInfo()
        );
    }
    ?>

Yields: 

    Key 2 is a response: HTTP/1.0 426
    Key 8 is a  request: GET / HTTP/1.0
    Key 6 is a response: HTTP/1.1 200
    Key 4 is a  request: GET / HTTP/1.1
