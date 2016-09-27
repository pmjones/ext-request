# static mixed http\Env::getResponseHeader([string $header_name])

Get one or all HTTP response headers to be sent.

## Parameters:

* Optional string $header_name  
  The name of the response header to retrieve.
  
## Returns:

* string, the compound value of the response header to send
* NULL, if the header was not found
* array, of all response headers, if $header_name was not specified
