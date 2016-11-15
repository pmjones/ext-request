<?php
putenv('TEST_USERLAND_REQUEST=1');
putenv('TEST_PHP_ARGS=-q');
require_once dirname(__DIR__) . '/src/ServerRequest.php';
require_once dirname(__DIR__) . '/src/ServerResponse.php';
