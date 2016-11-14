<?php
putenv('TEST_USERLAND_REQUEST=1');
putenv('TEST_PHP_ARGS=-q');
require dirname(__DIR__) . '/vendor/autoload.php';
