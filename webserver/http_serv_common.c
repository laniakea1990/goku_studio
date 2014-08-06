#include <http_serv_common.h>

//notice!http 1.1 suport connection keep alive
//char *http_rsp_200_def = "HTTP/1.0 200 OK\r\nServer: learn_c_web_server\r\nDate: Sat, 19 Jan 2013 12:15:02 GMT\r\nContent-Type: text/html; charset=UTF-8\r\nLast-Modified: Mon, 27 Aug 2012 13:06:40 GMT\r\nContent-Length: ";
//char *http_rsp_404_def = "HTTP/1.0 404 Not Found\r\nServer: learn_c_web_server\r\nDate: Sat, 19 Jan 2013 12:15:02 GMT\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 147\r\nLast-Modified: Mon, 27 Aug 2012 13:06:40 GMT\r\n\r\n<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><p>404 Not Found: The requested resource could not be found!</p></body></html>";
char *http_rsp_200_def = "HTTP/1.1 200 OK\r\nServer: learn_c_web_server\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: Close\r\nContent-Length: ";
char *http_rsp_404_def = "HTTP/1.1 404 Not Found\r\nServer: learn_c_web_server\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: Close\r\nContent-Length: 147\r\n\r\n<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><p>404 Not Found: The requested resource could not be found!</p></body></html>";

src_f_list *s_list = NULL;

size_t max_file_size = 0;
