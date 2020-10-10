#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#define SERVER_NAME     "dp2_120190211_ep"
#define SERVER_URL      "http://cspro9.sogang.ac.kr:1111/"
#define PROTOCOL        "HTTP/1.0"
#define RFC1123FMT      "%a, %d %b %Y %H:%M:%S GMT"
#define DEFAULT_PATH    "var/tmp/gr120190211"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <queue>
#include <sys/epoll.h>

#define EPOLL_SIZE      5
pthread_mutex_t mutex;

int epfd = 0;

static void file_details(int clnt_sock, char* dir, char* name);
static void send_error(int clnt_sock, int status, const char* title, char* extra_header, const char* text);
static void send_headers(int clnt_sock, int status, const char* title, char* extra_header, const char* mime_type, off_t length, time_t mod);
static const char* get_mime_type(char* name);
static void strdecode(char* to, char* from);
static void strencode(char* to, size_t tosize, const char* from);
static int hexit(char c);

//Additional Helper function
typedef struct _HTTPRequest {
	int conn;
	char *method;
	char *path;
	char *protocol;
}HTTPRequest;
std::queue<HTTPRequest *> client_list;

#include <errno.h>
ssize_t read_line(int fd, void *buffer, size_t n)
{
	ssize_t numRead;                    /* # of bytes fetched by last read() */
	size_t  totRead;                     /* Total bytes read so far */
	char *buf;
	char ch;

	if (n <= 0 || buffer == NULL) {
		errno = EINVAL;
		return -1;
	}

	buf = (char *)buffer;                       /* No pointer arithmetic on "void *" */

	totRead = 0;
	for (; ; )
	{
		numRead = read(fd, &ch, 1);

		if (numRead == -1) {
			if (errno == EINTR)         /* Interrupted --> restart read() */
				continue;
			else
				return -1;              /* Some other error */
		}
		else if (numRead == 0) {      /* EOF */
			if (totRead == 0)           /* No bytes read; return 0 */
				return 0;
			else                        /* Some bytes read; add '\0' */
				break;
		}
		else {                        /* 'numRead' must be 1 if we get here */
			if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
				totRead++;
				*buf++ = ch;
			}
			if (ch == '\n') break;
		}
	}

	*buf = '\0';

	return totRead;
}

static void  setnonblockingmode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
int EpollAdd(const int fd_epoll, const int fd, char *ip)
{
	if (ip != NULL)
		printf("[Info] client %s connect\n", ip);

	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = fd;

	return epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd, &ev);
}

