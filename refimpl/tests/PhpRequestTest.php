<?php
// this allows for using this test for *both* the reference
// implementation *and* the extension
if (! class_exists('PhpRequest')) {
    require dirname(__DIR__) . '/src/PhpRequest.php';
}

/**
 * @backupGlobals enabled
 */
class PhpRequestTest extends PHPUnit_Framework_TestCase
{
    protected function setUp()
    {
        $_SERVER['HTTP_HOST'] = 'example.com';
    }

    public function test__construct()
    {
        $request = new PhpRequest();
        $this->assertInstanceOf(PhpRequest::CLASS, $request);
    }

    public function test__get()
    {
        $request = new PhpRequest('PATCH');
        $this->assertTrue(isset($request->method));

        $this->setExpectedException(
            RuntimeException::CLASS,
            'PhpRequest::$noSuchProperty does not exist.'
        );
        $request->noSuchProperty;
    }

    public function test__unset()
    {
        $request = new PhpRequest('PATCH');

        $this->setExpectedException(
            RuntimeException::CLASS,
            'PhpRequest is read-only.'
        );

        $request->method = 'HEAD';
    }

    public function test__isset()
    {
        $request = new PhpRequest('PATCH');
        $this->assertTrue(isset($request->method));

        $this->setExpectedException(
            RuntimeException::CLASS,
            'PhpRequest::$noSuchProperty does not exist.'
        );
        isset($request->noSuchProperty);
    }

    public function test__set()
    {
        $request = new PhpRequest('PATCH');

        $this->setExpectedException(
            RuntimeException::CLASS,
            'PhpRequest is read-only.'
        );

        unset($request->method);
    }

    public function testSuperglobalsAreCopied()
    {
        $request = new PhpRequest();
        $_SERVER['HTTP_HOST'] = 'NOT example.com';
        $this->assertSame('example.com', $request->server['HTTP_HOST']);
    }

    public function testMethod()
    {
        $request = new PhpRequest();
        $this->assertSame('', $request->method);
        $this->assertFalse($request->xhr);

        $request = new PhpRequest('PATCH');
        $this->assertSame('PATCH', $request->method);
        $this->assertFalse($request->xhr);

        $_SERVER['REQUEST_METHOD'] = 'POST';
        $request = new PhpRequest();
        $this->assertSame('POST', $request->method);
        $this->assertFalse($request->xhr);

        $_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE'] = 'PATCH';
        $request = new PhpRequest();
        $this->assertSame('PATCH', $request->method);
        $this->assertTrue($request->xhr);
    }

    public function testHeaders()
    {
        $_SERVER = [
            'HTTP_HOST' => 'example.com',
            'HTTP_FOO_BAR_BAZ' => 'dib,zim,gir',
            'NON_HTTP_HEADER' => 'should not show',
            'CONTENT_LENGTH' => '123',
            'CONTENT_TYPE' => 'text/plain',
        ];

        $request = new PhpRequest();

        $expect = [
            'Host' => 'example.com',
            'Foo-Bar-Baz' => 'dib,zim,gir',
            'Content-Length' => '123',
            'Content-Type' => 'text/plain',
        ];

        $this->assertSame($expect, $request->headers);
    }

    public function testSecure()
    {
        // default
        $request = new PhpRequest();
        $this->assertFalse($request->secure);

        // https on
        $_SERVER = [
            'HTTP_HOST' => 'example.com',
            'HTTPS' => 'on',
        ];
        $request = new PhpRequest();
        $this->assertTrue($request->secure);

        // https off
        $_SERVER = [
            'HTTP_HOST' => 'example.com',
            'HTTPS' => 'off',
        ];
        $request = new PhpRequest();
        $this->assertFalse($request->secure);

        // port 443
        $_SERVER = [
            'HTTP_HOST' => 'example.com',
            'SERVER_PORT' => '443',
        ];
        $request = new PhpRequest();
        $this->assertTrue($request->secure);

        // port 80
        $_SERVER = [
            'HTTP_HOST' => 'example.com',
            'SERVER_PORT' => '80',
        ];
        $request = new PhpRequest();
        $this->assertFalse($request->secure);

        // forwarded https
        $_SERVER = [
            'HTTP_HOST' => 'example.com',
            'HTTP_X_FORWARDED_PROTO' => 'https',
        ];
        $request = new PhpRequest();
        $this->assertTrue($request->secure);

        // forwarded http
        $_SERVER = [
            'HTTP_HOST' => 'example.com',
            'HTTP_X_FORWARDED_PROTO' => 'http',
        ];
        $request = new PhpRequest();
        $this->assertFalse($request->secure);
    }

