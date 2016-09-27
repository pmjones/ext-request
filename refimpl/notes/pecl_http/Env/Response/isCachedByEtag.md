# int http\Env\Response::isCachedByEtag([string $header_name = "If-None-Match"])

Manually test the header $header_name of the environment's request for a cache hit.
http\Env\Response::send() checks that itself, though.

## Params:

* Optional string $header_name = "If-None-Match"  
  The request header to test.

## Returns:

* int, a http\Env\Response::CACHE_* constant.
