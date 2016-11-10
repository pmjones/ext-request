# array http\Env\Request::getFiles()

Retrieve the uploaded files list ($_FILES).

## Params:

None.

## Returns:

* array, the consolidated upload files array.

## Example:

Here's an example how the original $_FILES array and the http\Env\Request's files array compare:

	$r = new http\Env\Request;
	$f = array();

	foreach ($_FILES as $name => $data) {
		foreach ((array) $data["tmp_name"] as $i => $file) {
			$f[$name][$i] = array(
				"file" => $file,
				"name" => $data["name"][$i],
				"size" => $data["size"][$i],
				"type" => $data["type"][$i],
				"error"=> $data["error"][$i]
			);
		}
	}
	assert($f == $r->getFiles());

You can see that the second and third dimensions are swapped and ```tmp_name``` became ```file```, e.g:

    array(
		"upload" => array(
			array(
				"file" => "/tmp/phpXXXXXX",
				"name" => "picture.jpeg",
				"size" => 12345,
				"type" => "image/jpeg",
				"error"=> 0
			)
		)
	);
