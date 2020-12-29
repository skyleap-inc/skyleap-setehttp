#include "../../lib/SeteHTTP.h"
#include <stdio.h>

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

