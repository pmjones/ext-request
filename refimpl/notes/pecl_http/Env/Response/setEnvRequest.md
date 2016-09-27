# http\Env\Response http\Env\Response::setEnvRequest(http\Message $env_request)

Override the environment's request.

## Params:

* http\Message $env_request  
  The overriding request message.

## Returns:

* http\Env\Response, self.

## Throws:

* http\Exception\InvalidArgumentException

## Example:

    <?php
    $req = new http\Env\Request;
    $req->setRequestUrl("/ha/I/changed/it");
    $res = new http\Env\Response;
    $res->setEnvRequest($req);
    ?>
