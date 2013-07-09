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

static int checkDataFileDir(struct hados_context *context) {
	if (context->data_dir == NULL || strlen(context->data_dir) == 0)
		return hados_response_set_status(&context->response,
				HADOS_DATADIR_NOT_SET, "The data directory has not been set");
	if (context->file_dir == NULL || strlen(context->file_dir) == 0)
		return hados_response_set_status(&context->response,
				HADOS_FILEDIR_NOT_SET, "The file directory has not been set");
	return hados_response_set_success(&context->response);
}

static int hados_command_put(struct hados_context *context) {
	if (checkDataFileDir(context) != HADOS_SUCCESS)
		return context->response.status;
	if (hados_context_set_object(context) != HADOS_SUCCESS)
		return context->response.status;
	struct hados_tempfile tempfile;
	if (hados_tempfile_new(&tempfile, context) != HADOS_SUCCESS)
		return context->response.status;
	if (hados_tempfile_upload(&tempfile, context) != HADOS_SUCCESS) {
		hados_tempfile_free(&tempfile);
		return context->response.status;
	}
	if (hados_utils_mkdirs(context->object.filepath,
			&context->response) != HADOS_SUCCESS) {
		hados_tempfile_free(&tempfile);
		return context->response.status;
	}
	if (rename(tempfile.path, context->object.filepath) == -1) {
		hados_tempfile_free(&tempfile);
		hados_response_set_errno(&context->response);
	}
	return hados_response_set_success(&context->response);
}

static int hados_command_get(struct hados_context *context) {
	if (checkDataFileDir(context) != HADOS_SUCCESS)
		return context->response.status;
	if (hados_context_set_object(context) != HADOS_SUCCESS)
		return context->response.status;
	struct stat st;
	if (stat(context->object.filepath, &st) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(&context->response,
					HADOS_OBJECT_NOT_FOUND, "Object/file not found");
		return hados_response_set_errno(&context->response);
	}
	FILE *file = fopen(context->object.filepath, "rb");
	if (file == NULL )
		return hados_response_set_errno(&context->response);
	context->response.status = HADOS_BINARY_RESULT;
	hados_context_printf(context, "Content-type: application/octet-stream\r\n");
	hados_context_printf(context, "Content-Transfer-Encoding: binary\r\n\r\n");
	int c;
	while ((c = fgetc(file)) != EOF)
		FCGX_PutChar(c, context->fcgxRequest.out);
	fclose(file);
	return context->response.status;
}

static int hados_command_exists(struct hados_context *context) {
	if (checkDataFileDir(context) != HADOS_SUCCESS)
		return context->response.status;
	if (hados_context_set_object(context) != HADOS_SUCCESS)
		return context->response.status;
	struct statvfs stvfs;
	if (statvfs(context->data_dir, &stvfs) != 0)
		return hados_response_set_errno(&context->response);
	char s[256];
	unsigned long free_rate = (stvfs.f_blocks % stvfs.f_bavail != 0);
	unsigned long free_space = (stvfs.f_bavail * stvfs.f_frsize) / 1024;
	sprintf(s, ",\n\"space_free\": %lu,\n\"free_rate\": %lu", free_space,
			free_rate);
	hados_response_more_json(&context->response, s);
	struct stat st;
	if (stat(context->object.filepath, &st) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(&context->response,
					HADOS_OBJECT_NOT_FOUND, "Object/file not found");
		return hados_response_set_errno(&context->response);
	}
	return hados_response_set_status(&context->response, HADOS_OBJECT_FOUND,
			"Object/file exists");
}

static int hados_command_delete(struct hados_context *context) {
	if (checkDataFileDir(context) != HADOS_SUCCESS)
		return context->response.status;
	if (hados_context_set_object(context) != HADOS_SUCCESS)
		return context->response.status;
	if (unlink(context->object.filepath) != 0) {
		if (errno == ENOENT)
			return hados_response_set_status(&context->response,
					HADOS_OBJECT_NOT_FOUND, "Object/file not found");
		return hados_response_set_errno(&context->response);
	}
	return hados_response_set_status(&context->response, HADOS_SUCCESS,
			"Object/file deleted");
}

