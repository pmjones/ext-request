# mixed http\Env\Request::getForm([string $name = NULL[, mixed $type = NULL[, mixed $defval = NULL[, bool $delete = false]]]])

Retrieve a form value ($_POST).

See http\QueryString::get() and http\QueryString::TYPE_* constants.

## Params:

* Optional string $name = NULL  
  The key to retrieve the value for.
* Optional mixed $type = NULL  
  The type to cast the value to. See http\QueryString::TYPE_* constants.
* Optional mixed $defval = NULL  
  The default value to return if the key $name does not exist.
* Optional bool $delete = false  
  Whether to delete the entry from the querystring after retrieval.


## Returns:

* http\QueryString, if called without arguments.
* string, the whole querystring if $name is of zero length.
* mixed, $defval if the key $name does not exist.
* mixed, the querystring value cast to $type if $type was specified and the key $name exists.
* string, the querystring value if the key $name exists and $type is not specified or equals http\QueryString::TYPE_STRING.
