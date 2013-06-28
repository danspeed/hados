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
 Name        : create_index.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include "hados.h"

int checkDataDir(const char *data_dir) {
	if (data_dir == NULL || strlen(data_dir) == 0) {
		return HADOS_DATADIR_NOT_SET;
	}
	struct stat s;
	int err = stat(data_dir, &s);
	if (err == -1) {
		perror("");
		return HADOS_DATADIR_DONT_EXISTS;
	}
	if (!S_ISDIR(s.st_mode)) {
		return HADOS_DATADIR_IS_NOT_A_DIRECTORY;
	}
	return HADOS_SUCCESS;
}

char* checkPath(struct hados_parameter *parameters, int *status) {
	char* path = hados_getParameter(parameters, "path");
	if (path == NULL ) {
		*status = HADOS_PATH_IS_MISSING;
		return NULL ;
	}
	if (strstr(path, "../") != NULL ) {
		*status = HADOS_WRONG_CHARACTER_IN_PATH;
		return NULL ;
	}
	if (strstr(path, "./") != NULL ) {
		*status = HADOS_WRONG_CHARACTER_IN_PATH;
		return NULL ;
	}
	if (strstr(path, "//") != NULL ) {
		*status = HADOS_WRONG_CHARACTER_IN_PATH;
		return NULL ;
	}
	char *pc = path;
	char c;
	int pos = 0;
	while ((c = *pc) != 0) {
		pc++;
		pos++;
		if (c == 37 || c == 43 || c == 47 || c == 46)
			continue;
		if (c >= 48 && c <= 57)
			continue;
		if (c >= 65 && c <= 90)
			continue;
		if (c >= 97 && c <= 122)
			continue;
		if (pos > HADOS_MAX_PATH_LENGTH) {
			*status = HADOS_PATH_TOO_LONG;
			return NULL ;
		}
		*status = HADOS_WRONG_CHARACTER_IN_PATH;
		return NULL ;
	}

	return path;
}

char* concatPath(const char *root_dir, const char *file_name, char *buffer) {
	strcpy(buffer, root_dir);
	if (file_name == NULL )
		return buffer;
	if (strlen(file_name) == 0)
		return buffer;
	if (file_name[0] != '/') {
		strcat(buffer, "/");
	}
	strcat(buffer, file_name);
	return buffer;
}

int mkdirs(const char *file_path) {
	struct stat st;
	char path[strlen(file_path) + 1];
	strcpy(path, file_path);
	char *p = path;
	int dirExists = 0;
	while (*p != 0) {
		if (*p == '/') {
			*p = 0;
			if (strlen(path) > 0) {
				dirExists = 0;
				if (stat(path, &st) == 0)
					if (S_ISDIR(st.st_mode))
						dirExists = 1;
				if (dirExists == 0) {
					if (mkdir(path, S_IRWXU) == -1) {
						perror("mkdirs");
						return HADOS_INTERNAL_ERROR;
					}
				}
			}
			*p = '/';
		}
		p++;
	}
	return HADOS_SUCCESS;
}

int hados_put(struct hados_context *context, struct hados_parameter *parameters) {
	int status = checkDataDir(context->data_dir);
	if (status != HADOS_SUCCESS)
		return status;
	char *path = checkPath(parameters, &status);
	if (status != HADOS_SUCCESS)
		return status;
	char file_path[strlen(path) + strlen(context->data_dir) + 2];
	concatPath(context->data_dir, path, file_path);
	status = mkdirs(file_path);
	if (status != HADOS_SUCCESS)
		return status;
	FILE *file = fopen(file_path, "wb");
	if (file == NULL ) {
		perror("hados_put fopen");
		return HADOS_INTERNAL_ERROR;
	}
	long bytes = 0;
	int c;
	while ((c = getchar()) != EOF) {
		bytes++;
		if (fputc(c, file) == EOF) {
			perror("hados_put fputc");
			status = HADOS_INTERNAL_ERROR;
			break;
		}
	}
	fclose(file);
	return status;
}

int hados_get(struct hados_context *context, struct hados_parameter *parameters) {
	int status = checkDataDir(context->data_dir);
	if (status != HADOS_SUCCESS)
		return status;
	char *path = checkPath(parameters, &status);
	if (status != HADOS_SUCCESS)
		return status;
	struct stat st;
	char file_path[strlen(path) + strlen(context->data_dir) + 2];
	concatPath(context->data_dir, path, file_path);
	if (stat(file_path, &st) != 0) {
		if (errno == ENOENT)
			return HADOS_OBJECT_NOT_FOUND;
		perror("hados_delete stat");
		return HADOS_INTERNAL_ERROR;
	}
	FILE *file = fopen(file_path, "rb");
	if (file == NULL ) {
		perror("hados_put fopen");
		return HADOS_INTERNAL_ERROR;
	}
	status = HADOS_BINARY_RESULT;
	printf("Content-type: application/octet-stream\r\n");
	printf("Content-Transfer-Encoding: binary\r\n\r\n");
	int c;
	while ((c = fgetc(file)) != EOF)
		putchar(c);
	fclose(file);
	return status;
}

int hados_delete(struct hados_context *context,
		struct hados_parameter *parameters) {
	int status = checkDataDir(context->data_dir);
	if (status != HADOS_SUCCESS)
		return status;
	char *path = checkPath(parameters, &status);
	if (status != HADOS_SUCCESS)
		return status;
	char file_path[strlen(path) + strlen(context->data_dir) + 2];
	concatPath(context->data_dir, path, file_path);
	struct stat st;
	if (stat(file_path, &st) != 0) {
		if (errno == ENOENT)
			return HADOS_OBJECT_NOT_FOUND;
		perror("hados_delete stat");
		return HADOS_INTERNAL_ERROR;
	}
	if (unlink(file_path) != 0) {
		perror("hados_delete unlink");
		return HADOS_INTERNAL_ERROR;
	}
	return HADOS_SUCCESS;
}

int hados_exists(struct hados_context *context,
		struct hados_parameter *parameters) {
	int status = checkDataDir(context->data_dir);
	if (status != HADOS_SUCCESS)
		return status;
	char *path = checkPath(parameters, &status);
	if (status != HADOS_SUCCESS)
		return status;
	struct stat st;
	char file_path[strlen(path) + strlen(context->data_dir) + 2];
	concatPath(context->data_dir, path, file_path);
	if (stat(file_path, &st) != 0) {
		if (errno == ENOENT)
			return HADOS_OBJECT_NOT_FOUND;
		perror("hados_exists stat");
		return HADOS_INTERNAL_ERROR;
	}
	return HADOS_OBJECT_FOUND;
}
