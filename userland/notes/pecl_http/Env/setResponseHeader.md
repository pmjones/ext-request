# static bool http\Env::setResponseHeader(string $header_name, [mixed $header_value = NULL[, int $response_code = 0[, bool $replace = true]]]

Set a response header, either replacing a prior set header, or appending the new header value, depending on $replace.

If no $header_value is specified, or $header_value is NULL, then a previously set header with the same key will be deleted from the list.

If $response_code is not 0, the response status code is updated accordingly.

## Parameters:

* string $header_name, the name of the response header.
* Optional mixed $header_value, the header value.
* Optional int $response_code, any HTTP response status code to set.
* Optional bool $replace, whether to replace a previously set response header with the same name.

## Returns:

* bool Success.
