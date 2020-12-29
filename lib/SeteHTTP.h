#pragma once
#ifndef SETEHTTP_H
#define SETEHTTP_H

class SeteHTTP
{
public:
	SeteHTTP(const char* host, const char* port);
	~SeteHTTP();
    int request(const char* type, const char* endpoint, const char* body, char* buffer);

private:
	const char* host;
	const char* port;
	char* req;
	char* res;
	void clearbuffers();
};

#endif
