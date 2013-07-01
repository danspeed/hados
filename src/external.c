/*
 HADOS
 High Availability Distributed Search Engine
 Copyright (C) 2013 Jaeksoft / Emmanuel Keller

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ============================================================================
 Name        : external.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"
#include <curl/curl.h>

struct CallbackStruct {
	char *body;
	size_t size;
	struct hados_response *response;
	int hados_status;
};

static void hados_callback_init(struct CallbackStruct *callback,
		struct hados_response *response) {
	callback->response = response;
	callback->body = malloc(1); /* will be grown as needed by the realloc above */
	callback->size = 0; /* no data at this point */
	callback->hados_status = 0;
}

static void hados_callback_free(struct CallbackStruct *callback) {
	if (callback->body != NULL ) {
		free(callback->body);
		callback->body = NULL;
	}
}

static size_t writeDataCallback(void *content, size_t size, size_t nmemb,
		void *userdata) {
	size_t realsize = size * nmemb;
	struct CallbackStruct *callback = (struct CallbackStruct *) userdata;

	callback->body = realloc(callback->body, callback->size + realsize + 1);
	if (callback->body == NULL ) {
		hados_response_set_status(callback->response, HADOS_INTERNAL_ERROR,
				"Memory issue");
		return 0;
	}

	memcpy(&(callback->body[callback->size]), content, realsize);
	callback->size += realsize;
	callback->body[callback->size] = 0;
	return realsize;
}

static size_t writeHeaderCallback(void *content, size_t size, size_t nmemb,
		void *userdata) {
	size_t realsize = size * nmemb;
	struct CallbackStruct *callback = (struct CallbackStruct *) userdata;
	if (content == NULL )
		return realsize;
	char *saveptr;
	char *header = strdup(content);
	char* token = strtok_r(header, ":", &saveptr);
	if (token != NULL ) {
		if (strcmp(token, "X-Hados-Status") == 0) {
			token = strtok_r(NULL, ": ", &saveptr);
			if (token != NULL )
				callback->hados_status = atoi(token);
		}
	}
	return realsize;
}

static void curlGet(const char* url, struct CallbackStruct *callback) {
	CURL *curl_handle;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeDataCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void * ) callback);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, writeHeaderCallback);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void * ) callback);

	res = curl_easy_perform(curl_handle);

	if (res != CURLE_OK)
		perror(curl_easy_strerror(res));

	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
}

char* hados_external_url(const char* node_url, const char *cmd,
		const char *path) {
	int l = strlen(node_url) + strlen(cmd) + 5;
	if (path != NULL )
		l += strlen(path) + 6;
	char *url = malloc(l * sizeof(char) + 1);
	strcpy(url, node_url);
	strcat(url, "?cmd=");
	strcat(url, cmd);
	if (path != NULL ) {
		strcat(url, "&path=");
		strcat(url, path);
	}
	return url;
}

static int hados_external_command(struct hados_response *response,
		const char* node_url, const char *command, const char* path) {
	char *url = hados_external_url(node_url, command, path);
	struct CallbackStruct callback;
	hados_callback_init(&callback, response);
	callback.response = response;
	curlGet(url, &callback);
	hados_callback_free(&callback);
	free(url);
	return callback.hados_status;
}

int hados_external_exists(struct hados_response *response, const char* node_url,
		const char* path) {
	return hados_external_command(response, node_url, "exists", path);
}

int hados_external_delete(struct hados_response *response, const char* node_url,
		const char* path) {
	return hados_external_command(response, node_url, "delete", path);
}
