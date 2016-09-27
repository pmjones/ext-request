# http\Message\Body http\Message\Body::toCallback(callable $callback[, int $offset = 0[, int $maxlen = 0]])

Stream the message body through a callback.

## Params:

* callable $callback  
  The callback of the form function(http\Message\Body $from, string $data).
* Optional int $offset = 0  
  Start to stream from this offset.
* Optional int $maxlen = 0  
  Stream at most $maxlen bytes, or all if $maxlen is less than 1.

## Returns:

* http\Message\Body, self.