    public function testUrl()
    {
        $_SERVER += [
            'REQUEST_URI' => '/foo/bar?baz=dib',
            'SERVER_PORT' => '8080',
        ];
        $request = new PhpRequest();

        $expect = (object) [
            'scheme' => 'http',
            'host' => 'example.com',
            'port' => 8080,
            'user' => null,
            'pass' => null,
            'path' => '/foo/bar',
            'query' => 'baz=dib',
            'fragment' => null,
        ];
        $this->assertEquals($expect, $request->url);
    }

    public function testUrl_withoutHost()
    {
        $_SERVER = [];
        $this->setExpectedException(
            RuntimeException::CLASS,
            'Could not determine host for PhpRequest.'
        );
        $request = new PhpRequest();
    }

    public function testUrl_secure()
    {
        $_SERVER['HTTPS'] = 'on';
        $request = new PhpRequest();

        $this->assertSame('https', $request->url->scheme);
    }

    public function testUrl_serverName()
    {
        $_SERVER = [
            'SERVER_NAME' => 'example.com',
        ];

        $request = new PhpRequest();
        $this->assertSame('example.com', $request->url->host);
    }

    public function testUrl_hostPort()
    {
        $_SERVER = [
            'HTTP_HOST' => 'example.com:8080',
        ];

        $request = new PhpRequest();
        $this->assertSame('example.com', $request->url->host);
        $this->assertSame(8080, $request->url->port);
    }

    public function testUrl_serverPort()
    {
        $_SERVER += [
            'SERVER_PORT' => '8080',
        ];

        $request = new PhpRequest();
        $this->assertSame('example.com', $request->url->host);
        $this->assertSame(8080, $request->url->port);
    }

    public function testAcceptMedia()
    {
        $_SERVER += [
            'HTTP_ACCEPT' => 'application/xml;q=0.8, application/json;foo=bar, text/*;q=0.2, */*;q=0.1',
        ];

        $request = new PhpRequest();

        $expect = [
            0 => (object) [
                'value' => 'application/json',
                'quality' => '1.0',
                'params' => ['foo' => 'bar'],
            ],
            1 => (object) [
                'value' => 'application/xml',
                'quality' => '0.8',
                'params' => [],
            ],
            2 => (object) [
                'value' => 'text/*',
                'quality' => '0.2',
                'params' => [],
            ],
            3 => (object) [
                'value' => '*/*',
                'quality' => '0.1',
                'params' => [],
            ],
        ];

        $this->assertEquals($expect, $request->acceptMedia);
    }

    public function testAcceptCharset()
    {
        $_SERVER += [
            'HTTP_ACCEPT_CHARSET' => 'iso-8859-5;q=0.8, unicode-1-1',
        ];

        $request = new PhpRequest();

        $expect = [
            0 => (object) [
                'value' => 'unicode-1-1',
                'quality' => '1.0',
                'params' => [],
            ],
            1 => (object) [
                'value' => 'iso-8859-5',
                'quality' => '0.8',
                'params' => [],
            ],
        ];

        $this->assertEquals($expect, $request->acceptCharset);
    }

