# mixed http\Message::getHeader(string $header[, string $into_class = NULL])

Retrieve a single header, optionally hydrated into a http\Header extending class.

## Params:

* string $header  
  The header's name.
* Optional string $into_class = NULL  
  The name of a class extending http\Header.

## Returns:

* mixed, the header value if $into_class is NULL.
* http\Header, descendant.

## Warnings:

* If $into_class is specified but is not a descendant of http\Header.

## Example:

    <?php
    class hdr extends http\Header {
        function __construct($name, $value) {
            var_dump($name, $value);
            parent::__construct($name, $value);
        }
    }
    
    $msg = new http\Message("Foo: bar");
    $msg->getHeader("foo", "hdr");
    ?>

Yields:

    string(3) "foo"
    string(3) "bar"
