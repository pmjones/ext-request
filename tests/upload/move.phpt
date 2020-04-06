--TEST--
move_uploaded_file() function
--CREDITS--
Dave Kelsey <d_kelsey@uk.ibm.com>
--POST_RAW--
Content-type: multipart/form-data, boundary=AaB03x

--AaB03x
content-disposition: form-data; name="field1"

Joe Blow
--AaB03x
content-disposition: form-data; name="file1"; filename="file1.txt"
Content-Type: text/plain

abcdef123456789xxxDDDxxxDDDxxxDDD
--AaB03x
content-disposition: form-data; name="file2"; filename="file2.txt"
Content-Type: text/plain

abcdef123456789
--AaB03x--
--FILE--
<?php

$request = new SapiRequest($GLOBALS);

echo "Valid move\n";
$destination1 = __FILE__ . ".tmp";

var_dump($request->uploads['file1']->move($destination1));
$file_contents = file_get_contents($destination1);
$contents_matches = ($file_contents == "abcdef123456789xxxDDDxxxDDDxxxDDD");
var_dump($contents_matches);
unlink($destination1);
echo "\n";

echo "Original name of uploaded file\n";
$destination2 = __FILE__ . ".tmp2";
var_dump($request->uploads['file1']->move($destination2));
echo "\n";

echo "Valid move to existing file\n";
$destination4 = __FILE__ . ".tmp4";
$fd = fopen($destination4, "w");
fclose($fd);
var_dump($request->uploads['file2']->move($destination4));
unlink($destination4);

?>
--EXPECTF--
Valid move
bool(true)
bool(true)

Original name of uploaded file
bool(false)

Valid move to existing file
bool(true)