void * thr_func(void *arg)
{

	int clnt_sock;
	int len, ich, n;
	int i;
	bool requstflag = false;
	char *file;
	char temp[512];
	char buffer[512], path[512], location[512], idx[512];
	int flag = 0;
	struct dirent **dl;//뭔가 디렉토리 관련?

	FILE *fp;
	struct stat sb;//파일 정보 구조체
	HTTPRequest *request = NULL;

	while (1) {
		pthread_mutex_lock(&mutex);
		if (!client_list.empty()) {
			request = client_list.front();//들어 있으면 first in first out 조회하고
			client_list.pop();//삭제쓰
			requstflag = true;
		}
		pthread_mutex_unlock(&mutex);

		if (requstflag&&request) {
			clnt_sock = request->conn;

			if (strcasecmp(request->method, "get") != 0) {
				send_error(clnt_sock, 501, "Not Implemented", NULL, "That method is not implemented.");
				
				printf("[Info] client IP: %d closed\n", clnt_sock);
				epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
				close(clnt_sock);
				if (request) {
					free(request->method);
					free(request->path);
					free(request->protocol);
					free(request);
				}
				requstflag = false;
				continue;
			}

			if (request->path[0] != '/') {
				send_error(clnt_sock, 400, "Bad Request", NULL, "Bad filename.");
				
				printf("[Info] client IP: %d closed\n", clnt_sock);
				epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
				close(clnt_sock);
				if (request) {
					free(request->method);
					free(request->path);
					free(request->protocol);
					free(request);
				}
				requstflag = false;
				continue;
			}

			file = &(request->path[1]);
			strdecode(file, file);

			if (file[0] == '\0')
				file = "./";
			len = strlen(file);
			if (file[0] == '/' || strcmp(file, "..") == 0 || strncmp(file, "../", 3) == 0 || strstr(file, "/../") != NULL || (len >= 3 && strcmp(&(file[len - 3]), "/..") == 0)) {
				send_error(clnt_sock, 400, "Bad Request", NULL, "Illegal filename");
				printf("[Info] client IP: %d closed\n", clnt_sock);
				epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
				close(clnt_sock);
				if (request) {
					free(request->method);
					free(request->path);
					free(request->protocol);
					free(request);
				}
				requstflag = false;
				continue;
			}

			len = snprintf(temp, sizeof(temp), "%s/%s", DEFAULT_PATH, file);
			file = temp;

			// display file or file system status
			if (stat(file, &sb) < 0) {
				send_error(clnt_sock, 404, "Not Found", NULL, "File not found.");
				printf("[Info] client IP: %d closed\n", clnt_sock);
				epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
				close(clnt_sock);
				if (request) {
					free(request->method);
					free(request->path);
					free(request->protocol);
					free(request);
				}
				requstflag = false;
				continue;
			}
			
			//여기는 그대로..
			if (S_ISDIR(sb.st_mode)) {//디렉토리 파일인지 판별.  
				if (file[len - 1] != '/') {//만약 디렉토리고 끝이 / 이면  뭔가 잘못된거임.
					snprintf(location, sizeof(location), "Location: %s/", path);
					send_error(clnt_sock, 302, "Found", location, "Directories must end with a slash.");
					
					printf("[Info] client IP: %d closed\n", clnt_sock);
					epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
					close(clnt_sock);
					if (request) {
						free(request->method);
						free(request->path);
						free(request->protocol);
						free(request);
					}
					requstflag = false;

					continue;
				}

				snprintf(idx, sizeof(idx), "%sindex.html", file);//buffer에 %s 형식으로 넣어준다. file을
				if (stat(idx, &sb) < 0) { //고래서 매칭 되면 dofile함.
				//이건 뭘까? 기존 코드 놔두자.
					send_headers(clnt_sock, 200, "Ok", NULL, "text/html", -1, sb.st_mtime);
					snprintf(buffer, sizeof(buffer), "\
						you know what im saying?\
                        <!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\
                        <html>\n\
                        <head>\n\
                        <meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">\n\
                        <title>Index of %s</title>\n\
                        </head>\n\
                        <body bgcolor=\"#99cc99\">\n\
                        <h4>Index of %s</h4>\n\
                        <pre>\n", file, file);
					write(clnt_sock, buffer, strlen(buffer));
					n = scandir(file, &dl, NULL, alphasort);
					if (n < 0)
						perror("scandir");
					else
						for (i = 0; i < n; ++i)
							file_details(clnt_sock, file, dl[i]->d_name);

					snprintf(buffer, sizeof(buffer), "\
                        </pre>\n\
                        <hr>\n\
                        <address><a href=\"%s\">%s</a></address>\n\
                        </body>\n\
                        </html>\n", SERVER_URL, SERVER_NAME);
					write(clnt_sock, buffer, strlen(buffer));
					
					printf("[Info] client IP: %d closed\n", clnt_sock);
					epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
					close(clnt_sock);
					if (request) {
						free(request->method);
						free(request->path);
						free(request->protocol);
						free(request);
					}
					requstflag = false;
					
					continue;
				}

				file = idx;

			}

			fp = fopen(file, "r");
			if (fp == NULL) {
				send_error(clnt_sock, 403, "Forbidden", NULL, "File is protected.");
				printf("[Info] client IP: %d closed\n", clnt_sock);
				epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
				close(clnt_sock);
				if (request) {
					free(request->method);
					free(request->path);
					free(request->protocol);
					free(request);
				}
				requstflag = false;
				continue;
			}
			send_headers(clnt_sock, 200, "Ok", NULL, get_mime_type(file), sb.st_size, sb.st_mtime);
			while ((ich = getc(fp)) != EOF)
				write(clnt_sock, &ich, 1);//getchar 가능

			fclose(fp);

			printf("[Info] client IP: %d closed\n", clnt_sock);
			epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
			close(clnt_sock);
			if (request) {
				free(request->method);
				free(request->path);
				free(request->protocol);
				free(request);
			}
			requstflag = false;
		}
	}
}
char * tostring(char* buffer) {
	char* str;
	int str_len = strlen(buffer);
	str = (char*)malloc(sizeof(char)*(str_len + 1));
	strncpy(str, buffer, str_len);
	str[str_len] = '\0';
	return str;
}
void socket_init(char** argv)
{
	struct sockaddr_in serv_addr, clnt_addr;

	int serv_sock, clnt_sock;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;//주소 체계를 위한 변수 af_inet이면 sockaddr in 사용.
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1])); //호스트 바이트 순서를 바꿔준다.
	//inet addr ip-> 주소로 바꿔준다.
	//여기까지가 연결 준비

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	int enable = 1;
	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
		fprintf(stderr, "[ERROR] socket setting error\n");
		exit(-1);
	}

	if (bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
		fprintf(stderr, "[ERROR] socket binding error\n");
		exit(-1);
	}

	if (listen(serv_sock, 10) == -1) {
		fprintf(stderr, "[ERROR] socket listenning error\n");
		exit(-1);
	}
	 setnonblockingmode(serv_sock);

    
	epfd = epoll_create(EPOLL_SIZE);
	if (epfd > 0) {
		printf("[Info] create Epoll\n");
	}
    struct epoll_event ep_events[EPOLL_SIZE];
    EpollAdd(epfd, serv_sock, NULL);

    // master work single loop
    while(1) {
        int event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1) {
            fprintf(stderr, "[ERROR] epoll wait error");
            break;
        }

        printf("[Info] # of events: %d\n", event_cnt);

        for(int i = 0; i < event_cnt; i++) {
            if(ep_events[i].data.fd == serv_sock) {
                socklen_t addr_sz = sizeof(clnt_addr);

                clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &addr_sz);
               
                setnonblockingmode(clnt_sock);

				EpollAdd(epfd, clnt_sock, inet_ntoa(clnt_addr.sin_addr));
            } 
			else {
                char buffer[512];
                char method[512], path[512], protocol[512];

                int clnt_sock = ep_events[i].data.fd;

                int str_len;
                if ((str_len = read_line( clnt_sock, buffer, sizeof(buffer) ) ) == -1){
                    send_error( clnt_sock, 400, "Bad Request", NULL, "No request found." );
                    break;
                }

                if(str_len == 0){
					printf("[Info] client IP: %d closed\n", clnt_sock);
					epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
                    close(clnt_sock);
                }

                if(sscanf( buffer, "%[^ ] %[^ ] %[^ ]", method, path, protocol ) != 3){
                    send_error(clnt_sock, 400, "Bad Request", NULL, "Can't parse request.");
                    break;
                }

                while(read_line( clnt_sock, buffer, sizeof(buffer)) != 0) {
                    if(strcmp( buffer, "\n" ) == 0 || strcmp( buffer, "\r\n" ) == 0)
                        break;
                }

                printf("%s %s %s\n", method, path, protocol);
				HTTPRequest *new_Task = (HTTPRequest*)malloc(sizeof(HTTPRequest));
				new_Task->conn = clnt_sock;
				new_Task->method = tostring(method);
				new_Task->path = tostring(path);
				new_Task->protocol = tostring(protocol);

                
                pthread_mutex_lock(&mutex);
                client_list.push(new_Task);
                pthread_mutex_unlock(&mutex);
            }
        }
    }
	close(serv_sock);
}

