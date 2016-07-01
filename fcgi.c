/**
 * FastCGI protocol implementation
 * @author: xusong.lie
 */

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "fcgi.h"

#define PARAMS_BUFF_MAX_LEN 5120

static char _paramsbuf[PARAMS_BUFF_MAX_LEN];

int
fastcgi_connect(char *addr, short port)
{
    int fd;
    struct sockaddr_in sain;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
    	return -1;
    }

    bzero(&sain, sizeof(sain));

    sain.sin_family = AF_INET;
    sain.sin_addr.s_addr = inet_addr(addr);
    sain.sin_port = htons(port);

    if (connect(fd, (struct sockaddr *)&sain, sizeof(sain)) < 0) {
    	close(fd);
    	return -1;
    }

    return fd;
}

void
fastcgi_init_header(fcgi_header *header, int type,
	int request_id, int content_length, int padding_legnth)
{
	header->version = FCGI_VERSION_1;
	header->type = (unsigned char)type;
	header->request_id_b1 = (unsigned char)((request_id >> 8) & 0xFF);
	header->request_id_b0 = (unsigned char)(request_id & 0xFF);
	header->content_length_b1 = (unsigned char)((content_length >> 8) & 0xFF);
	header->content_length_b0 = (unsigned char)(content_length & 0xFF);
	header->padding_legnth = padding_legnth;
	header->reserved = 0;
}

void
fastcgi_init_begin_request_body(fcgi_begin_request_body *body,
	int role, int keepalive)
{
	bzero(body, sizeof(*body));

	body->role_b1 = (unsigned char)((role >> 8) & 0xFF);
	body->role_b0 = (unsigned char)(role & 0xFF);
	body->flags = (unsigned char)(keepalive ? 1 : 0);
}

void
fastcgi_init_kv_body(unsigned char *buf, int *len,
	unsigned char *key, int klen, unsigned char *val, int vlen)
{
	unsigned char *last = buf;

	if (klen < 128) {
		*last++ = (unsigned char)klen;
	} else {
		*last++ = (unsigned char)((klen >> 24) | 0x80);
		*last++ = (unsigned char)(klen >> 16);
		*last++ = (unsigned char)(klen >> 8);
		*last++ = (unsigned char)klen;
	}

	if (vlen < 128) {
		*last++ = (unsigned char)vlen;
	} else {
		*last++ = (unsigned char)((vlen >> 24) | 0x80);
		*last++ = (unsigned char)(vlen >> 16);
		*last++ = (unsigned char)(vlen >> 8);
		*last++ = (unsigned char)vlen;
	}

	memcpy(last, key, klen); last += klen;
	memcpy(last, val, vlen); last += vlen;

	*len = last - buf;
}

int
fastcgi_send_start_request(int fd)
{
	fcgi_begin_request_record record;
	int nwrite;

	fastcgi_init_header(&record.header,
		FCGI_TYPE_BEGIN_REQUEST, 0, sizeof(record.body), 0);
	fastcgi_init_begin_request_body(&record.body, FCGI_ROLE_RESPONDER, 0);

	nwrite = write(fd, (char *)&record, sizeof(record));

	return (nwrite == sizeof(record) ? 0 : -1);
}

int
fastcgi_send_param(int fd, char *key, int klen, char *val, int vlen)
{
	int bodylen, nwrite;
	fcgi_header header;

	bzero(_paramsbuf, PARAMS_BUFF_MAX_LEN);

	fastcgi_init_kv_body(_paramsbuf, &bodylen, key, klen, val, vlen);
	fastcgi_init_header(&header, FCGI_TYPE_PARAMS, 0, bodylen, 0);

	nwrite = write(fd, &header, FCGI_HEADER_LENGTH);
	if (nwrite != FCGI_HEADER_LENGTH) {
		return -1;
	}

	nwrite = write(fd, _paramsbuf, bodylen);
	if (nwrite != bodylen) {
		return -1;
	}

	return 0;
}

int
fastcgi_send_end_request(int fd)
{
	fcgi_header header;

	fastcgi_init_header(&header, FCGI_TYPE_PARAMS, 0, 0, 0);

	if (write(fd, &header, FCGI_HEADER_LENGTH) != FCGI_HEADER_LENGTH) {
		return -1;
	}

	return 0;
}

int
fastcgi_read_header(int fd, fcgi_header *header)
{
	if (read(fd, header, FCGI_HEADER_LENGTH) != FCGI_HEADER_LENGTH) {
		return -1;
	}

	return 0;
}

int
fastcgi_read_body(int fd, char *buffer, int length, int padding)
{
	int nread;
	char temp[8];

	nread = read(fd, buffer, length);
	if (nread != length) {
		return -1;
	}

	if (padding > 0 && padding <= 8) {
		nread = read(fd, temp, padding);
		if (nread != padding) {
			return -1;
		}
	}

	return 0;
}

int
fastcgi_read_end_request(int fd)
{
	fcgi_end_request_body body;

	if (read(fd, &body, sizeof(body)) != sizeof(body)) {
		return -1;
	}

	return 0;
}

// int
// fastcgi_read_package(int fd,
// 	void *(*memalloc)(size_t size),
// 	void (*memfree)(void *),
// 	char **retbuf, int *retlen)
// {
// 	fcgi_header header;
// 	size_t length;
// 	char *buffer = NULL, padding[8];
// 	int nread;

// 	nread = read(fd, &header, FCGI_HEADER_LENGTH);
// 	if (nread != FCGI_HEADER_LENGTH) {
// 		return FCGI_RESPONSE_ERROR;
// 	}

// 	switch (header.type) {

// 	case FCGI_TYPE_STDOUT:
// 	case FCGI_TYPE_STDERR:
// 	{
// 		length = (header.content_length_b1 << 8) + header.content_length_b0;

// 		buffer = (char *)memalloc(length);
// 		if (!buffer) {
// 			return FCGI_RESPONSE_ERROR;
// 		}

// 		/* read content */
// 		nread = read(fd, buffer, length);
// 		if (nread != length) {
// 			memfree(buffer);
// 			return FCGI_RESPONSE_ERROR;
// 		}

// 		/* read padding */
// 		if (header.padding_legnth > 0) {
// 			read(fd, padding, header.padding_legnth);
// 		}

// 		*retlen = length;
// 		*retbuf = buffer;

// 		return FCGI_RESPONSE_VALUE;
// 	}

// 	case FCGI_TYPE_END_REQUEST:
// 	{
// 		fcgi_end_request_body body;

// 		if (read(fd, &body, sizeof(body)) != sizeof(body)) {
// 			return FCGI_RESPONSE_ERROR;
// 		}
// 		return FCGI_RESPONSE_CLOSE;
// 	}
// 	}
// }
