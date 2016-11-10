# static http\Message\Body http\Env::getRequestBody([string $body_class_name])

Retreive the current HTTP request's body.

## Parameters:

* Optional string $body_class_name  
  A user class extending http\Message\Body.

## Returns:

* http\Message\Body instance representing the request body

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\UnexpectedValueException
