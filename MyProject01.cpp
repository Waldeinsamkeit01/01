#define _CRT_SERCURE_NO_WARNINGS_
#pragma warning(disable:4996)
#define bufsize 1024
#define SMTP_BUFSIZE 1024

#pragma comment(lib, "ws2_32.lib")
#include <Winsock2.h>
#include <Windows.h>
#include <Ws2tcpip.h>
#include <iostream>
#pragma warning(disable:4996)
#include "mysql.h"

#pragma comment(lib,"libmysql.lib")



using namespace std;

MYSQL* mysql = new MYSQL;//mysql链接
MYSQL_FIELD* fd;	//字段列数组
char field[32][32];	//存字段名的二维数组
MYSQL_RES* res;		//这个结构代表返回行的一个查询结果集
MYSQL_ROW column;	//一个行数据类型安全(type-safe)的表示，表示数据行的列
char query[150];	//查询语句

bool ConnectDatabase();
bool QueryDatabase();
bool ModifyData(char* data);
void EncodeBase64(char* src, char* encode);                             // Base64编码
int SendMail(char* from, char* pwd, char* to, char* title, char* text); // 发送邮件


bool ConnectDatabase() {
	mysql_init(mysql);//初始化
	if (!(mysql_real_connect(mysql, "localhost", "root", "1101101011Zsm", "JSP", 3306, NULL, 0)))//中间分别是主机，用户名，密码，数据库名，端口号（可以写默认0或者3306等），可以先写成参数再传进去
	{
		cout << "Error connect to database:" << mysql_error(mysql) << endl;
		return false;
	}
	else {
		cout << "Connect......" << endl;
		return true;

	}
	return true;
}

bool QueryDatabase() {
	sprintf_s(query, "select * from websites");//执行查询语句
	mysql_query(mysql, "set names gbk");//设置编码格式
	//返回0 查询成功，返回1查询失败 
	if (mysql_query(mysql, query)) {
		cout << "Error connect to query:" << mysql_errno(mysql) << endl;
		return false;
	}
	else {
		cout << "query success" << endl;
		
	}
	//获取结果集 
	if (!(res = mysql_store_result(mysql))) { //获得sql语句结束后返回的结果集  
		cout << "Can't get result from" << mysql_errno(mysql) << endl;
		return false;
	}

	char* str_field[32];  //定义一个字符串数组存储字段信息  
	for (int i = 0; i < 4; i++)  //在已知字段数量的情况下获取字段名  
	{
		str_field[i] = mysql_fetch_field(res)->name;
	}
	for (int i = 0; i < 4; i++)  //打印字段  
		cout << str_field[i]<<"\t\t";
	cout << endl;
	//打印获取的数据  
	while (column = mysql_fetch_row(res))   //在已知字段数量情况下，获取并打印下一行  
	{
		//printf("%10s\t%10s\t%10s\t%10s\n", column[0], column[1], column[2], column[3]);  //column是列数组  
		for (int i = 0; i < 4; i++) {
			cout << column[i] << "\t\t";
		}
		cout << endl;
	}
	return true;
}

bool InsertDatabase(char* data) {
	char buf[2048];
	sprintf(buf,"insert into html(webname, html) values('aqwu.net', '%s')", data);
	if (!(mysql_query(mysql,buf))) {
		cout << "already insert" << endl;
		return true;
	}
	else {
		cout << "Insert error " << mysql_errno(mysql);
		return false;
	}

}

bool ModifyData(char* data)
{
	char buf[2048];
	sprintf(buf,"UPDATE html SET html= CONCAT(html,'%s');", data);
	if (mysql_query(mysql, buf))        //执行SQL语句  
	{
		printf("Query failed (%s)\n", mysql_error(mysql));
		return false;
	}
	else
	{
		printf("Insert success\n");
		return true;
	}
}


void ReadPage(const char* host)
{
	WSADATA data;
	//winsock版本2.2
	int err = WSAStartup(MAKEWORD(2, 2), &data);
	if (err)
		return;

	//用域名获取对方主机名
	struct hostent* h = gethostbyname(host);
	if (h == NULL)
		return;

	//IPV4
	if (h->h_addrtype != AF_INET)
		return;
	struct in_addr ina;
	//解析IP
	memmove(&ina, h->h_addr, 4);
	LPSTR ipstr = inet_ntoa(ina);

	//Socket封装
	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = htons(80);
	si.sin_addr.S_un.S_addr = inet_addr(ipstr);
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	connect(sock, (SOCKADDR*)&si, sizeof(si));
	if (sock == -1 || sock == -2)
		return;

	//发送请求
	char request[1024] = "GET /?st=1 HTTP/1.1\r\nHost:";
	strcat(request, host);
	strcat(request, "\r\nConnection:Close\r\n\r\n");
	int ret = send(sock, request, strlen(request), 0);
	//获取网页内容
	int flag = 0;//是否为首次写入
	ConnectDatabase();
	while (ret > 0)
	{
		
		char* buf = (char*)calloc(bufsize, 1);
		ret = recv(sock, buf, bufsize - 1, 0);
	
		if(flag==0){
			InsertDatabase(buf);
			flag++;
		}
		else {
			ModifyData(buf);
		}
		free(buf);
	}
	closesocket(sock);
	WSACleanup();
	printf("读取网页内容成功，已保存在数据库中");
	return;
}

