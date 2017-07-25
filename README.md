# ext/request

This extension provides server-side request and response objects for PHP.
These are *not* HTTP message objects proper. They are more like wrappers
for existing global PHP variables and functions, with some limited
additional convenience functionality.

This extension defines two classes in the global namespace:

- ServerRequest, composed of read-only copies of PHP superglobals and some
  other commonly-used values, with methods for adding application-specific
  request information in immutable fashion.

- ServerResponse, essentially a wrapper around (and buffer for) response-
  related PHP functions, with some additional convenience methods, and self-
  sending capability.

## ServerRequest

An object representing the PHP request received by the server; use it in place
of the `$_GET`, `$_POST`, etc. superglobals. It provides:

- non-session superglobals as read-only properties;

- other read-only properties calculated from the superglobals (`$method`,
  `$headers`, `$content`, `$accept`, `$uploads`, etc.);

- immutable retention of application-specific information, such as parsed body
  input and routing parameters;

- extensibility.

### Instantiation

Instantiation of _ServerRequest_ is straightforward:

```php
<?php
$request = new ServerRequest();
?>
```

The _ServerRequest_ object builds itself from the PHP superglobals. If you want
to provide custom values in place of the superglobals, pass an array that mimics
`$GLOBALS` to the constructor:

```php
<?php
$request = new ServerRequest([
    '_SERVER' => [
        'foo' => 'bar',
    ],
]);
?>
```

If a superglobal is represented in the array of custom values, it will be used
instead of the real superglobal. If it is not represented in the array,
_ServerRequest_ will use the real superglobal.

### Properties

_ServerRequest_ has these public properties.

#### Superglobal-related

These properties are read-only and cannot be modified.

- `$env`: A copy of `$_ENV`.
- `$files`: A copy of `$_FILES`.
- `$get`: A copy of `$_GET`.
- `$cookie`: A copy of `$_COOKIE`.
- `$post`: A copy of `$_POST`.
- `$server`: A copy of `$_SERVER`.
- `$uploads`: A copy of `$_FILES`, restructured to look more like `$_POST`.

#### HTTP-related

These properties are read-only and cannot be modified.

- `$accept`: An array of arrays computed from `$_SERVER['HTTP_ACCEPT']`.
- `$acceptCharset`: An array of arrays computed from `$_SERVER['HTTP_ACCEPT_CHARSET']`.
- `$acceptEncoding`: An array of arrays computed from `$_SERVER['HTTP_ACCEPT_ENCODING']`.
- `$acceptLanguage`: An array of arrays computed from `$_SERVER['HTTP_ACCEPT_LANGUAGE']`.
- `$forwarded`: An array of arrays computed from `$_SERVER['HTTP_FORWARDED']`.
- `$forwardedFor`: An array computed from treating `$_SERVER['HTTP_X_FORWARDED_FOR']`
  as comma-separated values.
- `$forwardedHost`: The `$_SERVER['HTTP_X_FORWARDED_HOST']` value.
- `$forwardedProto`: The `$_SERVER['HTTP_X_FORWARDED_PROTO']` value.
- `$headers`: An array of key/value pairs computed from `$_SERVER` using all
  `HTTP_*` header keys, plus RFC 3875 headers not prefixed with `HTTP_`.
- `$method`: The `$_SERVER['REQUEST_METHOD']` value, or the
  `$_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE']` value when appropriate.
- `$xhr`: A boolean indicating if this is an XmlHttpRequest.

##### The `$accept*` Arrays

Each element of the `$accept*` arrays is an array with these keys:

```
'value' => The "main" value of the accept specifier
'quality' => The 'q=' parameter value
'params' => A key-value array of all other parameters
```

In addition, each `$acceptLanguage` array element has two additional keys:
`'type'` and `'subtype'`.

The `$accept*` array elements are sorted by highest `q` value to lowest.

##### The `$forwarded` Array

