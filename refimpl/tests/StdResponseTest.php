<?php
// this allows for using this test for *both* the reference
// implementation *and* the extension
if (! class_exists('StdResponse')) {
    require dirname(__DIR__) . '/src/StdResponse.php';
}

class StdResponseTest extends PHPUnit_Framework_TestCase
{
    protected $response;

    protected function setUp()
    {
        $this->response = new StdResponse();
    }

    public function testVersion()
    {
        $this->assertSame('1.1', $this->response->getVersion());
        $this->response->setVersion('1.0');
        $this->assertSame('1.0', $this->response->getVersion());
    }

    public function testStatus()
    {
        $this->assertSame(200, $this->response->getStatus());
        $this->response->setStatus(401);
        $this->assertSame(401, $this->response->getStatus());
    }

    public function testSetHeader()
    {
        $this->response->setHeader('', 'should-not-show');
        $this->response->setHeader('Should-Not-Show', '');
        $this->response->setHeader('Foo-Bar', 'baz');
        $this->response->setHeader('foo-bar', 'dib');

        $expect = [
            'foo-bar' => 'dib'
        ];

        $this->assertSame($expect, $this->response->getHeaders());
    }

    public function testSetHeader_array()
    {
        $this->response->setHeader('Cache-Control', [
            'public',
            'max-age' => '123',
            's-maxage' => '456',
            'no-cache',
        ]);

        $this->response->setHeader('content-type', [
            'text/plain' => ['charset' => 'utf-8']
        ]);

        $expect = [
            'cache-control' => 'public, max-age=123, s-maxage=456, no-cache',
            'content-type' => 'text/plain;charset=utf-8',
        ];

        $this->assertSame($expect, $this->response->getHeaders());
    }

    public function testAddHeader()
    {
        $this->response->addHeader('', 'should-not-show');
        $this->response->addHeader('Should-Not-Show', '');
        $this->response->addHeader('Foo-Bar', 'baz');
        $this->response->addHeader('foo-bar', 'dib');

        $expect = [
            'foo-bar' => 'baz, dib',
        ];

        $this->assertSame($expect, $this->response->getHeaders());
    }


    public function testAddHeader_array()
    {
        $this->response->addHeader('Foo', ['bar' => 'baz']);
        $this->response->addHeader('Foo', ['dib' => ['zim', 'gir']]);

        $expect = [
            'foo' => 'bar=baz, dib;zim;gir',
        ];

        $this->assertSame($expect, $this->response->getHeaders());
    }

    public function testCookies()
    {
        $this->response->setCookie('foo', 'bar');
        $this->response->setRawCookie('baz', 'dib');
        $expect = [
            'foo' => [
                'raw' => false,
                'value' => 'bar',
                'expire' => 0,
                'path' => '',
                'domain' => '',
                'secure' => false,
                'httponly' => false,
            ],
            'baz' => [
                'raw' => true,
                'value' => 'dib',
                'expire' => 0,
                'path' => '',
                'domain' => '',
                'secure' => false,
                'httponly' => false,
            ],
        ];
        $this->assertEquals($expect, $this->response->getCookies());
    }

    public function testContent()
    {
        $this->response->setContent('foo');
        $this->assertSame('foo', $this->response->getContent());
    }

    public function testSetContentJson()
    {
        $value = ['foo' => 'bar'];
        $this->response->setContentJson($value);
        $this->assertSame('{"foo":"bar"}', $this->response->getContent());
        $this->assertSame('application/json', $this->response->getHeader('content-type'));
    }

    public function testSetContentJson_failed()
    {
        $value = ['foo' => ['bar' => ['baz' => ['dib' => ['zim' => ['gir']]]]]];
        $this->setExpectedException(
            'RuntimeException',
            'JSON encoding failed: Maximum stack depth exceeded'
        );
        $this->response->setContentJson($value, 0, 1);
    }

