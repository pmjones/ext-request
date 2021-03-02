# ext/request

This extension provides server-side request and response objects for PHP.

These are *not* HTTP message objects proper. They are more like wrappers
for existing global PHP variables and functions.

This extension defines four classes and one interface in the global namespace:

- _SapiRequest_, composed of read-only copies of PHP superglobals and some other commonly-used values.

- _SapiUpload_, a value-object style descriptor of each uploaded file.

- _SapiResponse_ and _SapiResponseInterface_, essentially a wrapper around (and buffer for) response-related PHP functions.

- _SapiResponseSender_, for sending a _SapiResponse_.

## Installing

### Via PECL

In most cases, it is easiest to install via PECL:

```
# pecl install request
```

Alternatively, clone or otherwise download this repository; then, in the
repository directory, issue these commands to compile and install the extension:

```
$ phpize
$ ./configure
$ make
# make install
```

Finally, enable the `request.so` extension in your `php.ini` file, and restart your web server.

## _SapiRequest_

An object of public read-only properties representing the PHP request received
by the server. Use it in place of the `$_GET`, `$_POST`, etc. superglobals. It
provides:

- non-session superglobals as public, immutable, read-only properties;

- other public, immutable, read-only properties calculated from the superglobals
  (`$method`, `$headers`, `$accept`, `$uploads`, etc.);

Note that _SapiRequest_ can be extended to provide other userland functionality;
however, the public properties cannot be modified or overridden.

### Instantiation

Instantiation of _SapiRequest_ is straightforward:

```php
$request = new SapiRequest($GLOBALS);
```

If you want to provide custom superglobal values to the object, pass an array
that mimics `$GLOBALS` to the constructor:

```php
$request = new SapiRequest([
    '_SERVER' => [
        'foo' => 'bar',
    ],
]);
```

By default, the `$content` property will read from `php://input` on-the-fly. If
you want to provide a custom `$content` string instead, pass it as the second
constructor argument:

```php
$request = new SapiRequest(
  $GLOBALS,
  'custom-php-input-string'
);
```

> N.b.: It is up to you to make sure the various content-related header values in
`$GLOBALS` match the custom `$content` string.

### Properties

_SapiRequest_ has these public properties.

#### Superglobal-related

These properties are public, immutable, read-only, and cannot be modified or overridden.

- `?array $cookie`: A copy of `$_COOKIE`.
- `?array $files`: A copy of `$_FILES`.
- `?array $input`: A copy of `$_POST`.
- `?array $query`: A copy of `$_GET`.
- `?array $server`: A copy of `$_SERVER`.
- `?array $uploads`: A copy of `$_FILES`, restructured to look more like `$_POST`;
  instead of array descriptors, the elements are instances of _SapiUpload_.

##### The `$uploads` array

The _SapiRequest_ `$files` property is an identical copy of `$_FILES`. Normally,
`$_FILES` looks like this with multi-file uploads:

```php
// $_FILES ...
[
    'images' => [
        'name' => [
            0 => 'image1.png',
            1 => 'image2.gif',
            2 => 'image3.jpg',
        ],
        'type' => [
            0 => 'image/png',
            1 => 'image/gif',
            2 => 'image/jpeg',
        ],
        'tmp_name' [
            0 => '/tmp/path/phpABCDEF',
            1 => '/tmp/path/phpGHIJKL',
            2 => '/tmp/path/phpMNOPQR',
        ],
        'error' => [
            0 => 0,
            1 => 0,
            2 => 0,
        ],
        'size' =>[
            0 => 123456,
            1 => 234567,
            2 => 345678,
        ],
    ],
];
```

However, that structure is not at all what we expect when we are used to
working with `$_POST`. Therefore, the _SapiRequest_ `$uploads` property
restructures the data in `$_FILES` to look more like `$_POST` does ...

