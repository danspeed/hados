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
 Name        : commands.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

static int checkDataDir(const char *data_dir, struct hados_response *response) {
	if (data_dir == NULL || strlen(data_dir) == 0)
		return hados_response_set_status(response, HADOS_DATADIR_NOT_SET,
				"The data directory has not been set");
	struct stat s;
	int err = stat(data_dir, &s);
	if (err == -1) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_DATADIR_NOT_SET,
					"The data directory does not exist");
		return hados_response_set_errno(response);
	}
	if (!S_ISDIR(s.st_mode))
		return hados_response_set_status(response,
				HADOS_DATADIR_IS_NOT_A_DIRECTORY,
				"The data directory path is not a directory");
	return hados_response_set_success(response);
}

static int hados_command_put(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataDir(context->data_dir, response) != HADOS_SUCCESS)
		return response->status;
	if (hados_context_set_object(context, request, response) != HADOS_SUCCESS)
		return response->status;
	const char* content_length = hados_context_get_env(context,
			"CONTENT_LENGTH");
	long contentLength = 0;
	if (content_length != NULL )
		contentLength = atol(content_length);
	if (contentLength == 0)
		return hados_response_set_status(response,
				HADOS_NO_CONTENT_LENGTH_GIVEN, "Missing content length header");
	if (hados_utils_mkdirs(context->object.filepath, response) != HADOS_SUCCESS)
		return response->status;
	FILE *file = fopen(context->object.filepath, "wb");
	if (file == NULL )
		return hados_response_set_errno(response);
	int c;
	context->bytes_received = 0;
	for (;;) {
		c = FCGX_GetChar(context->fcgxRequest.in);
		if (c == EOF) {
			break;
		}
		context->bytes_received++;
		if (fputc(c, file) == EOF) {
			hados_response_set_errno(response);
			break;
		}
	}
	fclose(file);
	if (context->bytes_received != contentLength)
		return hados_response_set_status(response,
				HADOS_NOT_ENOUGH_BYTES_RECEIVED, "Not enough bytes received");;
	return hados_response_set_success(response);
}

static int hados_command_get(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataDir(context->data_dir, response) != HADOS_SUCCESS)
		return response->status;
	if (hados_context_set_object(context, request, response) != HADOS_SUCCESS)
		return response->status;
	struct stat st;
	if (stat(context->object.filepath, &st) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_OBJECT_NOT_FOUND,
					"Object/file not found");
		return hados_response_set_errno(response);
	}
	FILE *file = fopen(context->object.filepath, "rb");
	if (file == NULL )
		return hados_response_set_errno(response);
	response->status = HADOS_BINARY_RESULT;
	hados_context_printf(context, "Content-type: application/octet-stream\r\n");
	hados_context_printf(context, "Content-Transfer-Encoding: binary\r\n\r\n");
	int c;
	while ((c = fgetc(file)) != EOF)
		FCGX_PutChar(c, context->fcgxRequest.out);
	fclose(file);
	return response->status;
}

static int hados_command_delete(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataDir(context->data_dir, response) != HADOS_SUCCESS)
		return response->status;
	if (hados_context_set_object(context, request, response) != HADOS_SUCCESS)
		return response->status;
	struct stat st;
	if (stat(context->object.filepath, &st) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_OBJECT_NOT_FOUND,
					"Object/file not found");
		return hados_response_set_errno(response);
	}
	if (unlink(context->object.filepath) != 0)
		return hados_response_set_errno(response);
	return hados_response_set_success(response);
}

static int hados_command_exists(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataDir(context->data_dir, response) != HADOS_SUCCESS)
		return response->status;
	if (hados_context_set_object(context, request, response) != HADOS_SUCCESS)
		return response->status;
	struct stat st;
	if (stat(context->object.filepath, &st) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_OBJECT_NOT_FOUND,
					"Object/file not found");
		return hados_response_set_errno(response);
	}
	return hados_response_set_status(response, HADOS_OBJECT_FOUND,
			"Object/file exists");
}

static int hados_command_cluster_exists(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	char s[2048];
	int i;
	int found = 0;
	for (i = 0; i < context->nodesNumber; i++) {
		char *currentNode = context->nodeArray[i];
		if (hados_external_exists(response, currentNode, request->paramPath)
				== 200) {
			if (found == 0)
				sprintf(s, ", \"found_on\":[ \"%s\"", currentNode);
			else
				sprintf(s, ", \"%s\"", currentNode);
			hados_response_more_json(response, s);
			found++;
		}
	}
	if (found > 0)
		hados_response_more_json(response, "]");
	sprintf(s, "Found %d over %d", found, i);
	return hados_response_set_status(response, HADOS_SUCCESS, s);
}

static int hados_command_cluster_get(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (hados_command_get(context, request, response) == HADOS_SUCCESS)
		return HADOS_SUCCESS;
	int i;
	for (i = 0; i < context->nodesNumber; i++) {
		if (hados_external_exists(response, context->nodeArray[i],
				request->paramPath) == 200) {
			char *url = hados_external_url(context->nodeArray[i], "get",
					request->paramPath);
			hados_response_set_status(response, HADOS_REDIRECT, url);
			free(url);
			return response->status;
		}
	}
	return hados_response_set_status(response, HADOS_OBJECT_NOT_FOUND,
			"Object/file not found on the cluster");
}

void hados_command_dispatch(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {

	if (request->command == NULL )
		return;

	if (strcmp(request->command, "put") == 0) {
		hados_command_put(context, request, response);
	} else if (strcmp(request->command, "get") == 0) {
		hados_command_get(context, request, response);
	} else if (strcmp(request->command, "delete") == 0) {
		hados_command_delete(context, request, response);
	} else if (strcmp(request->command, "exists") == 0) {
		hados_command_exists(context, request, response);
	} else if (strcmp(request->command, "cluster_exists") == 0) {
		hados_command_cluster_exists(context, request, response);
	} else if (strcmp(request->command, "cluster_get") == 0) {
		hados_command_cluster_get(context, request, response);
	} else {
		hados_response_set_status(response, HADOS_UNKNOWN_COMMAND,
				"Unknown command");
	}
}