static int hados_command_list(struct hados_context *context) {
	if (checkDataFileDir(context) != HADOS_SUCCESS)
		return context->response.status;
	if (hados_context_set_object(context) != HADOS_SUCCESS)
		return context->response.status;
	int found = 0;
	char s1[2048];
	char s2[2200];
	struct hados_fileitem fileitem;
	DIR *dp;
	struct dirent *ep;
	dp = opendir(context->object.filepath);
	if (dp == NULL )
		return hados_response_set_errno(&context->response);
	while ((ep = readdir(dp)) != NULL ) {
		if (strcmp(".", ep->d_name) == 0)
			continue;
		if (strcmp("..", ep->d_name) == 0)
			continue;
		hados_fileitem_static(&fileitem, ep);
		hados_fileitem_to_json(&fileitem, s1, sizeof(s1));
		if (found++ == 0)
			sprintf(s2, ", \"list\":[ %s", s1);
		else
			sprintf(s2, ", %s", s1);
		hados_response_more_json(&context->response, s2);
	}
	closedir(dp);
	if (found > 0)
		hados_response_more_json(&context->response, "]");
	return hados_response_set_success(&context->response);
}

static int hados_command_cluster_exists(struct hados_context *context) {
	char s[2048];
	int i;
	int found = 0;
	struct hados_external external;
	for (i = 0; i < context->nodesNumber; i++) {
		char *currentNode = context->nodeArray[i];
		hados_external_init(&external, context);
		if (hados_external_exists(&external, currentNode,
				context->request.paramPath) == 200) {
			if (found == 0)
				sprintf(s, ", \"found_on\":[ \"%s\"", currentNode);
			else
				sprintf(s, ", \"%s\"", currentNode);
			hados_response_more_json(&context->response, s);
			found++;
		}
		hados_external_free(&external);
	}
	if (found > 0)
		hados_response_more_json(&context->response, "]");
	sprintf(s, "Found %d over %d", found, i);
	return hados_response_set_status(&context->response, HADOS_SUCCESS, s);
}

static int hados_command_cluster_list(struct hados_context *context) {
	int i;
	struct hados_external external;
	struct hados_fileitem_array fileitems;
	hados_fileitem_array_init(&fileitems);
	for (i = 0; i < context->nodesNumber; i++) {
		char *currentNode = context->nodeArray[i];
		hados_external_init(&external, context);
		if (hados_external_list(&external, currentNode,
				context->request.paramPath) == HADOS_SUCCESS) {
			json_value *json = hados_external_get_json(&external);
			if (json != NULL ) {
				json_value *json_list = hados_utils_json_get_array(json,
						"list");
				if (json_list != NULL )
					hados_fileitem_array_load(&fileitems, json_list);
			}
		}
		hados_external_free(&external);
	}
	hados_fileitem_array_sort(&fileitems);
	char s1[2048];
	char s2[2200];
	struct hados_fileitem *last_fileitem = NULL;
	for (i = 0; i < fileitems.length; i++) {
		struct hados_fileitem *fileitem = &fileitems.fileitems[i];
		// Node can share same files. After sorting, if they are consecutively identical, we can ignore
		if (last_fileitem != NULL )
			if (hados_fileitem_cmp(fileitem, last_fileitem) == 0)
				continue;
		hados_fileitem_to_json(fileitem, s1, sizeof(s1));
		if (i == 0)
			sprintf(s2, ", \"list\":[ %s", s1);
		else
			sprintf(s2, ", %s", s1);
		hados_response_more_json(&context->response, s2);
		last_fileitem = fileitem;
	}
	if (fileitems.length > 0)
		hados_response_more_json(&context->response, "]");
	hados_fileitem_array_free(&fileitems);
	return hados_response_set_success(&context->response);
}

