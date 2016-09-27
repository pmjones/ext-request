# int http\Env\Response::isCachedByLastModified([string $header_name = "If-Modified-Since"])

Manually test the header $header_name of the environment's request for a cache hit.
http\Env\Response::send() checks that itself, though.

## Params:

* Optional string $header_name = "If-Modified-Since"  
  The request header to test.

## Returns:

* int, a http\Env\Response::CACHE_* constant.
