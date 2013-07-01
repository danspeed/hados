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
 Name        : utils.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

/**
 * Concat the root_dir with the file_name using a / separator.
 */
char* hados_utils_concat_path(const char *root_dir, const char *file_name,
		char *buffer) {
	strcpy(buffer, root_dir);
	if (file_name == NULL )
		return buffer;
	if (strlen(file_name) == 0)
		return buffer;
	if (file_name[0] != '/')
		strcat(buffer, "/");
	strcat(buffer, file_name);
	return buffer;
}

/**
 * Concat two strings, by allocating or reallocating memory if required.
 */
char* hados_utils_strcat(char* str, const char* add) {
	if (add == NULL )
		return NULL ;
	if (strlen(add) == 0)
		return str;
	if (str == NULL )
		return strdup(add);
	size_t size = strlen(str) + strlen(add) + 1;
	str = realloc(str, size * sizeof(char));
	strcat(str, add);
	return str;
}

/**
 * Create all the required directory to build the path in the file system
 */
int hados_utils_mkdirs(const char *file_path, struct hados_response *response) {
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
				if (dirExists == 0)
					if (mkdir(path, S_IRWXU) == -1)
						return hados_response_set_errno(response);
			}
			*p = '/';
		}
		p++;
	}
	return hados_response_set_success(response);
}

int hados_utils_mkdir_if_not_exists(struct hados_context *context,
		const char *dir_path) {
	struct stat st;
	int err = stat(dir_path, &st);
	if (err == 0) {
		if (!S_ISDIR(st.st_mode)) {
			hados_context_error_printf(context,
					"File path is not a directory: %s", context->file_dir);
			err = -1;
		}
	} else {
		if (errno == ENOENT) {
			err = mkdir(dir_path, S_IRWXU);
		} else
			hados_context_error_printf(context,
					"Issue while creating directory %s", dir_path);
	}
	return err;
}