    public function testAcceptEncoding()
    {
        $_SERVER += [
            'HTTP_ACCEPT_ENCODING' => 'compress;q=0.5, gzip;q=1.0',
        ];

        $request = new PhpRequest();

        $expect = [
            0 => (object) [
                'value' => 'gzip',
                'quality' => '1.0',
                'params' => [],
            ],
            1 => (object) [
                'value' => 'compress',
                'quality' => '0.5',
                'params' => [],
            ],
        ];

        $this->assertEquals($expect, $request->acceptEncoding);
    }

    public function testAcceptLanguage()
    {
        $_SERVER += [
            'HTTP_ACCEPT_LANGUAGE' => 'en-US, en-GB, en, *',
        ];

        $request = new PhpRequest();

        $expect = [
            0 => (object) [
                'value' => 'en-US',
                'quality' => '1.0',
                'params' => [],
                'type' => 'en',
                'subtype' => 'US',
            ],
            1 => (object) [
                'value' => 'en-GB',
                'quality' => '1.0',
                'params' => [],
                'type' => 'en',
                'subtype' => 'GB',
            ],
            2 => (object) [
                'value' => 'en',
                'quality' => '1.0',
                'params' => [],
                'type' => 'en',
                'subtype' => null,
            ],
            3 => (object) [
                'value' => '*',
                'quality' => '1.0',
                'params' => [],
                'type' => '*',
                'subtype' => null,
            ],
        ];

        $this->assertEquals($expect, $request->acceptLanguage);
    }

    public function testAuthBasic()
    {
        $_SERVER += [
            'PHP_AUTH_TYPE' => 'Basic',
            'PHP_AUTH_USER' => 'foo',
            'PHP_AUTH_PW' => 'bar'
        ];

        $request = new PhpRequest();

        $this->assertSame('Basic', $request->authType);
        $this->assertSame('foo', $request->authUser);
        $this->assertSame('bar', $request->authPw);
    }

    public function testAuthDigest()
    {
        $_SERVER += [
            'PHP_AUTH_TYPE' => 'Digest',
            'PHP_AUTH_DIGEST' => implode(',', [
                'nonce="foo"',
                'nc=\'bar\'',
                'cnonce=baz',
                'qop="dib"',
                'username="zim"',
                'uri="gir"',
                'response="irk"',
            ]),
        ];

        $request = new PhpRequest();

        $expect = (object) [
            'nonce' => 'foo',
            'nc' => 'bar',
            'cnonce' => 'baz',
            'qop' => 'dib',
            'username' => 'zim',
            'uri' => 'gir',
            'response' => 'irk',
        ];

        $this->assertEquals($expect, $request->authDigest);
    }

    public function testAuthDigest_missingParts()
    {
        $_SERVER += [
            'PHP_AUTH_TYPE' => 'Digest',
            'PHP_AUTH_DIGEST' => implode(',', [
                'nonce="foo"',
                'nc=\'bar\'',
                'cnonce=baz',
            ]),
        ];

        $request = new PhpRequest();
        $this->assertSame('Digest', $request->authType);
        $this->assertNull($request->authDigest);
    }

    public function testContent()
    {
        $_SERVER += [
            'HTTP_CONTENT_MD5' => 'foobar',
            'HTTP_CONTENT_LENGTH' => '123',
            'HTTP_CONTENT_TYPE' => 'text/plain',
        ];

        $request = new PhpRequest();
        $this->assertSame('foobar', $request->contentMd5);
        $this->assertSame('123', $request->contentLength);
        $this->assertSame('text/plain', $request->contentType);
        $this->assertNull($request->contentCharset);

        $_SERVER['HTTP_CONTENT_TYPE'] = 'text/plain;foo=bar;charset=utf-8;baz=dib';
        $request = new PhpRequest();
        $this->assertSame('foobar', $request->contentMd5);
        $this->assertSame('123', $request->contentLength);
        $this->assertSame('text/plain', $request->contentType);
        $this->assertSame('utf-8', $request->contentCharset);
    }
}