    public function testSetContentResource()
    {
        $fh = fopen('php://temp', 'rb');
        $this->response->setContentResource(
            $fh,
            'whatever',
            $params = ['foo' => 'bar']
        );

        $expect = [
            'content-type' =>  'application/octet-stream',
            'content-transfer-encoding' => 'binary',
            'content-disposition' => 'whatever;foo=bar',
        ];
        $this->assertSame($expect, $this->response->getHeaders());
        $this->assertSame($fh, $this->response->getContent());
    }

    public function testSetContentResource_notResource()
    {
        $this->setExpectedException(
            'TypeError',
            'Argument 1 passed to StdResponse::setContentResource() must be of the type resource, string given'
        );
        $this->response->setContentResource('not-a-resource', 'disposition');
    }

    public function testSetDownload()
    {
        $fh = fopen('php://memory', 'rb');
        $this->response->setDownload($fh, 'foo.txt');

        $expect = [
            'content-type' =>  'application/octet-stream',
            'content-transfer-encoding' => 'binary',
            'content-disposition' => 'attachment;filename="foo.txt"',
        ];
        $this->assertSame($expect, $this->response->getHeaders());
        $this->assertSame($fh, $this->response->getContent());
    }

    public function testSetDownloadInline()
    {
        $fh = fopen('php://temp', 'rb');
        $this->response->setDownloadInline($fh, 'foo.txt');

        $expect = [
            'content-type' =>  'application/octet-stream',
            'content-transfer-encoding' => 'binary',
            'content-disposition' => 'inline;filename="foo.txt"',
        ];
        $this->assertSame($expect, $this->response->getHeaders());
        $this->assertSame($fh, $this->response->getContent());
    }

    public function testDate()
    {
        $expect = 'Wed, 07 Nov 1979 00:00:00 +0000';
        $this->assertSame($expect, $this->response->date('1979-11-07 +0000'));

        $date = new DateTime('1979-11-07 +0000');
        $this->assertSame($expect, $this->response->date($date));

        $this->assertSame($expect, $this->response->date('1979-11-07 03:00:00 +0300'));
    }

    public function testDateInvalid()
    {
        $this->setExpectedException('Exception');
        $this->response->date(true);
    }

    /**
     * @runInSeparateProcess
     */
    public function testSendHeaders()
    {
        $this->response->setHeader('Foo', 'bar');
        $this->response->setHeader('Baz', 'dib');
        $this->response->addHeader('Baz', 'zim');
        $this->response->setCookie('cookie1', 'value1');
        $this->response->setRawCookie('cookie2', 'value2');
        $this->response->send();
        // CANNOT ASSERT BECAUSE headers_list() DOES NOT WORK IN CLI.
        // HOW TO TEST?
    }

    /**
     * @runInSeparateProcess
     */
    public function testSendContent_string()
    {
        $this->response->setContent('foo');
        $this->assertSent(
            200,
            [],
            'foo'
        );
    }

    /**
     * @runInSeparateProcess
     */
    public function testSendContent_callable()
    {
        $this->response->setContent(function (StdResponse $response) {
            $this->assertSame($this->response, $response);
            echo 'foo';
        });
        $this->assertSent(
            200,
            [],
            'foo'
        );
    }

    /**
     * @runInSeparateProcess
     */
    public function testSendContent_callableWithReturnValue()
    {
        $this->response->setContent(function (StdResponse $response) {
            $this->assertSame($this->response, $response);
            echo 'foo';
            return 'bar';
        });
        $this->assertSent(
            200,
            [],
            'foobar'
        );
    }

    /**
     * @runInSeparateProcess
     */
    public function testSendContent_resource()
    {
        $fh = fopen('php://temp', 'w+');
        fwrite($fh, 'foo');
        $this->response->setContent($fh);
        $this->assertSent(
            200,
            [],
            'foo'
        );
    }

    protected function assertSent($expectStatus, $expectHeaders, $expectContent)
    {
        ob_start();
        $this->response->send();
        $actualContent = ob_get_clean();
        $this->assertSame($expectStatus, http_response_code());
        $this->assertSame($expectHeaders, headers_list());
        $this->assertSame($expectContent, $actualContent);
    }
}
