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

static int checkDataFileDir(struct hados_context *context,
		struct hados_response *response) {
	if (context->data_dir == NULL || strlen(context->data_dir) == 0)
		return hados_response_set_status(response, HADOS_DATADIR_NOT_SET,
				"The data directory has not been set");
	if (context->file_dir == NULL || strlen(context->file_dir) == 0)
		return hados_response_set_status(response, HADOS_FILEDIR_NOT_SET,
				"The file directory has not been set");
	return hados_response_set_success(response);
}

static int hados_command_put(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataFileDir(context, response) != HADOS_SUCCESS)
		return response->status;
	if (hados_context_set_object(context, request, response) != HADOS_SUCCESS)
		return response->status;
	struct hados_tempfile tempfile;
	if (hados_tempfile_new(&tempfile, context, response) != HADOS_SUCCESS)
		return response->status;
	if (hados_tempfile_upload(&tempfile, context, response) != HADOS_SUCCESS) {
		hados_tempfile_free(&tempfile);
		return response->status;
	}
	if (hados_utils_mkdirs(context->object.filepath, response) != HADOS_SUCCESS) {
		hados_tempfile_free(&tempfile);
		return response->status;
	}
	if (rename(tempfile.path, context->object.filepath) == -1) {
		hados_tempfile_free(&tempfile);
		hados_response_set_errno(response);
	}
	return hados_response_set_success(response);
}

static int hados_command_get(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataFileDir(context, response) != HADOS_SUCCESS)
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

static int hados_command_exists(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataFileDir(context, response) != HADOS_SUCCESS)
		return response->status;
	if (hados_context_set_object(context, request, response) != HADOS_SUCCESS)
		return response->status;
	struct statvfs stvfs;
	if (statvfs(context->data_dir, &stvfs) != 0)
		return hados_response_set_errno(response);
	char s[256];
	unsigned long free_rate = (stvfs.f_blocks % stvfs.f_bavail != 0);
	unsigned long free_space = (stvfs.f_bavail * stvfs.f_frsize) / 1024;
	sprintf(s, ",\n\"space_free\": %lu,\n\"free_rate\": %lu", free_space,
			free_rate);
	hados_response_more_json(response, s);
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

static int hados_command_delete(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	if (checkDataFileDir(context, response) != HADOS_SUCCESS)
		return response->status;
	if (hados_context_set_object(context, request, response) != HADOS_SUCCESS)
		return response->status;
	if (unlink(context->object.filepath) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(response, HADOS_OBJECT_NOT_FOUND,
					"Object/file not found");
		return hados_response_set_errno(response);
	}
	return hados_response_set_status(response, HADOS_SUCCESS,
			"Object/file deleted");
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

static int hados_command_cluster_put(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	char s[2048];
	int i;
	int found = 0;
	for (i = 0; i < context->nodesNumber; i++) {
		char *currentNode = context->nodeArray[i];
		if (hados_external_exists(response, currentNode, request->paramPath)
				== 200) {
			// PUT
			if (found == 0)
				sprintf(s, ", \"put_on\":[ \"%s\"", currentNode);
			else
				sprintf(s, ", \"%s\"", currentNode);
		}
	}
	if (found > 0)
		hados_response_more_json(response, "]");
	sprintf(s, "Put %d over %d", found, i);
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

static int hados_command_cluster_delete(struct hados_context *context,
		struct hados_request *request, struct hados_response *response) {
	char s[2048];
	int i;
	int found = 0;
	for (i = 0; i < context->nodesNumber; i++) {
		char *currentNode = context->nodeArray[i];
		if (hados_external_delete(response, currentNode, request->paramPath)
				== 0) {
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
	sprintf(s, "delete %d over %d node(s)", found, i);
	return hados_response_set_status(response, HADOS_SUCCESS, s);
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
	} else if (strcmp(request->command, "cluster_put") == 0) {
		hados_command_cluster_put(context, request, response);
	} else if (strcmp(request->command, "cluster_exists") == 0) {
		hados_command_cluster_exists(context, request, response);
	} else if (strcmp(request->command, "cluster_get") == 0) {
		hados_command_cluster_get(context, request, response);
	} else if (strcmp(request->command, "cluster_delete") == 0) {
		hados_command_cluster_delete(context, request, response);
	} else {
		hados_response_set_status(response, HADOS_UNKNOWN_COMMAND,
				"Unknown command");
	}
}
