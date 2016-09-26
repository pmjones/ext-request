
# ext/request

This extension provides fundamental server-side request and response objects for PHP. These are *not* HTTP message objects proper. They are more like wrappers for existing global PHP variables and functions, with some limited additional convenience functionality.

This extension provides two classes:

- StdRequest, essentially a read-only struct composed of PHP superglobals and some other commonly-used values

- StdResponse, essentially a wrapper around (and buffer for) response-related PHP functions, with some additional convenience methods, and self-sending capability

## StdRequest

Goals:

- Provide a struct of non-session superglobals as read-only properties.

- Add other read-only properties calculated from the superglobals ($method, $headers, $content, etc.) to the struct.

- Only build things that don't require application input; e.g., no negotiation, but build acceptables for an application to work through.

- No methods, just properties (i.e., a struct).

- Extendable so users can add custom functionality.

### Instantiation

Instantiation of _StdRequest_ is straightforward:

```php
<?php
$request = new StdRequest();
?>
```

The _StdRequest_ object builds itself from the PHP superglobals. If you want to provide custom values in place of the superglobals, pass an array that mimics `$GLOBALS` to the constructor:

```php
<?php
$request = new StdRequest([
    '_SERVER' => [
        'foo' => 'bar',
    ],
]);
?>
```

If a superglobal is represented in the array of custom values, it will be used instead of the real superglobal. If it is not represented in the array, _StdRequest_ will use the real superglobal.

### Properties

_StdRequest_ has these public properties, all of which are read-only.

#### Superglobal-related

- `$env`: A copy of `$_ENV`.
- `$files`: A copy of `$_FILES`.
- `$get`: A copy of `$_GET`.
- `$cookie`: A copy of `$_COOKIE`.
- `$post`: A copy of `$_POST`.
- `$server`: A copy of `$_SERVER`.
- `$uploads`: A copy of `$_FILES`, restructured to look more like `$_POST`.

#### HTTP-related

- `$acceptCharset`: An array computed from `$_SERVER['HTTP_ACCEPT_CHARSET']`.
- `$acceptEncoding`: An array computed from `$_SERVER['HTTP_ACCEPT_ENCODING']`.
- `$acceptLanguage`: An array computed from `$_SERVER['HTTP_ACCEPT_LANGUAGE']`.
- `$acceptMedia`: An array computed from `$_SERVER['HTTP_ACCEPT']`.
- `$headers`: An array of all `HTTP_*` header keys from `$_SERVER`, plus RFC 3875 headers not prefixed with `HTTP_`
- `$method`: The `$_SERVER['REQUEST_METHOD']` value, or the `$_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE']` when appropriate.
- `$secure`: A boolean indicating if this is an HTTPS or port 443 request.
- `$url`: The result of [`parse_url`](http://php.net/parse_url) as built from the `$_SERVER` keys `HTTP_HOST`/`SERVER_NAME`, `SERVER_PORT`, and `REQUEST_URI`.
- `$xhr`: A boolean indicating if this is an XmlHttpRequest.

> N.b.: Each element of the `$accept*` arrays has these sub-array keys:
>
> ```
> 'value' => The "main" value of the accept specifier
> 'quality' => The 'q=' parameter value
> 'params' => A key-value array of all other parameters
> ```
>
> In addition, each `$acceptLanguage` array element has two additional sub-array keys: `'type'` and `'subtype'`.
>
> The `$accept*` array elements are sorted by highest `q` value to lowest.

#### Content-related

- `$content`: The value of `file_get_contents('php://input')`.
- `$contentCharset`: The `charset` parameter value of `$_SERVER['CONTENT_TYPE']`.
- `$contentLength`: The value of `$_SERVER['CONTENT_LENGTH']`.
- `$contentMd5`: The value of `$_SERVER['HTTP_CONTENT_MD5']`.
- `$contentType`: The value of `$_SERVER['CONTENT_TYPE']`, minus any parameters.

#### Authentication-related

- `$authDigest`: An array of digest values computed from `$_SERVER['PHP_AUTH_DIGEST']`.
- `$authPw`: The value of `$_SERVER['PHP_AUTH_PW']`.
- `$authType`: The value of `$_SERVER['PHP_AUTH_TYPE']`.
- `$authUser`: The value of `$_SERVER['PHP_AUTH_USER']`.


### Methods

The _StdRequest_ object has no public methods.

## StdResponse

Goals:

- Light wrapper around PHP functions (with similar lack of validation)
- Buffer for headers and cookies
- Helper for HTTP date
- Helper for comma- and semicolon-separated header values
- Minimalist support for sending a file, and sending json
- Self-sendable
- Mutable, extendable

## Instantiation

Instantation is straightforward:

```php
<?php
$response = new StdResponse();
?>
```

### Properties

_StdResponse_ has no public properties.

### Methods

_StdResponse_ has these public methods.

#### HTTP Version

- `setVersion($version)`: Sets the HTTP version for the response (typically '1.0' or '1.1').

- `getVersion()`: Returns the HTTP version for the response.

#### Status Code

- `setStatus($status)`: Sets the HTTP response code; a buffered equivalent of `http_response_code($status)`.

- `getStatus()`: Gets the HTTP response code.

#### Headers

- `setHeader($label, $value)`: Overwrites an HTTP header; a buffered equivalent of `header("$label: $value", true)`.

- `addHeader($label, $value)`: Adds an HTTP header; a buffered equivalent of `header("$label: $value", false)`.

- `date($date)`: Returns a RFC 1123 formatted date. The `$date` argument can be any recognizable date-time string, or a _DateTime_ object.

- `getHeaders()`: Returns the array of headers to be sent.

> N.b.: The `$value` in a `setHeader()` or `addHeader()` call may be an array, in which case it will be converted to a semicolon-separated and/or comma-separated value string. Example TBD.

#### Cookies

- `setCookie(...)`: A buffered equivalent of [`setcookie()`](http://php.net/setcookie) with identical arguments.

- `setRawCookie(...)`: A buffered equivalent of [`setrawcookie()`](http://php.net/setrawcookie) with identical arguments.

- `getCookies()`: Returns the array of cookies to be sent.

#### Content

- `setContent($content)`: Sets the content of the response. This can be a string or resource (or anything else).

- `setContentJson($value, $options = 0, $depth = 512)`: A convenience method to `json_encode($value)` as the response content, and set a `Content-Type: application/json` header.

- `setContentResource($fh, $disposition, array $params = [])`: A convenience method to set the content to a resource (typically a file handle), as well as set the headers for `Content-Type: application/octet-stream`, `Content-Transfer-Encoding: binary`, and `Content-Disposition: $disposition`.  (The `$params` key-value array is added as parameters on the disposition.)

- `setDownload($fh, $name, array $params = [])`: A convenience method to set the content to a resource (typically a file handle), to be downloaded as `$name`, with `Content-Disposition: attachment`.   (The `$params` key-value array is added as parameters on the disposition.)

- `setDownloadInline($fh, $name, array $params = [])`: A convenience method to set the content to a resource (typically a file handle), to be downloaded as `$name`, with `Content-Disposition: inline`.   (The `$params` key-value array is added as parameters on the disposition.)

- `getContent()`: Returns the content of the response; this may be a string or resource (or anything else).

#### Sending

- `send()`: This sends the response version, status, headers, and cookies using native PHP functions `header()`, `setcookie()`, and `setrawcookie()`. Then, if the response content is a resource, it is sent with `fpassthru()`; if a callable object or closure, it is invoked; otherwise, the content is `echo`ed.
