# ext/request

This extension provides server-side request and response objects for PHP 7.3
and later.

These are *not* HTTP message objects proper. They are more like wrappers
for existing global PHP variables and functions.

This extension defines three classes in the global namespace:

- ServerRequest, composed of read-only copies of PHP superglobals and some
  other commonly-used values, with methods for adding application-specific
  request information in immutable fashion.

- ServerResponse, essentially a wrapper around (and buffer for) response-
  related PHP functions.

- ServerResponseSender, for sending a ServerResponse.

## ServerRequest

An object of public read-only properties representing the PHP request received
by the server. Use it in place of the `$_GET`, `$_POST`, etc. superglobals. It
provides:

- non-session superglobals as public read-only properties;

- other public read-only properties calculated from the superglobals (`$method`,
  `$headers`, `$content`, `$accept`, `$uploads`, etc.);

Note that ServerRequest can be extended to provide other userland functionality;
however, the public read-only properties cannot be modified or overridden.

### Instantiation

Instantiation of _ServerRequest_ is straightforward:

```php
$request = new ServerRequest($GLOBALS);
```

If you want to provide custom values to the object, pass an array that mimics
`$GLOBALS` to the constructor:

```php
$request = new ServerRequest([
    '_SERVER' => [
        'foo' => 'bar',
    ],
]);
```

### Properties

_ServerRequest_ has these public properties.

#### Superglobal-related

These properties are public, read-only, and cannot be modified or overridden.

- `?array $env`: A copy of `$_ENV`.
- `?array $files`: A copy of `$_FILES`.
- `?array $get`: A copy of `$_GET`.
- `?array $cookie`: A copy of `$_COOKIE`.
- `?array $post`: A copy of `$_POST`.
- `?array $server`: A copy of `$_SERVER`.
- `?array $uploads`: A copy of `$_FILES`, restructured to look more like `$_POST`.

#### HTTP-related

These properties are public, read-only, and cannot be modified or overridden.

- `?array $accept`: An array of arrays computed from `$_SERVER['HTTP_ACCEPT']`.
- `?array $acceptCharset`: An array of arrays computed from `$_SERVER['HTTP_ACCEPT_CHARSET']`.
- `?array $acceptEncoding`: An array of arrays computed from `$_SERVER['HTTP_ACCEPT_ENCODING']`.
- `?array $acceptLanguage`: An array of arrays computed from `$_SERVER['HTTP_ACCEPT_LANGUAGE']`.
- `?array $forwarded`: An array of arrays computed from `$_SERVER['HTTP_FORWARDED']`.
- `?array $forwardedFor`: An array computed from treating `$_SERVER['HTTP_X_FORWARDED_FOR']`
  as comma-separated values.
- `?string $forwardedHost`: The `$_SERVER['HTTP_X_FORWARDED_HOST']` value.
- `?string $forwardedProto`: The `$_SERVER['HTTP_X_FORWARDED_PROTO']` value.
- `?array $headers`: An array of key/value pairs computed from `$_SERVER` using all
  `HTTP_*` header keys, plus RFC 3875 headers not prefixed with `HTTP_`.
- `?string $method`: The `$_SERVER['REQUEST_METHOD']` value, or the
  `$_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE']` value when appropriate.
- `?string $requestedWith`: The value of `$_SERVER['HTTP_X_REQUESTED_WITH']`.

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

These properties are public, read-only, and cannot be modified or overridden.

- `string $content`: The value of `file_get_contents('php://input')`.
- `?string $contentCharset`: The `charset` parameter value of `$_SERVER['CONTENT_TYPE']`.
- `?string $contentLength`: The value of `$_SERVER['CONTENT_LENGTH']`.
- `?string $contentMd5`: The value of `$_SERVER['HTTP_CONTENT_MD5']`.
- `?string $contentType`: The value of `$_SERVER['CONTENT_TYPE']`, minus any parameters.

#### Authentication-related

These properties are public, read-only, and cannot be modified or overridden.

