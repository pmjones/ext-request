# class http\Env\Request extends http\Message

The http\Env\Request class' instances represent the server's current HTTP request.

See http\Message for inherited members.

## Constants:

None.

## Properties:

* protected http\QueryString $query = NULL  
  The request's query parameters. ($_GET)
* protected http\QueryString $form = NULL  
  The request's form parameters. ($_POST)
* protected array $files = NULL  
  The request's form uploads. ($_FILES)
* protected array $cookie = NULL  
  The request's cookies. ($_COOKIE)
