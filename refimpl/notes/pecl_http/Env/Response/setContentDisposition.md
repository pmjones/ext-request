# http\Env\Response http\Env\Response::setContentDisposition(array $disposition_params)

Set the reponse's content disposition parameters.

## Params:

* array $disposition_params  
  MIME content disposition as http\Params array.  
  
## Returns:

* http\Env\Response, self.

## Throws:

* http\Exception\InvalidArgumentException

## Example:

    <?php ob_end_Clean();chdir(__DIR__."/../../..");
    $res = new http\Env\Response;
    $res->setBody(new http\Message\Body(fopen("http.zip", "r")));
    $res->setContentType("application/zip");
    $res->setContentDisposition(["attachment" => ["filename" => "download.zip"]]);
    $res->send();
    ?>

Yields:

    Accept-Ranges: bytes
    X-Powered-By: PHP/5.5.5
    Content-Type: application/zip
    Content-Disposition: attachment;filename=download.zip
    ETag: "12009be-527d3e84-a0"
    Last-Modified: Fri, 08 Nov 2013 19:41:56 GMT

    PK...
