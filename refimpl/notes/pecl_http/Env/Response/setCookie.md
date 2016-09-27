# http\Env\Response http\Env\Response::setCookie(mixed $cookie)

Add cookies to the response to send.

## Params:

* mixed $cookie  
  The cookie to send.

## Returns:

* http\Env\Response, self.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\UnexpectedValueException

## Example:

	<?php
	
	$r = new http\Env\Response;
	$r->setCookie("visit=true; path=/");
	$r->send(STDOUT);
	
	?>

Yields:

	HTTP/1.1 200 OK
	Set-Cookie: visit=true; path=/; 
	ETag: ""

### Another example:

	<?php
	
	$c = new http\Cookie;
	$c->addCookie("foo", "bar");
	$c->setMaxAge(360);
	$c->setDomain(".example.org");
	$c->setPath("/");
	$c->setFlags(http\Cookie::SECURE | http\Cookie::HTTPONLY);
	
	$r = new http\Env\Response;
	$r->setCookie($c);
	$r->send(STDOUT);
	
	?>

Yields:

	HTTP/1.1 200 OK
	Set-Cookie: foo=bar; domain=.example.org; path=/; max-age=360; secure; httpOnly; 
	ETag: ""