```php
// $request->uploads ...
[
    'images' => [
        0 => [
            'name' => 'image1.png',
            'type' => 'image/png',
            'tmp_name' => '/tmp/path/phpABCDEF',
            'error' => 0,
            'size' => 123456,
        ],
        1 => [
            'name' => 'image2.gif',
            'type' => 'image/gif',
            'tmp_name' => '/tmp/path/phpGHIJKL',
            'error' => 0,
            'size' => 234567,
        ],
        2 => [
            'name' => 'image3.jpg',
            'type' => 'image/jpeg',
            'tmp_name' => '/tmp/path/phpMNOPQR',
            'error' => 0,
            'size' => 345678,
        ],
    ],
];
```

... and then replaces each array-based descriptor with a _SapiUpload_ instance.

#### HTTP-related

These properties are public, immutable, read-only, and cannot be modified or overridden.

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
  `HTTP_*` header keys, plus RFC 3875 headers not prefixed with `HTTP_`. Note that
  the header keys are retained in lower-case. This is to
  [comply with HTTP/2 requirements](https://tools.ietf.org/html/rfc7540#section-8.1.2);
  while HTTP/1.x has no such requirement, lower-case is also recognized as valid.
- `?string $method`: The `$_SERVER['REQUEST_METHOD']` value, or the
  `$_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE']` value when appropriate.
- `?array $url`: The result from applying `parse_url()` to a a URL string
  constructed from various `$_SERVER` elements.

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

##### The `$url` Array

_SapiRequest_ attempts to build a string of the full request URL of the using
the following:

- If `$_SERVER['HTTPS'] === 'on'`, the scheme is 'https'; otherwise, it is
  'http'.
- If `$_SERVER['HTTP_HOST']` is present, it is used as the host name; otherwise,
  `$_SERVER['SERVER_NAME']` is used.
- If a port number is present on the host name, it is used as the port;
  otherwise, `$_SERVER['SERVER_PORT']` is used.
- `$_SERVER['REQUEST_URI']` is used for the path and query string.

_SapiRequest_ then passes that string through [`parse_url()`](https://www.php.net/parse_url)
and retains the resulting array as `$url`.

You can then retrieve the array elements using the `PHP_URL_*` constants:

```php
$scheme = $request->url[PHP_URL_SCHEME];
$host = $request->url[PHP_URL_HOST];
$port = $request->url[PHP_URL_PORT];
$path = $request->url[PHP_URL_PATH];
$queryString = $request->url[PHP_URL_QUERY];
```

If `parse_url()` fails, the `$url` property will remain `null`.

#### Content-related

These properties are public, immutable, read-only, and cannot be modified or overridden.

- `string $content`: The value of `file_get_contents('php://input')`, or the
  custom content string provided at construction time.
- `?string $contentCharset`: The `charset` parameter value of `$_SERVER['CONTENT_TYPE']`.
- `?int $contentLength`: The value of `$_SERVER['CONTENT_LENGTH']`.
- `?string $contentMd5`: The value of `$_SERVER['HTTP_CONTENT_MD5']`.
- `?string $contentType`: The value of `$_SERVER['CONTENT_TYPE']`, minus any parameters.

#### Authentication-related

These properties are public, immutable, read-only, and cannot be modified or overridden.

- `?array $authDigest`: An array of digest values computed from
  `$_SERVER['PHP_AUTH_DIGEST']`.
- `?string $authPw`: The value of `$_SERVER['PHP_AUTH_PW']`.
- `?string $authType`: The value of `$_SERVER['PHP_AUTH_TYPE']`.
- `?string $authUser`: The value of `$_SERVER['PHP_AUTH_USER']`.

### Methods

The _SapiRequest_ object has no public methods other than its constructor:

- `__construct(array $globals, [?string $content = null])`

### Extending and Overriding

**Although it is easy and convenient to extend this class, the authors recommend
decoration and composition over extension in all but the most trivial of cases.**

_SapiRequest_ has a constructor. Child classes overriding `__construct()`
should be sure to call `parent::__construct()`, or else the public read-only
properties will not be set (defaulting to `null` in all cases).

The public read-only properties cannot be overridden; however, child classes may
add new properties as desired.

_SapiRequest_ has no methods; child classes may add methods as desired, and
_SapiRequest_ does not anticipate adding new methods of its own.

## _SapiUpload_

A read-only object describing an individual file upload.

### Properties

_SapiUpload_ has these public properties; they are immutable, read-only,
and cannot be modified or overridden:

- `?string $name`: The original name of the file on the client machine.

- `?string $type`: The mime type of the file, if the client provided this
  information.

- `?int $size`: The size, in bytes, of the uploaded file.

- `?string $tmpName`: The temporary filename of the file in which the uploaded
  file was stored on the server.

- `?int $error`: The [error code](https://www.php.net/manual/en/features.file-upload.errors.php)
  associated with this file upload.

### Methods

_SapiUpload_ has these public methods:

- `__construct(?string $name, ?string $type, ?int $size, ?string $tmpName, ?int $error)`:
  The constructor; once constructed, it cannot be constructed again.

- `move(string $destination) : bool`: The equivalent of
  [`move_uploaded_file`](https://www.php.net/move_uploaded_file).

## _SapiResponse_

A mutable object representing the PHP response to be sent from the server; use
it in place of the `header()`, `setcookie()`, `setrawcookie()`, etc. functions.
It provides a retention space for the HTTP response version, code, headers,
cookies, and content, so they can be inspected before sending.

Note that _SapiResponse_ can be extended to provide other userland
functionality. However, its public methods are final; they cannot be modified or
overridden.

## Instantiation

Instantation is straightforward:

```php
$response = new SapiResponse();
```

### Properties

_SapiResponse_ has no public properties.

### Methods

_SapiResponse_ implements _SapiResponseInterface_, which has these public
methods; all of them are declared `final` and so may not be overridden.

#### Protocol Version

- `setVersion(?string $version) : SapiResponseInterface`: Sets the protocol
  version for the response (typically '1.0', '1.1', or '2').

- `getVersion() : ?string`: Returns the protocol version for the response.

#### Status Code

- `setCode(?int $code) : SapiResponseInterface`: Sets the status code for the response; a buffered equivalent of
  `http_response_code($code)`.

- `getCode() : ?int`: Gets the status code for the response.

#### Headers

- `setHeader(string $label, string $value) : SapiResponseInterface`: Overwrites an HTTP header; a
  buffered equivalent of `header("$label: $value", true)`.

- `addHeader(string $label, string $value) : SapiResponseInterface`: Appends to an HTTP header,
  comma-separating it from the existing value; a buffered equivalent of
  `header("$label: $value", false)`.

- `unsetHeader(string $label) : SapiResponseInterface`: Removes a header from the buffer.

- `unsetHeaders() : SapiResponseInterface`: Removes all headers from the buffer.

- `getHeaders() : ?array`: Returns the array of headers to be sent.

- `getHeader(string $label) : ?string`: Returns a header from the buffer.

- `hasHeader(string $label) : bool`: Returns true if a header exists in buffer.

The header field labels are retained internally in lower-case, and are sent as
lower-case. This is to [comply with HTTP/2 requirements](https://tools.ietf.org/html/rfc7540#section-8.1.2);
while HTTP/1.x has no such requirement, lower-case is also recognized as valid.

#### Cookies

- `setCookie(...) : SapiResponseInterface`: A buffered equivalent of
  [`setcookie()`](http://php.net/setcookie) with identical arguments.

- `setRawCookie(...) : SapiResponseInterface`: A buffered equivalent of
  [`setrawcookie()`](http://php.net/setrawcookie) with identical arguments.

- `unsetCookie(string $name) : SapiResponseInterface`: Removes a cookie from the buffer.

- `unsetCookies() : SapiResponseInterface`: Removes all cookies from the buffer.

- `getCookies() : ?array`: Returns the array of cookies to be sent.

- `getCookie(string $name) : ?array`: Returns a cookie from the buffer.

- `hasCookie(string $name) : bool`: Returns true if a cookie exists in buffer.

#### Header Callbacks

- `setHeaderCallbacks(array $callbacks) : SapiResponseInterface`: Sets an array of callbacks to
  be invoked just before headers are sent. It replaces any existing callbacks.
  This is similar to [`header_register_callback()`](https://secure.php.net/header_register_callback),
  except that *multiple* callbacks may be registered with the Response.

- `addHeaderCallback(callable $callback) : SapiResponseInterface`: Appends one callback to the
  current array of header callbacks.

- `getHeaderCallbacks() : ?array`: Returns the array of header callbacks.

The header callback signature should be `function (SapiResponseInterface $response)`;
any return value is ignored.

#### Content

- `setContent(mixed $content) : SapiResponseInterface`: Sets the content of the response. This
  may be null, a string, resource, object, or anything else.

- `getContent() : mixed`: Returns the content of the response. This may be null,
  a string, resource, object, or anything else.

### Extending and Overriding

**Although it is easy and convenient to extend this class, the authors recommend
decoration and composition over extension in all but the most trivial of cases.**

_SapiResponse_ is constructorless, which means you can add any constructor you
like and not have to call a parent constructor.

The properties on _SapiResponse_ are private, which means you may not access
them, except through the existing _SapiResponse_ methods.

The methods on _SapiResponse_ are public **and final**, which means you cannot
extend or override them in child classes. This keeps their behavior consistent.

However, the class itself is **not** final, which means you can add any other
properties and methods you like.

The combination of a non-final class with private properties and public final
methods keeps _SapiResponse_ open for extension, but closed for modification.

## _SapiResponseSender_

An object to send a _SapiResponse_.

Note that _SapiResponseSender_ methods can be extended and overridden.

### Instantiation

Instantiation is straightforward:

```php
$sender = new SapiResponseSender();
```

### Properties

This class has no properties of any kind.

### Methods

_SapiResponseSender_ has these public methods:

- `send(SapiResponseInterface $response) : void`: Calls the following methods in order;
  that is: runHeaderCallbacks(), sendStatus(), sendHeaders(), sendCookies(), and
  sendContent().

- `runHeaderCallbacks(SapiResponseInterface $response) : void`: Invokes each callback
  returned by SapiResponse::getHeaderCallbacks().

- `sendStatus(SapiResponseInterface $response) : void`: Sends the HTTP status line
  using header(). The line is composed of SapiResponse::getVersion() and
  SapiResponse::getCode(). If the version is `null` it defaults to `1.1`;
  if the code is null is defaults to `200`.

- `sendHeaders(SapiResponseInterface $response) : void`: Sends each header returned
  by SapiResponse::getHeaders() using header().

- `sendCookies(SapiResponseInterface $response) : void`: Sends each cookie returned
  by SapiResponse::getCookies() using setcookie() or setrawcookie().

- `sendContent(SapiResponseInterface $response) : void`: Sends the content returned
  by SapiResponse::getContent().

    - If the content is a resource, it is sent using `rewind()` and then
      `fpassthru()`; there is no further handling thereafter.

    - If the content is a callable object or closure, it is invoked, and
      its return value (if any) is passed along to be handled by the next step.

    - If the content or returned value is iterable, it is `foreach()`-ed
      through, and each value is echoed as a string; note that object values
      will be cast to string at this point, invoking their `__toString()`
      method if present.

    - Otherwise, the content or returned value is echoed as a string; note that
      an object will be cast to string at this point, invoking its
      `__toString()` method if present.

### Extending and Overriding

**Although it is easy and convenient to extend this class, the authors recommend
decoration and composition over extension in all but the most trivial of cases.**

_SapiResponseSender_ is constructorless, which means you can add any constructor
you like and not have to call a parent constructor.

The _SapiResponseSender_ methods are public but not final, which means you can
extend and override them as you see fit. Doing so for any method other than
sendContent() might not make sense. There is pretty much only one way to send
headers, cookies, etc., but different kinds of content might well deserve
sending logic that differs from the default sendContent() logic.