static int hados_command_cluster_put(struct hados_context *context) {
	char s[2048];
	int i;
	int found = 0;
	struct hados_external external;
	for (i = 0; i < context->nodesNumber; i++) {
		char *currentNode = context->nodeArray[i];
		hados_external_init(&external, context);
		if (hados_external_exists(&external, currentNode,
				context->request.paramPath) == 200) {
			// PUT
			if (found == 0)
				sprintf(s, ", \"put_on\":[ \"%s\"", currentNode);
			else
				sprintf(s, ", \"%s\"", currentNode);
		}
		hados_external_free(&external);
	}
	if (found > 0)
		hados_response_more_json(&context->response, "]");
	sprintf(s, "Put %d over %d", found, i);
	return hados_response_set_status(&context->response, HADOS_SUCCESS, s);
}

static int hados_command_cluster_get(struct hados_context *context) {
	if (hados_command_get(context) == HADOS_SUCCESS)
		return HADOS_SUCCESS;
	int i;
	struct hados_external external;
	// TODO: Change for a random choice
	for (i = 0; i < context->nodesNumber; i++) {
		hados_external_init(&external, context);
		if (hados_external_exists(&external, context->nodeArray[i],
				context->request.paramPath) == 200) {
			char *url = hados_external_url(&external, context->nodeArray[i],
					"get", context->request.paramPath);
			hados_response_set_status(&context->response, HADOS_REDIRECT, url);
			free(url);
			hados_external_free(&external);
			return context->response.status;
		}
		hados_external_free(&external);
	}
	return hados_response_set_status(&context->response, HADOS_OBJECT_NOT_FOUND,
			"Object/file not found on the cluster");
}

static int hados_command_cluster_delete(struct hados_context *context) {
	char s[2048];
	int i;
	int found = 0;
	struct hados_external external;
	for (i = 0; i < context->nodesNumber; i++) {
		char *currentNode = context->nodeArray[i];
		hados_external_init(&external, context);
		if (hados_external_delete(&external, currentNode,
				context->request.paramPath) == 0) {
			if (found == 0)
				sprintf(s, ", \"found_on\":[ \"%s\"", currentNode);
			else
				sprintf(s, ", \"%s\"", currentNode);
			hados_response_more_json(&context->response, s);
			found++;
		}
		hados_external_free(&external);
	}
	if (found > 0)
		hados_response_more_json(&context->response, "]");
	sprintf(s, "delete %d over %d node(s)", found, i);
	return hados_response_set_status(&context->response, HADOS_SUCCESS, s);
}

void hados_command_dispatch(struct hados_context *context) {

	if (context->request.command == NULL )
		return;

	if (strcmp(context->request.command, "put") == 0) {
		hados_command_put(context);
	} else if (strcmp(context->request.command, "get") == 0) {
		hados_command_get(context);
	} else if (strcmp(context->request.command, "delete") == 0) {
		hados_command_delete(context);
	} else if (strcmp(context->request.command, "exists") == 0) {
		hados_command_exists(context);
	} else if (strcmp(context->request.command, "list") == 0) {
		hados_command_list(context);
	} else if (strcmp(context->request.command, "cluster_put") == 0) {
		hados_command_cluster_put(context);
	} else if (strcmp(context->request.command, "cluster_exists") == 0) {
		hados_command_cluster_exists(context);
	} else if (strcmp(context->request.command, "cluster_list") == 0) {
		hados_command_cluster_list(context);
	} else if (strcmp(context->request.command, "cluster_get") == 0) {
		hados_command_cluster_get(context);
	} else if (strcmp(context->request.command, "cluster_delete") == 0) {
		hados_command_cluster_delete(context);
	} else {
		hados_response_set_status(&context->response, HADOS_UNKNOWN_COMMAND,
				"Unknown command");
	}
}
