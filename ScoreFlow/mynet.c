#include "stdafx.h"
#include "utlis.h"

int selectList(void* json_array, int argc, char** argv, char** azColName) {
    cJSON* json_row = cJSON_CreateObject();
    for (int i = 0; i < argc; i++) {
        cJSON_AddStringToObject(json_row, azColName[i], argv[i] ? argv[i] : "NULL");
    }
    cJSON_AddItemToArray((cJSON*)json_array, json_row);
    return 0;
}

int HandleTcpRecv(Connector* con) {
    memset(con->buffer, 0, sizeof(con->buffer));
	int bytesRead = recv(con->socket, con->buffer, TOTLA_BUFFER_SIZE,0);
    if (bytesRead > 0) {
        HTMLParseObject htmlObj;
        memset(&htmlObj, 0, sizeof(htmlObj));
        HTTPParse(con->buffer,&htmlObj);
        HandleRequest(con, &htmlObj);
    }
    return bytesRead;
}

BOOL HTTPParse(const char* buffer, HTMLParseObject* obj)
{
    int splitSize = 0;
    char** httpLine = Split(buffer, "\r\n", &splitSize);
    if (httpLine == NULL)
        return FALSE;

    int httpLine0Size = 0;
    int httpLine1Size = 0;
    char** httpLine0 = Split(httpLine[0]," ",&httpLine0Size);
    char** httpLine1 = Split(httpLine[1]," ",&httpLine1Size);
    if (strcmp(httpLine0[0], "GET") == 0)  
        obj->method = GET;
    else
    {
        obj->method = POST;
        int postSplitSize;
        char** postHttpLine = Split(buffer, "\r\n\r\n", &postSplitSize);
        int requestData = strlen(postHttpLine[1]);
        obj->data = cJSON_Parse(postHttpLine[1]);
        if (obj->data == NULL) {
            const char* error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                fprintf(stderr, "JSON ÆÄ½Ì ¿À·ù: %s\n", error_ptr);
            }
            return 1;
        }
        FreeSplit(postHttpLine, postSplitSize);
    }

    memcpy(obj->host, httpLine1[1], strlen(httpLine1[1]));
    memcpy(obj->url, httpLine0[1], strlen(httpLine0[1]));
    printf("Request method: %s url: %s\n", httpLine0[0], httpLine0[1]);
    FreeSplit(httpLine1, httpLine1Size);
    FreeSplit(httpLine0, httpLine0Size);
    FreeSplit(httpLine, splitSize);
    return TRUE;
}

