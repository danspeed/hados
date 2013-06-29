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
