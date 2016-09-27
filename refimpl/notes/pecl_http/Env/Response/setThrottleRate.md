# http\Env\Response http\Env\Response::setThrottleRate(int $chunk_size[, float $delay = 1])

Enable throttling.
Send $chunk_size bytes every $delay seconds.

> ***NOTE:***  
> If you need throttling by regular means, check for other options in your stack, because this method blocks the executing process/thread until the response has completely been sent.

## Params:

* int $chunk_size  
  Bytes to send.
* Optional float $delay = 1  
  Seconds to sleep.

## Returns:

* http\Env\Response, self.