void HandleRequest(Connector* con, HTMLParseObject* obj)
{
    switch (obj->method)
    {
    case GET:
    {
        char responseHeader[BUFFER_SIZE];
        const char* contentType = "text/html";
        if (strstr(obj->url, ".js")) {
            contentType = "application/javascript";
        }
        else if (strstr(obj->url, ".css")) {
            contentType = "text/css";
        }

        if (strcmp(obj->url, "/") == 0)
        {
            char* htmlContent;
            int fileSize = Readfile("/template/index.html", &htmlContent);
            if (fileSize > 0) {
                snprintf(responseHeader, sizeof(responseHeader),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n", contentType, fileSize);

                send(con->socket, responseHeader, (int)strlen(responseHeader), 0);
                send(con->socket, htmlContent, fileSize, 0);
                free(htmlContent);
            }
        }
        else if (StartsWith(obj->url, "/static/js")) {
            char* jsContent;
            int fileSize = Readfile(obj->url, &jsContent);
            if (fileSize > 0) {
                snprintf(responseHeader, sizeof(responseHeader),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n", contentType, fileSize);

                send(con->socket, responseHeader, (int)strlen(responseHeader), 0);
                send(con->socket, jsContent, fileSize, 0);
                free(jsContent);
            }
        }
        else if (StartsWith(obj->url, "/static/css")) {
            char* cssContent;
            int fileSize = Readfile(obj->url, &cssContent);
            if (fileSize > 0) {
                snprintf(responseHeader, sizeof(responseHeader),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n", contentType, fileSize);

                send(con->socket, responseHeader, (int)strlen(responseHeader), 0);
                send(con->socket, cssContent, fileSize, 0);
                free(cssContent);
            }
        }
        else
        {
            const char* notFoundHeader =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n";
            send(con->socket, notFoundHeader, (int)strlen(notFoundHeader), 0);

            const char* notFoundResponse =
                "<html>"
                "<head><title>404 Not Found</title></head>"
                "<body><h1>404 Not Found</h1></body>"
                "</html>";
            send(con->socket, notFoundResponse, (int)strlen(notFoundResponse), 0);
        }
    }
        break;

    case POST:
    {
        if (strcmp(obj->url, "/saveStudent")==0) {
            sqlite3* db;
            char* zErrMsg = 0;
            int rc;
            char sql[256] = "";

            cJSON* studentName = cJSON_GetObjectItemCaseSensitive(obj->data, "studentName");
            cJSON* studentId = cJSON_GetObjectItemCaseSensitive(obj->data, "studentId");
            cJSON* studentClass = cJSON_GetObjectItemCaseSensitive(obj->data, "studentClass");
     
            rc = sqlite3_open("scoreflow.db", &db);
            sprintf_s(sql, sizeof(sql), "INSERT INTO student (STUDENT_NAME, STUDENT_ID, STUDENT_CLASS) VALUES ('%s', '%s', '%s');", studentName->valuestring, studentId->valuestring, studentClass->valuestring);
            rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            else {
                fprintf(stdout, "Tables listed successfully\n");
            }
            sqlite3_close(db);

            char* jsonString = cJSON_PrintUnformatted(obj->data);
            char responseHeader[BUFFER_SIZE];
            snprintf(responseHeader, sizeof(responseHeader),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n"
                "\r\n", (int)strlen(jsonString));
            send(con->socket, responseHeader, (int)strlen(responseHeader), 0);
            send(con->socket, jsonString, (int)strlen(jsonString), 0);
            free(jsonString);
            cJSON_Delete(obj->data);
        }
        else if (strcmp(obj->url, "/findListStudent") == 0) {
            sqlite3* db;
            char* zErrMsg = 0;
            int rc;
            char sql[256] = "select STUDENT_SQ as student_sq, STUDENT_NAME as student_name, STUDENT_ID as student_id, STUDENT_CLASS as student_class from student;";
            rc = sqlite3_open("scoreflow.db", &db);
            cJSON* jsonArray = cJSON_CreateArray();
            rc = sqlite3_exec(db, sql, selectList, (void*)jsonArray, &zErrMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            else {
                fprintf(stdout, "Tables listed successfully\n");
            }
            sqlite3_close(db);
            char* jsonString = cJSON_PrintUnformatted(jsonArray);
            char responseHeader[BUFFER_SIZE];
            snprintf(responseHeader, sizeof(responseHeader),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n"
                "\r\n", (int)strlen(jsonString));

            send(con->socket, responseHeader, (int)strlen(responseHeader), 0);
            send(con->socket, jsonString, (int)strlen(jsonString), 0);
            free(jsonString);
            cJSON_Delete(jsonArray);
            cJSON_Delete(obj->data);
        }
        else if (strcmp(obj->url, "/saveScore") == 0) {
            sqlite3* db;
            char* zErrMsg = 0;
            int rc;
            char sql[256] = "";

            cJSON* studentId = cJSON_GetObjectItemCaseSensitive(obj->data, "studentId");
            cJSON* subject = cJSON_GetObjectItemCaseSensitive(obj->data, "subject");
            cJSON* score = cJSON_GetObjectItemCaseSensitive(obj->data, "score");

            rc = sqlite3_open("scoreflow.db", &db);
            sprintf_s(sql, sizeof(sql), "INSERT INTO mark (SUBJECT_NAME, MARK, STUDENT_SQ) VALUES ('%s', '%s', '%s');", subject->valuestring, score->valuestring, studentId->valuestring);
            rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            else {
                fprintf(stdout, "Tables listed successfully\n");
            }
            sqlite3_close(db);

            char* jsonString = cJSON_PrintUnformatted(obj->data);
            char responseHeader[BUFFER_SIZE];
            snprintf(responseHeader, sizeof(responseHeader),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n"
                "\r\n", (int)strlen(jsonString));
            send(con->socket, responseHeader, (int)strlen(responseHeader), 0);
            send(con->socket, jsonString, (int)strlen(jsonString), 0);
            free(jsonString);
            cJSON_Delete(obj->data);
        }

        else if (strcmp(obj->url, "/findListMainData") == 0) {
        sqlite3* db;
        char* zErrMsg = 0;
        int rc;
        char sql[256] = "SELECT st.STUDENT_NAME, mk.SUBJECT_NAME, mk.MARK, st.STUDENT_CLASS  FROM MARK AS mk LEFT OUTER JOIN STUDENT as st ON  mk.STUDENT_SQ = st.STUDENT_SQ;";
        rc = sqlite3_open("scoreflow.db", &db);
        cJSON* jsonArray = cJSON_CreateArray();
        rc = sqlite3_exec(db, sql, selectList, (void*)jsonArray, &zErrMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "Tables listed successfully\n");
        }
        sqlite3_close(db);
        char* jsonString = cJSON_PrintUnformatted(jsonArray);
        char responseHeader[BUFFER_SIZE];
        snprintf(responseHeader, sizeof(responseHeader),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n", (int)strlen(jsonString));
        send(con->socket, responseHeader, (int)strlen(responseHeader), 0);
        send(con->socket, jsonString, (int)strlen(jsonString), 0);
        free(jsonString);
        cJSON_Delete(jsonArray);
        cJSON_Delete(obj->data);
        }
    }
        break;
    }
}
