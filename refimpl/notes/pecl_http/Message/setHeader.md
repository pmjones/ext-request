# http\Message http\Message::setHeader(string $header[, mixed $value = NULL])

Set a single header.
See http\Message::getHeader() and http\Message::addHeader().

> ***NOTE:***  
> Prior to v2.5.6/v3.1.0 headers with the same name were merged into a single
> header with values concatenated by comma.

## Params:

* string $header  
  The header's name.
* Optional mixed $value = NULL  
  The header's value. Removes the header if NULL.

## Returns:

* http\Message, self.

## Changelog:

0. v2.5.6, v3.1.0
	* Multiple headers with the same name are kept separate instead of merged together.
