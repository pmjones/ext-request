<?php
echo (new ReflectionClass(ServerRequest::CLASS));
echo PHP_EOL;

echo (new ReflectionClass(ServerResponse::CLASS));
echo PHP_EOL;

echo (new ReflectionClass(ServerResponseSender::CLASS));
echo PHP_EOL;

$request = new ServerRequest([]);
var_export($request);
echo PHP_EOL;

$response = new ServerResponse();
var_export($response);
echo PHP_EOL;

$responseSender = new ServerResponseSender();
var_export($responseSender);
echo PHP_EOL;
