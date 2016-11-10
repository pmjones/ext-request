# void http\Message::__construct([mixed $message = NULL[, bool $greedy = true])

Create a new HTTP message.

## Params:

* Optional mixed $message = NULL  
  Either a resource or a string, representing the HTTP message.
* Optional bool $greedy = true  
  Whether to read from a $message resource until EOF.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\BadMessageException
