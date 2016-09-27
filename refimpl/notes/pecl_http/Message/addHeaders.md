# http\Message http\Message::addHeaders(array $headers[, bool $append = false])

Add headers, optionally appending values, if header keys already exist.
See http\Message::addHeader() and http\Message::setHeaders().

## Params:

* array $headers  
  The HTTP headers to add.
* Optional bool $append = false  
  Whether to append values for existing headers.

## Returns:

* http\Message, self.

## Example:

    <?php
    $msg = new http\Message;
    
    $msg->addHeaders(["Cache-Control" => "public"]);
    var_dump($msg->getHeaders());
    
    $msg->addHeaders(["Cache-Control" => "private"]);
    var_dump($msg->getHeaders());
    
    $msg->addHeaders(["Cache-Control" => "must-revalidate"], true);
    var_dump($msg->getHeaders());
    
    echo $msg;
    ?>

Yields:

    array(1) {
      ["Cache-Control"]=>
      string(6) "public"
    }
    array(1) {
      ["Cache-Control"]=>
      string(7) "private"
    }
    array(1) {
      ["Cache-Control"]=>
      string(24) "private, must-revalidate"
    }
    Cache-Control: private, must-revalidate