// Base64编码
void EncodeBase64(char* src, char* encode)
{
	char base64_table[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '=' };
	int len = strlen(src);
	int i = 0;
	for (i = 0; i < len / 3; i++)
	{
		int temp = byte(src[3 * i + 2]) + (byte(src[3 * i + 1]) << 8) + (byte(src[3 * i]) << 16);
		encode[4 * i] = base64_table[(temp & 0xfc0000) >> 18];
		encode[4 * i + 1] = base64_table[(temp & 0x3f000) >> 12];
		encode[4 * i + 2] = base64_table[(temp & 0xfc0) >> 6];
		encode[4 * i + 3] = base64_table[temp & 0x3f];
	}
	encode[4 * i] = 0;
	if (1 == len % 3)
	{
		int temp = byte(src[3 * i]) << 16;
		encode[4 * i] = base64_table[(temp & 0xfc0000) >> 18];
		encode[4 * i + 1] = base64_table[(temp & 0x3f000) >> 12];
		encode[4 * i + 2] = base64_table[64];
		encode[4 * i + 3] = base64_table[64];
		encode[4 * i + 4] = 0;
	}
	else if (2 == len % 3)
	{
		int temp = (byte(src[3 * i + 1]) << 8) + (byte(src[3 * i]) << 16);
		encode[4 * i] = base64_table[(temp & 0xfc0000) >> 18];
		encode[4 * i + 1] = base64_table[(temp & 0x3f000) >> 12];
		encode[4 * i + 2] = base64_table[(temp & 0xfc0) >> 6];
		encode[4 * i + 3] = base64_table[64];
		encode[4 * i + 4] = 0;
	}
}

// 发送邮件
int SendMail(char* from, char* pwd, char* to, char* title, char* text)
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	char buf[SMTP_BUFSIZE] = { 0 };
	char account[128] = { 0 };
	char password[128] = { 0 };
	// 连接邮件服务器
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(25);
	hostent* phost = gethostbyname("smtp.163.com");
	memcpy(&addr.sin_addr.S_un.S_addr, phost->h_addr_list[0], phost->h_length);
	SOCKET sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("smtp socket() error");
		return 1;
	}
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0)
	{
		printf("smtp connect() error");
		return 2;
	}
	// EHLO
	char pcname[128] = { 0 };
	DWORD size = 128;
	GetComputerName(pcname, &size); // 获取计算机名
	sprintf_s(buf, SMTP_BUFSIZE, "EHLO %s\r\n", pcname);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// AUTH LOGIN
	sprintf_s(buf, SMTP_BUFSIZE, "AUTH LOGIN\r\n");
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// 邮箱账号
	EncodeBase64(from, account);
	sprintf_s(buf, SMTP_BUFSIZE, "%s\r\n", account);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// 密码
	EncodeBase64(pwd, password);
	sprintf_s(buf, SMTP_BUFSIZE, "%s\r\n", password);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// MAIL FROM 发件人
	sprintf_s(buf, SMTP_BUFSIZE, "MAIL FROM:<%s>\r\n", from);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// RCPT TO 收件人
	sprintf_s(buf, SMTP_BUFSIZE, "RCPT TO:<%s>\r\n", to);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// DATA 准备开始发送邮件内容
	sprintf_s(buf, SMTP_BUFSIZE, "DATA\r\n");
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// 发送邮件内容
	sprintf_s(buf, SMTP_BUFSIZE,
		"From: \"Lemon00\"<%s>\r\nTo: \"test\"<%s>\r\nSubject: %s\r\n\r\n%s\r\n.\r\n", from, to, title, text);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// QUIT 结束
	sprintf_s(buf, SMTP_BUFSIZE, "QUIT\r\n");
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	if (strlen(buf) >= 3)
	{
		if (buf[0] == '2' && buf[1] == '5' && buf[2] == '0')
		{
			printf("sucess sent a mail\n");
		}
	}
	closesocket(sockfd);
	WSACleanup();
	system("pause");
	return 0;
}

int main() {
	try {
		ReadPage("www.baidu.com");
		SendMail("13260620311@163.com", "JOUKRRSLETMGNVLG", "875141750@qq.com", "Hello", "Hi，这封邮件来自张书萌,是我写的程序发出来的！");
		return 0;
	}
	catch(exception e) {
		cout << "Error "<< endl;
		SendMail("13260620311@163.com", "JOUKRRSLETMGNVLG", "875141750@qq.com", "Hello", "Hi，这封邮件来自Lemon00，您的程序似乎出现了一些问题！");
		return -1;
	}
	
}





