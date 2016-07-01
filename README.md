# php-fastcgi-client
PHP FastCGI client

### Example: ###

```php
<?php

/* Example */

$fcgi = new FastCGI_Client();

$fcgi->connect('127.0.0.1', 9000);
$fcgi->send_param('SCRIPT_FILENAME', '/index.php'); /* script filename */
$fcgi->send_param('REQUEST_METHOD', 'GET');
$fcgi->start_request();

$content = $fcgi->read_response();
var_dump($content);

$fcgi->close();
```
