/*

███████╗███████╗████████╗███████╗██╗  ██╗████████╗████████╗██████╗
██╔════╝██╔════╝╚══██╔══╝██╔════╝██║  ██║╚══██╔══╝╚══██╔══╝██╔══██╗
███████╗█████╗     ██║   █████╗  ███████║   ██║      ██║   ██████╔╝
╚════██║██╔══╝     ██║   ██╔══╝  ██╔══██║   ██║      ██║   ██╔═══╝
███████║███████╗   ██║   ███████╗██║  ██║   ██║      ██║   ██║
╚══════╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝   ╚═╝      ╚═╝   ╚═╝

=============================================
    USAGE EXAMPLE:
=============================================
	#include <stdio.h>
	#include "SeteHTTP.h"

	int main(void) {

		SeteHTTP http("localhost", "8080");
		char res[4096] = { 0 };

		// GET
		if (http.request("GET", "/", 0, res)) {
			printf("Response:\r\n%s", res);
		}
		else {
			printf("Error");
		}

		// POST
		const char* body = "test=value&cool=another";
		if (http.request("POST", "/", body, res)) {
			printf("Response:\r\n%s", res);
		}
		else {
			printf("Error");
		}

		getchar();
	}
*/

// WINDOWS
#ifdef _WIN32
#define _WIN32_WINNT 0x501
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

// LINUX
#ifndef _WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

// COMMON
#include "SeteHTTP.h"


// Configuration
#define REQ_BUFFER_LEN  2^20 // 1 MiB
#define RES_BUFFER_LEN  2^20 // 1 MiB


// Constructor
SeteHTTP::SeteHTTP(const char* host, const char* port)
{
	// Set connection vars
	this->host = host;
	this->port = port;

	// Allocate space for requests
	this->req = (char*)malloc(REQ_BUFFER_LEN);

	// Allocate space for responses
	this->res = (char*)malloc(RES_BUFFER_LEN);
}


// Destructor
SeteHTTP::~SeteHTTP()
{
	this->clearbuffers();
	free(this->req);
	free(this->res);
}


// Zeroes out dynamically allocated memory
void SeteHTTP::clearbuffers()
{
	memset(this->req, 0, REQ_BUFFER_LEN);
	memset(this->res, 0, RES_BUFFER_LEN);
}