void thread_init(char **argv)
{
	int thread_nums = atoi(argv[2]);		//쓰레두 수설정 
	pthread_t threadPool[thread_nums];	//pthread 설정

	for (int i = 0; i < thread_nums; i++) {//만들어 주시고.
		pthread_create(&threadPool[i], NULL, thr_func, NULL);
		pthread_detach(threadPool[i]);
	}

	pthread_mutex_init(&mutex, NULL);//init 해 주시고 

}
int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;

	if (argc != 3) {
		printf("[Usage] %s <port> <threads_num>\n", argv[0]);
		exit(-1);
	}

	thread_init(argv);
	socket_init(argv);

	//연결 다해주고...
	//printf("어디인가?");
	// master work single loop

	//기둘려 준다.


	return 0;
}
static void
file_details(int clnt_sock, char* dir, char* name)
{
	static char encoded_name[1000];
	static char path[2000];
	struct stat sb;
	char timestr[16];
	char buffer[512];

	strencode(encoded_name, sizeof(encoded_name), name);

	snprintf(path, sizeof(path), "%s/%s", dir, name);
	if (lstat(path, &sb) < 0) {
		snprintf(buffer, sizeof(buffer), "<a href=\"%s\">%-32.32s</a>    ???\n", encoded_name, name);
		write(clnt_sock, buffer, strlen(buffer));
	}

	else {
		strftime(timestr, sizeof(timestr), "%d%b%Y %H:%M", localtime(&sb.st_mtime));
		snprintf(buffer, sizeof(buffer), "<a href=\"%s\">%-32.32s</a>    %15s %14lld\n", encoded_name, name, timestr, (long long)sb.st_size);
		write(clnt_sock, buffer, strlen(buffer));
	}
}

