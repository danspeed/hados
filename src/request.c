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
 Name        : request.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

void hados_request_init(struct hados_request *request) {
	request->count = 0;
	request->key = NULL;
	request->value = NULL;
	request->keyvalue = NULL;
	request->queryString = NULL;
	request->command = NULL;
}

void hados_request_free(struct hados_request *request) {
	if (request->key != NULL ) {
		free(request->key);
		request->key = NULL;
	}
	if (request->value != NULL ) {
		free(request->value);
		request->value = NULL;
	}
	if (request->keyvalue != NULL ) {
		free(request->keyvalue);
		request->keyvalue = NULL;
	}
	if (request->queryString != NULL ) {
		free(request->queryString);
		request->queryString = NULL;
	}
	request->count = 0;
}

/**
 * Build the parameters structure using the query string
 */
void hados_request_load(struct hados_request *request,
		struct hados_context *context) {

	gettimeofday(&request->requestTime, NULL );

	const char* envQueryString = hados_context_get_env(context, "QUERY_STRING");

	CURL *curl_handle = NULL;
	curl_handle = curl_easy_init();
	char *queryString = curl_easy_unescape(curl_handle, envQueryString, 0,
			NULL );

	// Count the number of parameters
	if (queryString != NULL && strlen(queryString) > 0) {
		char *qs = strdup(queryString);
		char *token = strtok(qs, "&");
		while (token != NULL ) {
			request->count++;
			token = strtok(NULL, "&");
		}
		free(qs);
		if (request->count == 0)
			goto exit;
		request->keyvalue = (char **) malloc(request->count * sizeof(void *));
		request->key = (char **) malloc(request->count * sizeof(void *));
		request->value = (char **) malloc(request->count * sizeof(void *));

		// Populate the keyvalue array
		request->queryString = strdup(queryString);
		token = strtok(request->queryString, "&");
		int pos = 0;
		while (token != NULL ) {
			request->keyvalue[pos] = token;
			token = strtok(NULL, "&");
			pos++;
		}

		// Populate the key/value array
		int i = 0;
		for (i = 0; i < request->count; i++) {
			request->key[i] = strtok(request->keyvalue[i], "=");
			request->value[i] = strtok(NULL, "=");
		}

		request->command = hados_request_getvalue(request, "cmd");
		request->paramPath = hados_request_getvalue(request, "path");
	}

	exit:

	if (queryString != NULL )
		curl_free(queryString);
	if (curl_handle != NULL )
		curl_easy_cleanup(curl_handle);
}

/**
 * Retrieve the value of the parameter with the passed key
 */
char* hados_request_getvalue(struct hados_request *request, const char* key) {
	int i;
	for (i = 0; i < request->count; i++)
		if (request->key[i] != NULL )
			if (strcmp(key, request->key[i]) == 0)
				return request->value[i];
	return NULL ;
}
