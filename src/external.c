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

void hados_external_init(struct hados_external *external,
		struct hados_context *context) {
	external->context = context;
	external->body = malloc(1); /* will be grown as needed by the realloc above */
	external->size = 0; /* no data at this point */
	external->hados_status = 0;
	external->json = NULL;
}

void hados_external_free(struct hados_external *external) {
	if (external->body != NULL ) {
		free(external->body);
		external->body = NULL;
	}
	if (external->json != NULL ) {
		json_value_free(external->json);
		external->json = NULL;
	}
}

static size_t writeDataCallback(void *content, size_t size, size_t nmemb,
		void *userdata) {
	size_t realsize = size * nmemb;
	struct hados_external *external = (struct hados_external *) userdata;

	external->body = realloc(external->body, external->size + realsize + 1);
	if (external->body == NULL ) {
		hados_response_set_status(&external->context->response,
				HADOS_INTERNAL_ERROR, "Memory issue");
		return 0;
	}

	memcpy(&(external->body[external->size]), content, realsize);
	external->size += realsize;
	external->body[external->size] = 0;
	return realsize;
}

static size_t writeHeaderCallback(void *content, size_t size, size_t nmemb,
		void *userdata) {
	size_t realsize = size * nmemb;
	struct hados_external *external = (struct hados_external *) userdata;
	if (content == NULL )
		return realsize;
	char *saveptr;
	char *header = strdup(content);
	char* token = strtok_r(header, ":", &saveptr);
	if (token != NULL ) {
		if (strcmp(token, HADOS_HEADER_STATUS) == 0) {
			token = strtok_r(NULL, ": ", &saveptr);
			if (token != NULL )
				external->hados_status = atoi(token);
		}
	}
	return realsize;
}

/**
 * Perform a CURL call using its own CURL handle
 */
static void hados_external_curl_get(struct hados_external *external,
		const char* url) {
	CURLcode res;
	CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * ) external);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaderCallback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void * ) external);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
		hados_context_error_printf(external->context, "CURL error: %s",
				curl_easy_strerror(res));
	curl_easy_cleanup(curl);
}

char* hados_external_url(struct hados_external *external, const char* node_url,
		const char *cmd, const char *path) {
	CURL *curl = curl_easy_init();
	char* esc_path = curl_easy_escape(curl, path, 0);
	int l = strlen(node_url) + strlen(cmd) + 6;
	if (path != NULL )
		l += strlen(esc_path) + 7;
	char *url = malloc(l * sizeof(char));
	strcpy(url, node_url);
	strcat(url, "?cmd=");
	strcat(url, cmd);
	if (path != NULL ) {
		strcat(url, "&path=");
		strcat(url, esc_path);
	}
	curl_free(esc_path);
	curl_easy_cleanup(curl);
	return url;
}

static int hados_external_command(struct hados_external *external,
		const char* node_url, const char *command, const char* path) {
	char *url = hados_external_url(external, node_url, command, path);
	hados_external_curl_get(external, url);
	free(url);
	return external->hados_status;
}

int hados_external_exists(struct hados_external *external, const char* node_url,
		const char* path) {
	return hados_external_command(external, node_url, "exists", path);
}

int hados_external_list(struct hados_external *external, const char* node_url,
		const char* path) {
	return hados_external_command(external, node_url, "list", path);
}

int hados_external_delete(struct hados_external *external, const char* node_url,
		const char* path) {
	return hados_external_command(external, node_url, "delete", path);
}

json_value* hados_external_get_json(struct hados_external *external) {
	if (external->json != NULL )
		return external->json;
	if (external->body == NULL )
		return NULL ;
	char err[256];
	json_settings json_settings = { 0 };
	external->json = json_parse_ex(&json_settings, external->body,
			strlen(external->body), err);
	if (external->json == NULL )
		hados_context_error_printf(external->context, "JSON error: %s", err);
	return external->json;
}
