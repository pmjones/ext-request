# string http\Message\Body::etag()

Retrieve the ETag of the body.

## Params:

None.

## Returns:

* string, an Apache style ETag of inode, mtime and size in hex concatenated by hyphens if the message body stream is stat-able.
* string, a content hash (which algorithm is determined by INI http.etag.mode) if the stream is not stat-able.
* false, if http.etag.mode is not a known hash algorithm.

## Example:

    <?php
    $temp = (new http\Message\Body)->etag();
    $file = (new http\Message\Body(fopen(__FILE__,"r")))->etag();
    
    ini_set("http.etag.mode", "bogus");
    $fail = (new http\Message\Body)->etag();
    
    var_dump(compact("temp", "file", "fail"));
    ?>

Yields:

    array(3) {
      ["temp"]=>
      string(8) "00000000"
      ["file"]=>
      string(20) "138043f-527b91d5-28c"
      ["fail"]=>
      bool(false)
    }