> Cf. the [Forwarded HTTP Extension](https://tools.ietf.org/html/rfc7239)

Each element of the `$forwarded` array is an array composed of one or more of
the following keys:

```
'by' => The interface where the request came in to the proxy server.
'for' => Discloses information about the client that initiated the request.
'host' =>  The original value of the Host header field.
'proto' => The value of the used protocol type.
```

#### Content-related

These properties are read-only and cannot be modified.

- `$content`: The value of `file_get_contents('php://input')`.
- `$contentCharset`: The `charset` parameter value of `$_SERVER['CONTENT_TYPE']`.
- `$contentLength`: The value of `$_SERVER['CONTENT_LENGTH']`.
- `$contentMd5`: The value of `$_SERVER['HTTP_CONTENT_MD5']`.
- `$contentType`: The value of `$_SERVER['CONTENT_TYPE']`, minus any parameters.

#### Authentication-related

These properties are read-only and cannot be modified.

- `$authDigest`: An array of digest values computed from
  `$_SERVER['PHP_AUTH_DIGEST']`.
- `$authPw`: The value of `$_SERVER['PHP_AUTH_PW']`.
- `$authType`: The value of `$_SERVER['PHP_AUTH_TYPE']`.
- `$authUser`: The value of `$_SERVER['PHP_AUTH_USER']`.

#### Application-related

These property values are "immutable" rather than read-only. That is, they can
changed using the methods below, but the changed values are available only on a
new instance of the _ServerRequest_ as returned by the method.

- `$input`: Typically the parsed body content of the request, such as from
  `json_decode()`.
- `$params`: Typically path-info or routing parameters.
- `$url`: The result of [`parse_url`](http://php.net/parse_url) as built from
  the `$_SERVER` keys `HTTPS`, `HTTP_HOST`/`SERVER_NAME`, `SERVER_PORT`, and
  `REQUEST_URI`.

### Methods

The _ServerRequest_ object has these public methods.

#### `withInput(mixed $input)`

Sets the `$input` value on a clone of the called _ServerRequest_ instance.

For example:

```php
<?php
$request = new ServerRequest();
if ($request->contentType == 'application/json') {
    $input = json_decode($request->content, true);
    $request = $request->withInput($input);
}
?>
```

Note that this method returns a clone of the _ServerRequest_ instance with the
new property value. It does not modify the property value on the called
instance.

The value may be null, scalar, or array. Arrays are recursively checked to make
sure they contain only null, scalar, or array values; this is to preserve
immutability of the value.

#### `withParam(mixed $key, mixed $val)`

Sets the value of one `$params` key on a clone of the called _ServerRequest_
instance.

For example:

```php
<?php
$request = new ServerRequest();
var_dump($request->params); // []

$request = $request->withParam('foo', 'bar');
var_dump($request->params); // ['foo' => 'bar']
?>
```

Note that this method returns a clone of the _ServerRequest_ instance with the
new property value. It does not modify the property value on the called
instance.

The value may be null, scalar, or array. Arrays are recursively checked to make
sure they contain only null, scalar, or array values; this is to preserve
immutability of the value.

#### `withParams(array $params)`

Sets the `$params` value on a clone of the called _ServerRequest_ instance.

For example:

```php
<?php
$request = new ServerRequest();
var_dump($request->params); // []

$request = $request->withParams(['foo' => 'bar']);
var_dump($request->params); // ['foo' => 'bar']
?>
```

Note that this method returns a clone of the _ServerRequest_ instance with the
new property value. It does not modify the property value on the called
instance.

The value may be null, scalar, or array. Arrays are recursively checked to make
sure they contain only null, scalar, or array values; this is to preserve
immutability of the value.

#### `withoutParam(mixed $key)`

Unsets a single `$params` key on a clone of the called _ServerRequest_ instance.

For example:

```php
<?php
$request = new ServerRequest();
$request = $request->withParams(['foo' => 'bar', 'baz' => 'dib']);
var_dump($request->params); // ['foo' => 'bar', 'baz' => 'dib']

$request = $request->withoutParam('baz');
var_dump($request->params); // ['foo' => 'bar']
?>
```

Note that this method returns a clone of the _ServerRequest_ instance with the
new property value. It does not modify the property value on the called
instance.

#### `withoutParams([array $keys = null])`

Unsets multiple `$params` keys on a clone of the called _ServerRequest_
instance.

For example:

```php
<?php
$request = new ServerRequest();
$request = $request->withParams([
    'foo' => 'bar',
    'baz' => 'dib',
    'zim' => 'gir',
]);
var_dump($request->params); // ['foo' => 'bar', 'baz' => 'dib', 'zim' => 'gir']

$request = $request->withoutParams(['baz', 'zim']);
var_dump($request->params); // ['foo' => 'bar']
?>
```

Calling `withoutParams()` with no arguments removes all `$params` on a clone of
the called _ServerRequest_ instance:

```php
<?php
$request = new ServerRequest();
$request = $request->withParams([
    'foo' => 'bar',
    'baz' => 'dib',
    'zim' => 'gir',
]);
var_dump($request->params); // ['foo' => 'bar', 'baz' => 'dib', 'zim' => 'gir']

$request = $request->withoutParams();
var_dump($request->params); // []
?>
```

Note that this method returns a clone of the _ServerRequest_ instance with the
new property value. It does not modify the property value on the called
instance.

#### `withUrl(array $url)`

Sets the value of the `$url` array on a clone of the called _ServerRequest_
instance.

For example:

```php
<?php
$request = new ServerRequest();

$request = $request->withUrl([
    'scheme' => 'https',
    'host' => 'example.com',
    'port' => 8080,
    'path' => '/foo/bar',
    'query' => 'baz=dib',
]);

var_dump($request->url);
/* [
    'scheme' => 'https',
    'host' => 'example.com',
    'port' => 8080,
    'user' => null,
    'pass' => null,
    'path' => '/foo/bar',
    'query' => 'baz=dib',
    'fragment' => null,
] */
?>
```

Note that this method returns a clone of the _ServerRequest_ instance with the
new property value. It does not modify the property value on the called
instance.


## ServerResponse

An object representing the PHP response to be sent from the server; use it in
place of the `header()`, `setcookie()`, `setrawcookie()`, etc. functions. It
provides:

- a retention space for headers, cookies, and status, so they can be inspected
  before sending;
- a helper method for building HTTP date strings;
- a helper for building header comma- and semicolon-separated strings for headers;
- support for specifying content as a download (with appropriate headers);
- support for specifying content as JSON (with appropriate headers);
- self-sending capability;
- mutability and extensibility.

## Instantiation

Instantation is straightforward:

```php
<?php
$response = new ServerResponse();
?>
```

### Properties

_ServerResponse_ has no public properties.

### Methods

_ServerResponse_ has these public methods.

#### HTTP Version

- `setVersion($version)`: Sets the HTTP version for the response (typically
  '1.0' or '1.1').

- `getVersion()`: Returns the HTTP version for the response.

#### Status Code

- `setStatus($status)`: Sets the HTTP response code; a buffered equivalent of
  `http_response_code($status)`.

- `getStatus()`: Gets the HTTP response code.

#### Headers

- `setHeader($label, $value)`: Overwrites an HTTP header; a buffered equivalent
  of `header("$label: $value", true)`.

- `addHeader($label, $value)`: Appends to an HTTP header, comma-separating it
  from the existing value; a buffered equivalent of
  `header("$label: $value", false)`.

- `getHeader($label)`: Returns the value for a particular header.

- `getHeaders()`: Returns the array of headers to be sent.

- `date($date)`: Returns a RFC 1123 formatted date. The `$date` argument can be
  any recognizable date-time string, or a _DateTime_ object.

Notes:

The `$value` in a `setHeader()` or `addHeader()` call may be an array, in which
case it will be converted to a comma-separated and/or semicolon-separated value
string. For example:

```php
<?php
$response = new ServerResponse();

$response->setHeader('Cache-Control', [
    'public',
    'max-age' => '123',
    's-maxage' => '456',
    'no-cache',
]); // Cache-Control: public, max-age=123, s-maxage=456, no-cache

$response->setHeader('content-type', [
    'text/plain' => [
        'charset' => 'utf-8'
    ],
]); // content-type: text/plain;charset=utf-8

$response->setHeader('X-Whatever', [
    'foo',
    'bar' => [
        'baz' => 'dib',
        'zim',
        'gir' => 'irk',
    ],
    'qux' => 'quux',
]); // X-Whatever: foo, bar;baz=dib;zim;gir=irk, qux=quux
```

Finally, the header field labels are retained internally in lower-case, and are
sent as lower-case. This is to
[comply with HTTP/2 requirements](https://tools.ietf.org/html/rfc7540#section-8.1.2);
while HTTP/1.x has no such requirement, lower-case is also recognized as valid.

#### Cookies

- `setCookie(...)`: A buffered equivalent of [`setcookie()`](http://php.net/setcookie)
  with identical arguments.

- `setRawCookie(...)`: A buffered equivalent of [`setrawcookie()`](http://php.net/setrawcookie)
  with identical arguments.

- `getCookies()`: Returns the array of cookies to be sent.

#### Header Callbacks

- `setHeaderCallbacks($callbacks)`: Sets an array of callbacks to be invoked
  just before headers are sent. It replaces any existing callbacks. This is similar to
  [`header_register_callback()`](https://secure.php.net/header_register_callback),
  except that *multiple* callbacks may be registered with the Response.

- `addHeaderCallback($callback)`: Appends one callback to the current array of
  header callbacks.

- `getHeaderCallbacks()`: Returns the array of header callbacks.

#### Content

- `setContent($content)`: Sets the content of the response. This may be a
  string, resource, object, or anything else.

- `setContentJson($value, $options = 0, $depth = 512)`: A convenience method to
  `json_encode($value)` as the response content, and set a
  `content-type: application/json` header.

- `setContentDownload($fh, $name, $disposition = 'attachment', array $params = [])`:
  A convenience method to set the content to a resource (typically a file handle)
  for download to the client. This sets the headers
  `content-type: application/octet-stream`, `content-transfer-encoding: binary`,
  and `content-disposition: $disposition;filename="$name"`.  (The `$params`
  key-value array are included as parameters on the disposition.)

- `getContent()`: Returns the content of the response. This may be a string,
  resource, object, or anything else.

#### Sending

- `send()`: This sends the response version, status, headers, and cookies using
  native PHP functions `header()`, `setcookie()`, and `setrawcookie()`. Then,
  if the response content is a resource, it is sent with `fpassthru()`; if a
  callable object or closure, it is invoked; otherwise, the content is `echo`ed
  (which calls `__toString()` if the content is an object).
