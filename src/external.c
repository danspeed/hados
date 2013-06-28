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

#include "fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "hados.h"

static size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb,
		void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *) userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL ) {
		perror("not enough memory (realloc returned NULL)");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}

static int hados_do_get(const char* url, struct MemoryStruct *chunk) {
	CURL *curl_handle;
	CURLcode res;

	chunk->memory = malloc(1); /* will be grown as needed by the realloc above */
	chunk->size = 0; /* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void * ) chunk);

	res = curl_easy_perform(curl_handle);

	int status;
	if (res != CURLE_OK) {
		perror(curl_easy_strerror(res));
		status = HADOS_INTERNAL_ERROR;
	} else
		status = HADOS_SUCCESS;

	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	return status;
}

static int hados_external_exists(const char* url, const char* path) {
	char urlQuery[2048];
	strcpy(urlQuery, url);
	strcat(urlQuery, "?cmd=exists&path=");
	strcat(urlQuery, path);
	struct MemoryStruct chunck;
	int status = hados_do_get(urlQuery, &chunck);
	perror(chunck.memory);
	free(chunck.memory);
	return status;
}

int hados_external_put_if_exists(struct hados_context *context) {
	int i;
	for (i = 0; i < context->nodesNumber; i++) {
		if (strcmp(context->node, context->nodeArray[i]) == 0)
			continue;
		hados_external_exists(context->nodeArray[i], context->paramPath);
	}
	return HADOS_SUCCESS;
}
