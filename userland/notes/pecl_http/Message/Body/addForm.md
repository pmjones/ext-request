# http\Message\Body http\Message\Body::addForm([array $fields = NULL[, array $files = NULL]])

Add form fields and files to the message body.

> ***NOTE:***  
> Currently, http\Message\Body::addForm() creates "multipart/form-data" bodies.

## Params:

* array $fields = NULL  
  List of form fields to add.  

* array $files = NULL  
  List of form files to add.

$fields must look like:

    [
      "field_name" => "value",
      "multi_field" => [
        "value1",
        "value2"
      ]
    ]

$files must look like:

    [
      [  
        "name" => "field_name",  
        "type" => "content/type",  
        "file" => "/path/to/file.ext"
      ], [
        "name" => "field_name2",
        "type" => "text/plain",
        "file" => "file.ext",
        "data" => "string"
      ], [
        "name" => "field_name3",
        "type" => "image/jpeg",
        "file" => "file.ext",
        "data" => fopen("/home/mike/Pictures/mike.jpg","r")
    ]

As you can see, a file structure must contain a "file" entry, which holds a file path, and an optional "data" entry, which may either contain a resource to read from or the actual data as string.

## Returns:

* http\Message\Body, self.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\RuntimeException

## Example:

    <?php
    $body = new http\Message\Body;
    $body->addForm([
      "field_name" => "value",
      "multi_field" => [
        "value1",
        "value2"
      ]
    ], [
      [  
        "name" => "field_name",  
        "type" => "application/octet-stream",  
        "file" => "/run/gpm.pid"
      ], [
        "name" => "field_name2",
        "type" => "text/plain",
        "file" => "signature.txt",
        "data" => "--  \nMike\n"
      ], [
        "name" => "field_name3",
        "type" => "image/jpeg",
        "file" => "picture.jpg",
        "data" => fopen("/home/mike/Pictures/mike.jpg","r")
      ]
    ]);
    
    echo $body;
    ?>

Yields:

    --32260b4b.3fea9114
    Content-Disposition: form-data; name="field_name"

    value
    --32260b4b.3fea9114
    Content-Disposition: form-data; name="multi_field[0]"

    value1
    --32260b4b.3fea9114
    Content-Disposition: form-data; name="multi_field[1]"

    value2
    --32260b4b.3fea9114
    Content-Disposition: form-data; name="field_name"; filename="gpm.pid"
    Content-Transfer-Encoding: binary
    Content-Type: application/octet-stream

    316

    --32260b4b.3fea9114
    Content-Disposition: form-data; name="field_name2"; filename="signature.txt"
    Content-Transfer-Encoding: binary
    Content-Type: text/plain

    --  
    Mike

    --32260b4b.3fea9114
    Content-Disposition: form-data; name="field_name3"; filename="picture.jpg"
    Content-Transfer-Encoding: binary
    Content-Type: image/jpeg

    ...JPEG...
    --32260b4b.3fea9114--
