<?php
class ExtendedRequest extends ServerRequest
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
            'ExtendedRequest::$method is read-only.'
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
