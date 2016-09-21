<?php
// this allows for using this test for *both* the reference
// implementation *and* the extension
if (! class_exists('StdRequest')) {
    require dirname(__DIR__) . '/src/StdRequest.php';
}

/**
 *
 * Notes:
 *
 * - Exceptions should use the class name, not the literal StdRequest name.
 *
 */
class ExtendedRequest extends StdRequest
{
    private $other;

    public function __get($key)
    {
        if (extension_loaded('request')) {
            return $this->$key;
        } else if( property_exists($this, $key) ) {
            return $this->$key;
        } else {
            return parent::__get($key);
        }
    }

    public function setParent($name, $value)
    {
        $this->$name = $value;
    }

    public function setOther($value)
    {
        $this->other = $value;
    }
}

class ExtendedRequestTest extends PHPUnit_Framework_TestCase
{
    protected function setUp()
    {
        $_SERVER['HTTP_HOST'] = 'example.com';
    }

    public function test__construct()
    {
        $request = new ExtendedRequest();
        $this->assertInstanceOf(ExtendedRequest::CLASS, $request);
    }

    public function testParentPropertiesStayReadOnly()
    {
        $request = new ExtendedRequest();

        $this->setExpectedException(
            RuntimeException::CLASS,
            'StdRequest is read-only.'
        );

        $request->setParent('method', 'PATCH');
    }

    public function testChildPropertiesAreUnderChildControl()
    {
        $request = new ExtendedRequest();
        $request->setOther('foo');
        $this->assertSame('foo', $request->other);
        $this->assertSame('example.com', $request->url['host']);
    }
}
