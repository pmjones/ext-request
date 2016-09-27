# bool http\Env\Response::send([resource $stream = NULL])

Send the response through the SAPI or $stream.
Flushes all output buffers.

## Params:

* Optional resource $stream = NULL  
  A writable stream to send the response through.

## Returns:

* bool, success.
