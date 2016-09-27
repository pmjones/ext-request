# static mixed http\Env::getRequestHeader([string $header_name])

Retrieve one or all headers of the current HTTP request.

## Parameters:

* Optional string $header_name  
  The key of a header to retrieve.
  
## Returns:  

* NULL, if $header_name was not found
* string, the compound header when $header_name was found
* array of all headers if $header_name was not specified
