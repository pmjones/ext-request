# http\Message http\Message::setHeaders(array $headers = NULL)

Set the message headers.
See http\Message::getHeaders() and http\Message::addHeaders().

> ***NOTE:***  
> Prior to v2.5.6/v3.1.0 headers with the same name were merged into a single
> header with values concatenated by comma.

## Params:

* array $headers = NULL  
  The message's headers.

## Returns:

* http\Message, null.

## Example:

    <?php
    $msg = new http\Message;
    
    $msg->setHeaders([
        "Content-Type" => "text/plain",
        "Content-Encoding" => "gzip",
        "Content-Location" => "/foo/bar"
    ]);
    var_dump($msg->getHeaders());
    
    $msg->setHeaders(null);
    var_dump($msg->getHeaders());
    ?>

Yields:

    array(3) {
      ["Content-Type"]=>
      string(10) "text/plain"
      ["Content-Encoding"]=>
      string(4) "gzip"
      ["Content-Location"]=>
      string(8) "/foo/bar"
    }
    array(0) {
    }

## Changelog:

0. 2.5.6, 3.1.0
	* Multiple headers with the same name are kept separate instead of merged together.
