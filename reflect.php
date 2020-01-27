<?php
echo (new ReflectionClass(ServerRequest::CLASS));
echo (new ReflectionClass(ServerResponse::CLASS));
echo (new ReflectionClass(ServerResponseSender::CLASS));

$request = new ServerRequest([]);
var_export($request);

$response = new ServerResponse();
var_export($response);

$responseSender = new ServerResponseSender();
var_export($responseSender);
