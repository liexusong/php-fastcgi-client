/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_fastcgi_client.h"
#include "fcgi.h"

#define  FASTCGI_CONNECTION_SOCKET  "__sockfd"

/* If you declare any globals in php_fastcgi_client.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(fastcgi_client)
*/

/* True global resources - no need for thread safety here */
static int le_fastcgi_client;
static zend_class_entry *fastcgi_ce;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("fastcgi_client.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_fastcgi_client_globals, fastcgi_client_globals)
    STD_PHP_INI_ENTRY("fastcgi_client.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_fastcgi_client_globals, fastcgi_client_globals)
PHP_INI_END()
*/
/* }}} */

zend_function_entry fastcgi_methods[] = {
	PHP_ME(FastCGI_Client, connect,       NULL, ZEND_ACC_PUBLIC)
	PHP_ME(FastCGI_Client, set_param,     NULL, ZEND_ACC_PUBLIC)
	PHP_ME(FastCGI_Client, start_request, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(FastCGI_Client, read_response, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(FastCGI_Client, close,         NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};


PHP_METHOD(FastCGI_Client, connect)
{
	int fd;
	zval *instance;
	char *addr;
	int addr_len;
	int port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"sl", &addr, &addr_len, &port) == FAILURE)
	{
		RETURN_FALSE;
	}

	instance = getThis();

	fd = fastcgi_connect(addr, (short)port);
	if (fd < 0) {
		RETURN_FALSE;
	}

	zend_update_property_long(fastcgi_ce, instance,
		ZEND_STRL(FASTCGI_CONNECTION_SOCKET), fd TSRMLS_CC);

	RETURN_TRUE;
}


PHP_METHOD(FastCGI_Client, set_param)
{
	int fd;
	zval *instance, *sock;
	char *key, *val;
	int key_len, val_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"ss", &key, &key_len, &val, &val_len) == FAILURE)
	{
		RETURN_FALSE;
	}

	instance = getThis();

	sock = zend_read_property(fastcgi_ce,
		instance, ZEND_STRL(FASTCGI_CONNECTION_SOCKET), 0 TSRMLS_CC);

	if (Z_LVAL_P(sock) < 0
		|| fastcgi_send_param(Z_LVAL_P(sock), key, key_len, val, val_len) == -1)
	{
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(FastCGI_Client, start_request)
{
	int fd;
	zval *instance, *sock;

	instance = getThis();

	sock = zend_read_property(fastcgi_ce,
		instance, ZEND_STRL(FASTCGI_CONNECTION_SOCKET), 0 TSRMLS_CC);

	if (Z_LVAL_P(sock) < 0
		|| fastcgi_send_end_request(Z_LVAL_P(sock)) == -1)
	{
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_METHOD(FastCGI_Client, read_response)
{
	int fd;
	zval *instance, *sock;
	fcgi_header response_header;
	char *response = NULL;
	int total = 0;
	int exit_flag = 0;

	instance = getThis();

	sock = zend_read_property(fastcgi_ce,
		instance, ZEND_STRL(FASTCGI_CONNECTION_SOCKET), 0 TSRMLS_CC);
	if (Z_LVAL_P(sock) < 0){
		RETURN_FALSE;
	}

	while (!exit_flag) {

		if (fastcgi_read_header(Z_LVAL_P(sock), &response_header) == -1) {
			RETURN_FALSE;
		}

		switch (response_header.type) {
			case FCGI_TYPE_STDOUT:
			case FCGI_TYPE_STDERR:
			{
				int length, padding;
				char *response;
				int ret;

				length = (response_header.content_length_b1 << 8)
						+ response_header.content_length_b0;
				padding = response_header.padding_length;

				total += length; /* total bytes */

				response = erealloc(response, total);
				if (!response) {
					RETURN_FALSE;
				}

				ret = fastcgi_read_body(Z_LVAL_P(sock),
					response + (total - length), length, padding);
				if (ret == -1) {
					RETURN_FALSE;
				}

				break;
			}

			case FCGI_TYPE_END_REQUEST:
			{
				if (fastcgi_read_end_request(Z_LVAL_P(sock)) == -1) {
					RETURN_FALSE;
				}

				exit_flag = 1;

				break;
			}
		}
	}

	RETURN_STRINGL(response, total, 0);
}


PHP_METHOD(FastCGI_Client, close)
{
	zval *instance, *sock;

	instance = getThis();

	sock = zend_read_property(fastcgi_ce,
		instance, ZEND_STRL(FASTCGI_CONNECTION_SOCKET), 0 TSRMLS_CC);

	if (Z_LVAL_P(sock) < 0) {
		RETURN_FALSE;
	}

	close(Z_LVAL_P(sock));

	zend_update_property_long(fastcgi_ce, instance,
		ZEND_STRL(FASTCGI_CONNECTION_SOCKET), -1 TSRMLS_CC);

	RETURN_TRUE;
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(fastcgi_client)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "FastCGI_Client", fastcgi_methods);

	fastcgi_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_declare_property_long(fastcgi_ce,
		ZEND_STRL(FASTCGI_CONNECTION_SOCKET), -1, ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(fastcgi_client)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(fastcgi_client)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(fastcgi_client)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(fastcgi_client)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "fastcgi_client support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ fastcgi_client_functions[]
 *
 * Every user visible function must have an entry in fastcgi_client_functions[].
 */
const zend_function_entry fastcgi_client_functions[] = {
	PHP_FE_END	/* Must be the last line in fastcgi_client_functions[] */
};
/* }}} */

/* {{{ fastcgi_client_module_entry
 */
zend_module_entry fastcgi_client_module_entry = {
	STANDARD_MODULE_HEADER,
	"fastcgi_client",
	fastcgi_client_functions,
	PHP_MINIT(fastcgi_client),
	PHP_MSHUTDOWN(fastcgi_client),
	PHP_RINIT(fastcgi_client),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(fastcgi_client),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(fastcgi_client),
	PHP_FASTCGI_CLIENT_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FASTCGI_CLIENT
ZEND_GET_MODULE(fastcgi_client)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
