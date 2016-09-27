# http\Message http\Message::toStream(resource $stream[, int $offset = 0[, int $maxlen = 0]])

Stream the message into stream $stream, starting from $offset, streaming $maxlen at most.

## Params:

* resource $stream  
  The resource to write to.
* Optional int $offset = 0  
  The starting offset.
* Optional int $maxlen = 0  
  The maximum amount of data to stream. All content if less than 1.

## Returns:

* http\Message, self.
