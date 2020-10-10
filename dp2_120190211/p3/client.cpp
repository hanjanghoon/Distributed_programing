#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <vector>

#define BUFFER_SIZE    2048
#define NAME_LEN	   10

typedef struct _clnt_soket {
	struct sockaddr_in serv_addr;
	int request_num;
}clnt_socket;

std::vector<char *> file_list;

pthread_mutex_t mutex;
static int iterations = 0;
static int total_bytes = 0;
static int no_of_files = 0;

char * tostring(char* buffer) {
	char* str;
	int str_len = strlen(buffer);
	str = (char*)malloc(sizeof(char)*(str_len + 1));
	strncpy(str, buffer, str_len);
	str[str_len] = '\0';
	return str;
}
void make_file_list(){
	
	char file_name[NAME_LEN];
	char buffer[BUFFER_SIZE];

	FILE *fp = fopen("request.txt", "r");//���� ������.
	if (fp == NULL) {
		fprintf(stderr, "[ERROR] no file open\n");
		exit(1);
	}//�ȵǸ� ����

	while (!feof(fp)) {//����������
		fgets(buffer, sizeof(buffer), fp);//�Է¹ް�.
		sscanf(buffer, "%s\n", file_name);//���۸� �Է¹����� ��Ʈ�� �������� ���ϳ��ӿ� ����.
		file_list.push_back(tostring(file_name));//vector �� ����. ������ ������ �����ϴ°���.���� �̸� ������ִ°���.
		//printf("vec:%s\n", file_list.begin());
	}
	return;
}

clnt_socket socket_init(char** argv)
{	
	clnt_socket box;
	//struct sockaddr_in serv_addr;
	memset(&box.serv_addr, 0, sizeof(box.serv_addr));
	box.serv_addr.sin_family = AF_INET;//�ּ� ü�踦 ���� ���� af_inet�̸� sockaddr in ���.
	box.serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	box.serv_addr.sin_port = htons(atoi(argv[2])); //ȣ��Ʈ ����Ʈ ������ �ٲ��ش�.
	//inet addr ip-> �ּҷ� �ٲ��ش�.
	//��������� ���� �غ�.
	box.request_num = atoi(argv[4]);
	return box;
}


void* thr_func(void * arg) {
	clnt_socket *box = (clnt_socket*)arg;
	int temp;


	for (int i = 0; i < box->request_num; ++i) {
		char buffer[BUFFER_SIZE];
		int sock = socket(PF_INET, SOCK_STREAM, 0);//���� ����

		pthread_mutex_lock(&mutex);//�� �ɾ��ְ�
		if (connect(sock, (struct sockaddr *) &(box->serv_addr), sizeof(box->serv_addr)) == -1) {
			fprintf(stderr, "[ERROR] no connect\n");
			pthread_mutex_unlock(&mutex);
			continue;
			//pthread_exit((void *)1);
		}//�ȵǸ� ����
		pthread_mutex_unlock(&mutex);
		
		//printf("filelistsize %d\n", file_list.size());
		//pthread_exit((void *)0);
		int index = i % file_list.size();//requetst list �� �ִ� �� ����.
		
		
		
		snprintf(buffer, sizeof(buffer), "GET /%s HTTP/1.1\r\n\r\n", file_list[index]);
	
		puts("\n[Info] client http request : ");
		write(sock, buffer, strlen(buffer));//���Ͽ� ����
		write(1, buffer, strlen(buffer));//�̰� ����? �̰� ����ϴ°��ε�.
		
		pthread_mutex_lock(&mutex);
		puts("\n[Info] client http response : ");
		int len, temp;
		while ((len = read(sock, buffer, sizeof(buffer))) > 0) {
			if (write(1, buffer, len) < 0) break;
			fflush(stdin);
			temp += len;
		}
		pthread_mutex_unlock(&mutex);

		close(sock);

		pthread_mutex_lock(&mutex);
		iterations += 1;
		total_bytes += temp;
		pthread_mutex_unlock(&mutex);
	}

	puts("\t[Debug] THREAD END");
	pthread_exit((void *)0);
}
double thread_init(char **argv, clnt_socket box)
{
	void *tret;
	int threads_num = atoi(argv[3]);
	pthread_t tids[threads_num];//������ �������
	pthread_mutex_init(&mutex, NULL);//���ؽ� ����.
	
	clock_t start = clock();
	
	
	for (int i = 0; i < threads_num; ++i)
		pthread_create(&tids[i], NULL, thr_func, (void *)&box);//threadwork�� ���� ��Ű�鼭 ������.

	for (int i = 0; i < threads_num; ++i)
		pthread_join(tids[i], &tret);//���� �Ǵ°��� ��ٸ���. tret�� ���� status �ε�.

	double result = (double)(clock() - start) / CLOCKS_PER_SEC; //�ð����.
	
	return result;
}
int main(int argc, char **argv)
{
	double result;
	if (argc != 5) {
		printf("[Usage] %s <ip> <port> <threads_num> <request_num>\n", argv[0]);
		exit(1);
	}
	make_file_list();
	clnt_socket box;
	box = socket_init(argv);
	
	result = thread_init(argv,box);
	
	// Finalize
	for (int i = 0; i < file_list.size(); ++i)
		free(file_list[i]);
	pthread_mutex_destroy(&mutex);

	//time print
	printf("[Info] Total iterations: %d\n", iterations);
	printf("[Info] Total %d bytes received\n", total_bytes);
	printf("[Info] Total time: %5.6f second\n", result);
	

	

	return 0;
}
