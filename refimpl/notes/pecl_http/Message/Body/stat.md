# mixed http\Message\Body::stat([string $field = NULL])

Stat size, atime, mtime and/or ctime.

## Params:

* Optional string $field = NULL  
  A single stat field to retrieve.

## Returns:

* int, the requested stat field.
* object, stdClass instance holding all four stat fields.

## Example:

    <?php
    $body = new http\Message\Body(fopen(__FILE__, "r"));
    var_dump($body->stat(), $body->stat()->size, $body->stat("s"));
    ?>

Yields:

    object(stdClass)#2 (4) {
      ["size"]=>
      int(661)
      ["atime"]=>
      int(1383830756)
      ["mtime"]=>
      int(1383830753)
      ["ctime"]=>
      int(1383830753)
    }
    int(661)
    int(661)