// Makes either a GET or POST request
int SeteHTTP::request(const char* type, const char* endpoint, const char* body, char* buffer)
{
	SOCKET sock;
	WSADATA wsa;

	// Clear memory
	this->clearbuffers();

	// Start winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		this->clearbuffers();
		return 0;
	}

	// Get address info
	struct addrinfo *addr;
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("localhost", "8080", &hints, &addr) != 0) {
		this->clearbuffers();
		return 0;
	}

	// Create socket
	sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (sock == INVALID_SOCKET) {
		this->clearbuffers();
		return 0;
	}

	// Connect
	if (connect(sock, addr->ai_addr, addr->ai_addrlen) != 0) {
		this->clearbuffers();
		return 0;
	}

	// We are connected
	printf("Connected.\r\n\r\n");


	// Will contain the length of the HTTP header we create below
	int len = 0;

	// GET
	if (strcmp(type, "GET") == 0)
	{
		// Prepare header
		len += sprintf(this->req + len, "GET %s HTTP/1.1\r\n", endpoint);
		len += sprintf(this->req + len, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; AS; rv:11.0) like Gecko\r\n"); // IE11
		len += sprintf(this->req + len, "Host: %s\r\n", this->host);
		len += sprintf(this->req + len, "Accept-Language: en-us\r\n");
		len += sprintf(this->req + len, "Accept-Charset: utf-8\r\n");
		len += sprintf(this->req + len, "Content-Type: text/plain\r\n");
		len += sprintf(this->req + len, "\r\n"); // Extra newline required
	}

	// POST
	if (strcmp(type, "POST") == 0)
	{
		// Prepare header
		len += sprintf(this->req + len, "POST %s HTTP/1.1\r\n", endpoint);
		len += sprintf(this->req + len, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; AS; rv:11.0) like Gecko\r\n"); // IE11
		len += sprintf(this->req + len, "Host: %s\r\n", this->host);
		len += sprintf(this->req + len, "Accept-Language: en-us\r\n");
		len += sprintf(this->req + len, "Accept-Charset: utf-8\r\n");
		len += sprintf(this->req + len, "Content-Type: application/x-www-form-urlencoded\r\n");
		len += sprintf(this->req + len, "Content-Length: %d\r\n\n", strlen(body));
		len += sprintf(this->req + len, "%s\r\n", body);
		len += sprintf(this->req + len, "\r\n"); // Extra newline required
	}

	// Print out the request
	printf("Request:\r\n");
	printf("%s\n\n", req);

	// Send
	if (send(sock, req, len, 0) < 0) {
		this->clearbuffers();
		printf("Send failed with error code : %d\n", WSAGetLastError());
		return 0;
	}
	puts("Data sent\r\n\r\n");

	// Response
	int received;
	if ((received = recv(sock, this->res, 2000, 0)) == SOCKET_ERROR) {
		this->clearbuffers();
		printf("Receive failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	// Make sure we got something
	if (received == 0) {
		this->clearbuffers();
		printf("No data received\r\n\r\n");
		return 0;
	}

	// Cut the tail of the response
	this->res[received] = '\0';

	// Copy response into return buffer
	strcpy(buffer, this->res);

	// Close the socket
	freeaddrinfo(addr);
	closesocket(sock);

	// Clean memory again
	this->clearbuffers();

	// Return success
	return 1;
}


#ifndef _WIN32
int i;

struct hostent *server;
struct sockaddr_in serv_addr;
int sockfd, bytes, sent, received, total, message_size;
char *message, response[4096];

/* How big is the message? */
message_size = 0;
if (!strcmp(method, "GET")) {
	message_size += strlen("%s %s%s%s HTTP/1.0\r\n");        /* method         */
	message_size += strlen(method);                         /* path           */
	message_size += strlen(argv[4]);                         /* headers        */
	if (argc > 5)
		message_size += strlen(argv[5]);                     /* query string   */
	for (i = 6; i < argc; i++)                                    /* headers        */
		message_size += strlen(argv[i]) + strlen("\r\n");
	message_size += strlen("\r\n");                          /* blank line     */
}
else {
	message_size += strlen("%s %s HTTP/1.0\r\n");
	message_size += strlen(method);                         /* method         */
	message_size += strlen(argv[4]);                         /* path           */
	for (i = 6; i < argc; i++)                                    /* headers        */
		message_size += strlen(argv[i]) + strlen("\r\n");
	if (argc > 5)
		message_size += strlen("Content-Length: %d\r\n") + 10; /* content length */
	message_size += strlen("\r\n");                          /* blank line     */
	if (argc > 5)
		message_size += strlen(argv[5]);                     /* body           */
}

/* allocate space for the message */
message = (char *)malloc(message_size);

/* fill in the parameters */
if (!strcmp(method, "GET")) {
	if (argc > 5)
		sprintf(message, "%s %s%s%s HTTP/1.0\r\n",
			strlen(method) > 0 ? method : "GET",               /* method         */
			strlen(argv[4]) > 0 ? argv[4] : "/",                 /* path           */
			strlen(argv[5]) > 0 ? "?" : "",                      /* ?              */
			strlen(argv[5]) > 0 ? argv[5] : "");                 /* query string   */
	else
		sprintf(message, "%s %s HTTP/1.0\r\n",
			strlen(method) > 0 ? method : "GET",               /* method         */
			strlen(argv[4]) > 0 ? argv[4] : "/");                /* path           */
	for (i = 6; i < argc; i++)                                    /* headers        */
	{
		strcat(message, argv[i]);
		strcat(message, "\r\n");
	}
	strcat(message, "\r\n");                                /* blank line     */
}
else {
	sprintf(message, "%s %s HTTP/1.0\r\n",
		strlen(method) > 0 ? method : "POST",                  /* method         */
		strlen(argv[4]) > 0 ? argv[4] : "/");                    /* path           */
	for (i = 6; i < argc; i++)                                    /* headers        */
	{
		strcat(message, argv[i]);
		strcat(message, "\r\n");
	}
	if (argc > 5)
		sprintf(message + strlen(message), "Content-Length: %d\r\n", strlen(argv[5]));
	strcat(message, "\r\n");                                /* blank line     */
	if (argc > 5)
		strcat(message, argv[5]);                           /* body           */
}

/* What are we going to send? */
printf("Request:\n%s\n", message);

/* create the socket */
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) error("ERROR opening socket");

/* lookup the ip address */
server = gethostbyname(host);
if (server == NULL) error("ERROR, no such host");

