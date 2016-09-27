# http\Message http\Message::addHeader(string $name, mixed $value)

Add an header, appending to already existing headers.
See http\Message::addHeaders() and http\Message::setHeader().

## Params:

* string $name  
  The header name.
* mixed $value  
  The header value.

## Returns:

* http\Message, self.

## Example:

    <?php
    $msg = new http\Message;
    
    $msg->addHeader("X-Num", "123");
    $msg->addHeader("X-Num", "456");
    
    echo $msg;
    ?>

Yields:

    X-Num: 123, 456