static void
send_error(int clnt_sock, int status, const char* title, char* extra_header, const char* text)
{
	char msg[1024];
	send_headers(clnt_sock, status, title, extra_header, "text/html", -1, -1);

	snprintf(msg, sizeof(msg), "\
            <!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\
            <html>\n\
            <head>\n\
            <meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">\n\
            <title>%d %s</title>\n\
            </head>\n\
            <body bgcolor=\"#cc9999\">\n\
            <h4>%d %s</h4>\n", status, title, status, title);
	write(clnt_sock, msg, strlen(msg));

	snprintf(msg, sizeof(msg), "%s\n", text);
	write(clnt_sock, msg, strlen(msg));

	snprintf(msg, sizeof(msg), "\
            <hr>\n\
            <address><a href=\"%s\">%s</a></address>\n\
            </body>\n\
            </html>\n", SERVER_URL, SERVER_NAME);
	write(clnt_sock, msg, strlen(msg));
	return;
}

static void
send_headers(int clnt_sock, int status, const char* title, char* extra_header, const char* mime_type, off_t length, time_t mod)
{
	time_t now;
	char timebuf[100];
	char msg[1024];

	snprintf(msg, sizeof(msg), "%s %d %s\015\012", PROTOCOL, status, title);
	write(clnt_sock, msg, strlen(msg));

	snprintf(msg, sizeof(msg), "Server: %s\015\012", SERVER_NAME);
	write(clnt_sock, msg, strlen(msg));

	now = time((time_t*)0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	snprintf(msg, sizeof(msg), "Date: %s\015\012", timebuf);
	write(clnt_sock, msg, strlen(msg));

	if (extra_header) {
		snprintf(msg, sizeof(msg), "%s\015\012", extra_header);
		write(clnt_sock, msg, strlen(msg));
	}

	if (mime_type) {
		snprintf(msg, sizeof(msg), "Content-Type: %s\015\012", mime_type);
		write(clnt_sock, msg, strlen(msg));
	}

	if (length >= 0) {
		snprintf(msg, sizeof(msg), "Content-Length: %lld\015\012", (long long)length);
		write(clnt_sock, msg, strlen(msg));
	}

	if (mod != (time_t)-1) {
		(void)strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		snprintf(msg, sizeof(msg), "Last-Modified: %s\015\012", timebuf);
		write(clnt_sock, msg, strlen(msg));
	}

	snprintf(msg, sizeof(msg), "Connection: close\015\012");
	write(clnt_sock, msg, strlen(msg));
	snprintf(msg, sizeof(msg), "\015\012");
	write(clnt_sock, msg, strlen(msg));
}

static const char*
get_mime_type(char* name)
{
	char* dot;

	dot = strrchr(name, '.');
	if (dot == (char*)0)
		return "text/plain; charset=UTF-8";
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=UTF-8";
	if (strcmp(dot, ".xhtml") == 0 || strcmp(dot, ".xht") == 0)
		return "application/xhtml+xml; charset=UTF-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".xml") == 0 || strcmp(dot, ".xsl") == 0)
		return "text/xml; charset=UTF-8";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";
	return "text/plain; charset=UTF-8";
}

static void
strdecode(char* to, char* from)
{
	for (; *from != '\0'; ++to, ++from)
	{
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
		{
			*to = hexit(from[1]) * 16 + hexit(from[2]);
			from += 2;
		}
		else
			*to = *from;
	}
	*to = '\0';
}

static int
hexit(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;       /* shouldn't happen, we're guarded by isxdigit() */
}

static void
strencode(char* to, size_t tosize, const char* from)
{
	int tolen;

	for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from)
	{
		if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0)
		{
			*to = *from;
			++to;
			++tolen;
		}
		else
		{
			(void)sprintf(to, "%%%02x", (int)*from & 0xff);
			to += 3;
			tolen += 3;
		}
	}
	*to = '\0';
}