/* fill in the structure */
memset(&serv_addr, 0, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(portno);
memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

/* connect the socket */
if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	error("ERROR connecting");

/* send the request */
total = strlen(message);
sent = 0;
do {
	bytes = write(sockfd, message + sent, total - sent);
	if (bytes < 0)
		error("ERROR writing message to socket");
	if (bytes == 0)
		break;
	sent += bytes;
} while (sent < total);

/* receive the response */
memset(response, 0, sizeof(response));
total = sizeof(response) - 1;
received = 0;
do {
	bytes = read(sockfd, response + received, total - received);
	if (bytes < 0)
		error("ERROR reading response from socket");
	if (bytes == 0)
		break;
	received += bytes;
} while (received < total);

if (received == total)
error("ERROR storing complete response from socket");

/* close the socket */
close(sockfd);

/* process response */
printf("Response:\n%s\n", response);

free(message);
return 0;
#endif


#ifdef xxx


#ifndef _WIN32
int i;

struct hostent *server;
struct sockaddr_in serv_addr;
int sockfd, bytes, sent, received, total, message_size;
char *message, response[4096];

/* How big is the message? */
message_size = 0;
if (!strcmp(method, "GET")) {
	message_size += strlen("%s %s%s%s HTTP/1.0\r\n");        /* method         */
	message_size += strlen(method);                         /* path           */
	message_size += strlen(argv[4]);                         /* headers        */
	if (argc > 5)
		message_size += strlen(argv[5]);                     /* query string   */
	for (i = 6; i < argc; i++)                                    /* headers        */
		message_size += strlen(argv[i]) + strlen("\r\n");
	message_size += strlen("\r\n");                          /* blank line     */
}
else {
	message_size += strlen("%s %s HTTP/1.0\r\n");
	message_size += strlen(method);                         /* method         */
	message_size += strlen(argv[4]);                         /* path           */
	for (i = 6; i < argc; i++)                                    /* headers        */
		message_size += strlen(argv[i]) + strlen("\r\n");
	if (argc > 5)
		message_size += strlen("Content-Length: %d\r\n") + 10; /* content length */
	message_size += strlen("\r\n");                          /* blank line     */
	if (argc > 5)
		message_size += strlen(argv[5]);                     /* body           */
}

/* allocate space for the message */
message = (char *)malloc(message_size);

/* fill in the parameters */
if (!strcmp(method, "GET")) {
	if (argc > 5)
		sprintf(message, "%s %s%s%s HTTP/1.0\r\n",
			strlen(method) > 0 ? method : "GET",               /* method         */
			strlen(argv[4]) > 0 ? argv[4] : "/",                 /* path           */
			strlen(argv[5]) > 0 ? "?" : "",                      /* ?              */
			strlen(argv[5]) > 0 ? argv[5] : "");                 /* query string   */
	else
		sprintf(message, "%s %s HTTP/1.0\r\n",
			strlen(method) > 0 ? method : "GET",               /* method         */
			strlen(argv[4]) > 0 ? argv[4] : "/");                /* path           */
	for (i = 6; i < argc; i++)                                    /* headers        */
	{
		strcat(message, argv[i]);
		strcat(message, "\r\n");
	}
	strcat(message, "\r\n");                                /* blank line     */
}
else {
	sprintf(message, "%s %s HTTP/1.0\r\n",
		strlen(method) > 0 ? method : "POST",                  /* method         */
		strlen(argv[4]) > 0 ? argv[4] : "/");                    /* path           */
	for (i = 6; i < argc; i++)                                    /* headers        */
	{
		strcat(message, argv[i]);
		strcat(message, "\r\n");
	}
	if (argc > 5)
		sprintf(message + strlen(message), "Content-Length: %d\r\n", strlen(argv[5]));
	strcat(message, "\r\n");                                /* blank line     */
	if (argc > 5)
		strcat(message, argv[5]);                           /* body           */
}

/* What are we going to send? */
printf("Request:\n%s\n", message);

/* create the socket */
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) error("ERROR opening socket");

/* lookup the ip address */
server = gethostbyname(host);
if (server == NULL) error("ERROR, no such host");

/* fill in the structure */
memset(&serv_addr, 0, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(portno);
memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

/* connect the socket */
if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	error("ERROR connecting");

/* send the request */
total = strlen(message);
sent = 0;
do {
	bytes = write(sockfd, message + sent, total - sent);
	if (bytes < 0)
		error("ERROR writing message to socket");
	if (bytes == 0)
		break;
	sent += bytes;
} while (sent < total);

/* receive the response */
memset(response, 0, sizeof(response));
total = sizeof(response) - 1;
received = 0;
do {
	bytes = read(sockfd, response + received, total - received);
	if (bytes < 0)
		error("ERROR reading response from socket");
	if (bytes == 0)
		break;
	received += bytes;
} while (received < total);

if (received == total)
error("ERROR storing complete response from socket");

/* close the socket */
close(sockfd);

/* process response */
printf("Response:\n%s\n", response);

free(message);
return 0;
#endif


#endif