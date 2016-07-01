#ifndef __FCGI_H
#define __FCGI_H

/* FastCGI protocol's structs and defines */

typedef struct {
  unsigned char version;
  unsigned char type;
  unsigned char request_id_b1;
  unsigned char request_id_b0;
  unsigned char content_length_b1;
  unsigned char content_length_b0;
  unsigned char padding_length;
  unsigned char reserved;
} fcgi_header;

#define FCGI_MAX_LENGTH     0xFFFF
#define FCGI_HEADER_LENGTH  8
#define FCGI_VERSION_1      1

/* FastCGI header types */
#define FCGI_TYPE_BEGIN_REQUEST      1
#define FCGI_TYPE_ABORT_REQUEST      2
#define FCGI_TYPE_END_REQUEST        3
#define FCGI_TYPE_PARAMS             4
#define FCGI_TYPE_STDIN              5
#define FCGI_TYPE_STDOUT             6
#define FCGI_TYPE_STDERR             7
#define FCGI_TYPE_DATA               8
#define FCGI_TYPE_GET_VALUES         9
#define FCGI_TYPE_GET_VALUES_RESULT  10
#define FCGI_TYPE_MAX                11

typedef struct {
  unsigned char role_b1;
  unsigned char role_b0;
  unsigned char flags;
  unsigned char reserved[5];
} fcgi_begin_request_body;

typedef struct {
  fcgi_header header;
  fcgi_begin_request_body body;
} fcgi_begin_request_record;

/* application role types */
#define FCGI_ROLE_KEEP_CONNECTION  1
#define FCGI_ROLE_RESPONDER        1
#define FCGI_ROLE_AUTHORIZER       2
#define FCGI_ROLE_FILTER           3

typedef struct {
  unsigned char app_status_b3;
  unsigned char app_status_b2;
  unsigned char app_status_b1;
  unsigned char app_status_b0;
  unsigned char protocol_status;
  unsigned char reserved[3];
} fcgi_end_request_body;

typedef struct {
  fcgi_header header;
  fcgi_end_request_body body;
} fcgi_end_request_record;

/* end request types */
#define FCGI_ROLE_REQUEST_COMPLETE  0
#define FCGI_ROLE_CANT_MPX_CONN     1
#define FCGI_ROLE_OVERLOADED        2
#define FCGI_ROLE_UNKONW            3

#define FCGI_MAX_CONNS   "FCGI_MAX_CONNS"
#define FCGI_MAX_REQS    "FCGI_MAX_REQS"
#define FCGI_MPXS_CONNS  "FCGI_MPXS_CONNS"

typedef struct {
  unsigned char type;
  unsigned char reserved[7];
} fcgi_unkonw_type_body;

typedef struct {
  fcgi_header header;
  fcgi_unkonw_type_body body;
} fcgi_unkonw_type_record;

#define FCGI_RESPONSE_ERROR  0
#define FCGI_RESPONSE_VALUE  1
#define FCGI_RESPONSE_CLOSE  2

/* FastCGI APIs */

int fastcgi_connect(char *addr, short port);
void fastcgi_init_header(fcgi_header *header, int type,
	int request_id, int content_length, int padding_legnth);
void fastcgi_init_begin_request_body(fcgi_begin_request_body *body,
	int role, int keepalive);
void fastcgi_init_kv_body(unsigned char *buf, int *len,
	unsigned char *key, int klen, unsigned char *val, int vlen);
int fastcgi_send_start_request(int fd);
int fastcgi_send_param(int fd, char *key, int klen, char *val, int vlen);
int fastcgi_send_end_request(int fd);
int fastcgi_read_header(int fd, fcgi_header *header);
int fastcgi_read_body(int fd, char *buffer, int length, int padding);
int fastcgi_read_end_request(int fd);

#endif
