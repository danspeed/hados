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
 Name        : nodes.c
 Author      : Emmanuel Keller
 Version     :
 Copyright   : Jaeksoft / Emmanuel Keller
 ============================================================================
 */

#include "hados.h"

void hados_nodes_init(struct hados_nodes *nodes, int length) {
	nodes->array = NULL;
	nodes->length = length;
	if (length == 0)
		return;
	nodes->array = malloc(length * sizeof(char));
	int i;
	for (i = 0; i < length; i++)
		nodes->array[i] = 0;
}

void hados_nodes_free(struct hados_nodes *nodes) {
	if (nodes->array != NULL ) {
		free(nodes->array);
		nodes->array = NULL;
		nodes->length = 0;
	}
}

void hados_nodes_set(struct hados_nodes *nodes, int pos, char val) {
	if (pos > nodes->length)
		return;
	if (pos < 0)
		return;
	nodes->array[pos] = val;
}
