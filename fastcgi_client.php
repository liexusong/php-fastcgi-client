<?php

/* Example */

$fcgi = new FastCGI_Client();

$fcgi->connect('127.0.0.1', 9000);
$fcgi->set_param('SCRIPT_FILENAME', '/index.php'); /* script filename */
$fcgi->set_param('REQUEST_METHOD', 'GET');
$fcgi->start_request();

$content = $fcgi->read_response();
var_dump($content);

$fcgi->close();
