# http\Message http\Message::toCallback(callable $callback[, int $offset = 0[, int $maxlen = 0]])

Stream the message through a callback.

## Params:

* callable $callback  
  The callback of the form function(http\Message $from, string $data).
* Optional int $offset = 0  
  Start to stream from this offset.
* Optional int $maxlen = 0  
  Stream at most $maxlen bytes, or all if $maxlen is less than 1.

## Returns:

* http\Message, self.