- `?array $authDigest`: An array of digest values computed from
  `$_SERVER['PHP_AUTH_DIGEST']`.
- `?string $authPw`: The value of `$_SERVER['PHP_AUTH_PW']`.
- `?string $authType`: The value of `$_SERVER['PHP_AUTH_TYPE']`.
- `?string $authUser`: The value of `$_SERVER['PHP_AUTH_USER']`.

### Methods

The _ServerRequest_ object has no public methods.

### Extending and Overriding

tl;dr: Be sure to call the parent constructor or the properties will not be set.

## ServerResponse

An object representing the PHP response to be sent from the server; use it in
place of the `header()`, `setcookie()`, `setrawcookie()`, etc. functions. It
provides:

- a retention space for version, code, headers, and cookies so they can be
  inspected before sending;

- mutability and extensibility.

## Instantiation

Instantation is straightforward:

```php
$response = new ServerResponse();
```

### Properties

_ServerResponse_ has no public properties.

### Methods

_ServerResponse_ has these public methods, all of which are declared `final`;
they may not be overridden.

#### HTTP Version

- `setVersion(string $version) : void`: Sets the HTTP version for the response (typically
  '1.0' or '1.1').

- `getVersion() : ?string`: Returns the HTTP version for the response.

#### Response Code

- `setCode(string $code) : void`: Sets the HTTP response code; a buffered equivalent of
  `http_response_code($code)`.

- `getCode() : ?long`: Gets the HTTP response code.

#### Headers

- `setHeader(string $label, string $value) : void`: Overwrites an HTTP header; a buffered equivalent
  of `header("$label: $value", true)`.

- `addHeader(string $label, string $value) : void`: Appends to an HTTP header, comma-separating it
  from the existing value; a buffered equivalent of
  `header("$label: $value", false)`.

- `getHeaders() : ?array`: Returns the array of headers to be sent.

The header field labels are retained internally in lower-case, and are
sent as lower-case. This is to
[comply with HTTP/2 requirements](https://tools.ietf.org/html/rfc7540#section-8.1.2);
while HTTP/1.x has no such requirement, lower-case is also recognized as valid.

#### Cookies

- `setCookie(...) : bool`: A buffered equivalent of [`setcookie()`](http://php.net/setcookie)
  with identical arguments.

- `setRawCookie(...) : bool`: A buffered equivalent of [`setrawcookie()`](http://php.net/setrawcookie)
  with identical arguments.

- `getCookies() : ?array`: Returns the array of cookies to be sent.

#### Header Callbacks

- `setHeaderCallbacks(array $callbacks) : void`: Sets an array of callbacks to be invoked
  just before headers are sent. It replaces any existing callbacks. This is similar to
  [`header_register_callback()`](https://secure.php.net/header_register_callback),
  except that *multiple* callbacks may be registered with the Response.

- `addHeaderCallback(callable $callback) : void`: Appends one callback to the current array of
  header callbacks.

- `getHeaderCallbacks() : ?array`: Returns the array of header callbacks.

The header callback signature should be `function (ServerResponse $response)`;
any return value is ignored.

#### Content

- `setContent(mixed $content) : void`: Sets the content of the response. This
  may be null, a string, resource, object, or anything else.

- `getContent() : mixed`: Returns the content of the response. This may be null,
  a string, resource, object, or anything else.

### Extending and Overriding

TBD

## ServerResponseSender

### Instantiation

Instantiation is straightforward:

```php
$sender = new ServerResponseSender();
```

### Properties

This class has no public properties.

### Methods

The primary public method is `send(ServerResponse $response) : void`. It ...

- invokes the header callbacks
- sends the status line (version and code) using `header()`
- sends the non-cookie headers using `header()`
- sends the cookie headers using `setcookie()` and `setrawcookie()`
- sends the content

If the response content is a resource, it is sent with `fpassthru()`. If the
content is a callable object or closure, it is invoked, and its return value (if
any) is `echo`ed. Otherwise, the content is `echo`ed (which calls `__toString()`
if the content is an object).
