# http\Env\Response http\Env\Response::setContentEncoding(int $content_encoding)

Enable support for "Accept-Encoding" requests with deflate or gzip.
The response will be compressed if the client indicates support and wishes that.

## Params:

* int $content_encoding  
  See http\Env\Response::CONTENT_ENCODING_* constants.

## Returns:

* http\Env\Response, self.

## Throws:

* http\Exception\InvalidArgumentException
