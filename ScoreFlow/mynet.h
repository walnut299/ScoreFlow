#pragma once
#define BUFFER_SIZE 4096
#define TOTLA_BUFFER_SIZE BUFFER_SIZE * 5
typedef struct {
	SOCKET socket;
	char   buffer[TOTLA_BUFFER_SIZE];
} Connector;

typedef enum HTMLRequestMethod {
	GET,
	POST
} HTMLRequestMethod;

typedef struct {
	HTMLRequestMethod method;
	char host[40];
	char url[256];
	cJSON* data;
} HTMLParseObject;

int HandleTcpRecv(Connector* con);
BOOL HTTPParse(const char* buffer, HTMLParseObject* obj);
void HandleRequest(Connector* con, HTMLParseObject* obj);