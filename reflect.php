<?php
echo (new ReflectionClass(ServerRequest::CLASS));
echo (new ReflectionClass(ServerResponse::CLASS));
$request = new ServerRequest([]);
var_export($request);
