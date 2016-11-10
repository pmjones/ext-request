# class http\Env\Response extends http\Message is callable

The http\Env\Response class' instances represent the server's current HTTP response.

See http\Message for inherited members.

## Constants:

* CONTENT_ENCODING_NONE  
  Do not use content encoding.
* CONTENT_ENCODING_GZIP  
  Support "Accept-Encoding" requests with gzip and deflate encoding.
* CACHE_NO  
  No caching info available.
* CACHE_HIT  
  The cache was hit.
* CACHE_MISS  
  The cache was missed.

## Properties:

* protected http\Env\Request $request = NULL  
  A request instance which overrides the environments default request.
* protected string $contentType = NULL  
  The response's MIME content type.
* protected string $contentDisposition = NULL  
  The response's MIME content disposition.
* protected int $contentEncoding = NULL  
  See http\Env\Response::CONTENT_ENCODING_* constants.
* protected string $cacheControl = NULL  
  How the client should treat this response in regards to caching.
* protected string $etag = NULL  
  A custom ETag.
* protected int $lastModified = NULL  
  A "Last-Modified" time stamp.
* protected int $throttleDelay = NULL  
  Any throttling delay.
* protected int $throttleChunk = NULL  
  The chunk to send every $throttleDelay seconds.
* protected array $cookies = NULL  
  The response's cookies.
