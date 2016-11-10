# http\Message http\Message::setResponseCode(int $response_code[, bool $strict = true])

Set the response status code.
See http\Message::getResponseCode() and http\Message::setResponseStatus().

> ***NOTE:***  
> This method also resets the response status phrase to the default for that code.

## Params:

* int $response_code  
  The response code.
* Optional bool $strict = true  
  Whether to check that the response code is between 100 and 599 inclusive.

## Returns:

* http\Message, self.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\BadMethodCallException
