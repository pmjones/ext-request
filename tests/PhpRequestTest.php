<?php
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

        $expect = [
            'scheme' => 'http',
            'host' => 'example.com',
            'port' => 8080,
            'user' => NULL,
            'pass' => NULL,
            'path' => '/foo/bar',
            'query' => 'baz=dib',
            'fragment' => NULL,
        ];
        $this->assertSame($expect, $request->url);
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

        $this->assertSame('https', $request->url['scheme']);
    }

    public function testUrl_serverName()
    {
        $_SERVER = [
            'SERVER_NAME' => 'example.com',
        ];

        $request = new PhpRequest();
        $this->assertSame('example.com', $request->url['host']);
    }

    public function testUrl_hostPort()
    {
        $_SERVER = [
            'HTTP_HOST' => 'example.com:8080',
        ];

        $request = new PhpRequest();
        $this->assertSame('example.com', $request->url['host']);
        $this->assertSame(8080, $request->url['port']);
    }

    public function testUrl_serverPort()
    {
        $_SERVER += [
            'SERVER_PORT' => '8080',
        ];

        $request = new PhpRequest();
        $this->assertSame('example.com', $request->url['host']);
        $this->assertSame(8080, $request->url['port']);
    }

    public function testAcceptMedia()
    {
        $_SERVER += [
            'HTTP_ACCEPT' => 'application/xml;q=0.8, application/json;foo=bar, text/*;q=0.2, */*;q=0.1',
        ];

        $request = new PhpRequest();

        $expect = [
            0 => [
                'value' => 'application/json',
                'quality' => '1.0',
                'params' => ['foo' => 'bar'],
            ],
            1 => [
                'value' => 'application/xml',
                'quality' => '0.8',
                'params' => [],
            ],
            2 => [
                'value' => 'text/*',
                'quality' => '0.2',
                'params' => [],
            ],
            3 => [
                'value' => '*/*',
                'quality' => '0.1',
                'params' => [],
            ],
        ];
        $this->assertSame($expect, $request->acceptMedia);
    }

    public function testAcceptCharset()
    {
        $_SERVER += [
            'HTTP_ACCEPT_CHARSET' => 'iso-8859-5;q=0.8, unicode-1-1',
        ];

        $request = new PhpRequest();

        $expect = [
            0 => [
                'value' => 'unicode-1-1',
                'quality' => '1.0',
                'params' => [],
            ],
            1 => [
                'value' => 'iso-8859-5',
                'quality' => '0.8',
                'params' => [],
            ],
        ];

        $this->assertSame($expect, $request->acceptCharset);
    }

    public function testAcceptEncoding()
    {
        $_SERVER += [
            'HTTP_ACCEPT_ENCODING' => 'compress;q=0.5, gzip;q=1.0',
        ];

        $request = new PhpRequest();

        $expect = [
            0 => [
                'value' => 'gzip',
                'quality' => '1.0',
                'params' => [],
            ],
            1 => [
                'value' => 'compress',
                'quality' => '0.5',
                'params' => [],
            ],
        ];

        $this->assertSame($expect, $request->acceptEncoding);
    }

    public function testAcceptLanguage()
    {
        $_SERVER += [
            'HTTP_ACCEPT_LANGUAGE' => 'en-US, en-GB, en, *',
        ];

        $request = new PhpRequest();

        $expect = [
            0 => [
                'value' => 'en-US',
                'quality' => '1.0',
                'params' => [],
                'type' => 'en',
                'subtype' => 'US',
            ],
            1 => [
                'value' => 'en-GB',
                'quality' => '1.0',
                'params' => [],
                'type' => 'en',
                'subtype' => 'GB',
            ],
            2 => [
                'value' => 'en',
                'quality' => '1.0',
                'params' => [],
                'type' => 'en',
                'subtype' => null,
            ],
            3 => [
                'value' => '*',
                'quality' => '1.0',
                'params' => [],
                'type' => '*',
                'subtype' => null,
            ],
        ];

        $this->assertSame($expect, $request->acceptLanguage);
    }
}
