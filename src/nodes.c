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

void hados_nodes_random_set(struct hados_nodes *nodes, int howmany, char val) {
	int i;
	int avail_nodes[nodes->length];
	int avail_count = 0;
	int already_set = 0;
	for (i = 0; i < nodes->length; i++) {
		if (nodes->array[i] == val)
			already_set++;
		else
			avail_nodes[avail_count++] = i;
	}
	// We have more nodes already set than expected
	if (already_set >= howmany)
		return;
	// We have less or equals nodes available than requested
	// All are set
	if (howmany >= avail_count) {
		for (i = 0; i < avail_count; i++)
			nodes->array[avail_nodes[i]] = val;
		return;
	}

	// Find random nodes
	long int rand = lrand48();
	howmany -= already_set;
	for (i = 0; i < howmany; i++)
		nodes->array[avail_nodes[(rand + i) % avail_count]] = val;
}
