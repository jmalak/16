/* Project 16 Source Code~
 * Copyright (C) 2012-2016 sparky4 & pngwen & andrius4669 & joncampbell123 & yakui-lover
 *
 * This file is part of Project 16.
 *
 * Project 16 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Project 16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>, or
 * write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include "src/lib/16_vrs.h"
#include "src/lib/typdefst.h"

// Read .vrs file into far memory
int read_vrs(global_game_variables_t *gvar, char *filename, struct vrs_container *vrs_cont){
	int fd;
	dword size;
#ifdef __WATCOMC__
	__segment seg;
	void __based(seg)* bigbuffer;
#endif
#ifdef __BORLANDC__
	memptr bigbuffer;
#endif
	byte huge *buffer;
	vrl1_vgax_offset_t **vrl_line_offsets;
	uint32_t huge *vrl_headers_offsets;
	uint16_t huge *vrl_id_iter;
	uint32_t vrl_size;
	int num_of_vrl, i;
	struct vrl1_vgax_header huge *curr_vrl;
	int success;

	// Open filename, get size of file,
	// populate the vrs_container if all tests pass
	fd = open(filename, O_RDONLY|O_BINARY);
	// Insert sanity cheks later
	size = lseek(fd, 0, SEEK_END);
	buffer = malloc(size);
	lseek(fd, 0, SEEK_SET);
	read(fd, buffer, size);
	close(fd);
	if(!success)
	{
		fprintf(stderr, "Unablee to load file");
		exit(3);
	}
	vrs_cont->data_size = size - sizeof(struct vrs_header);
	vrs_cont->buffer = buffer;

	// Calculate vrl offsets
	
	// Count sprites
	vrl_id_iter = (uint16_t huge *)(buffer + vrs_cont->vrs_hdr->offset_table[VRS_HEADER_OFFSET_SPRITE_ID_LIST]);
	while(vrl_id_iter[num_of_vrl]){
		num_of_vrl++;
	}
	// Allocate memory for vrl line offsets table
	vrl_line_offsets = malloc(sizeof(vrl1_vgax_offset_t *)*num_of_vrl);

	vrl_headers_offsets = (uint32_t huge *)(buffer + vrs_cont->vrs_hdr->offset_table[VRS_HEADER_OFFSET_VRS_LIST]);
	// Calculate line offsets for each vrl
	for(i = 0; i < num_of_vrl; i++){
		curr_vrl = (struct vrl1_vgax_header huge *)(buffer + vrl_headers_offsets[i]);

		// Calc. vrl size as (next_offset - curr_offset)
		if (i != num_of_vrl - 1){
			vrl_size = vrl_headers_offsets[i+1] - vrl_headers_offsets[i] - sizeof(struct vrl1_vgax_header);
		}
		// If it's the last vrl, size is (next_vrs_struct_offset - curr_offset)
		else{
			vrl_size = vrs_cont->vrs_hdr->offset_table[VRS_HEADER_OFFSET_SPRITE_ID_LIST] - vrl_headers_offsets[i] - sizeof(struct vrl1_vgax_header);
		}
		vrl_line_offsets[i] = vrl1_vgax_genlineoffsets(curr_vrl, (byte *)curr_vrl + sizeof(struct vrl1_vgax_header), vrl_size);
	}
	vrs_cont->vrl_line_offsets = vrl_line_offsets;
	return 0;
}

// Seek and return a specified .vrl blob from .vrs blob in far memory
int get_vrl_by_id(struct vrs_container /*huge*/ *vrs_cont, uint16_t id, struct vrl_container *vrl_cont){
	uint16_t huge *ids;
	uint32_t huge *vrl_offs_list;
	int counter = 0;

	// If id is invalid, return -1
	if(id == 0){
		// Probably add an error message?
		return -1;
	}

	// Get id list from .vrs blob (base + offset)
	ids = (uint16_t huge*)(vrs_cont->buffer + 
		vrs_cont->vrs_hdr->offset_table[VRS_HEADER_OFFSET_SPRITE_ID_LIST]);

	// Loop through the id list until we found the right one or hit the end of the list
	// Counter is keeping track of the offset(in ids/vrl blobs)
	while(ids[counter] != id && ids[counter]){
		counter++;
	}
	// Return -2 if we couldn't find the requested id
	if(!ids[counter]){
		// Error message?
		return -2;
	}

	// Get vrl offsets list from .vrs blob (base + offset)
	vrl_offs_list = (uint32_t huge *)(vrs_cont->buffer +
					vrs_cont->vrs_hdr->offset_table[VRS_HEADER_OFFSET_VRS_LIST]);

	// Get vrl_header from .vrs (base + offset from vrl_list)
	// Counter is number of vrls to skip (ids and vrls are aligned according to the .vrs specification)
	vrl_cont->vrl_header = (struct vrl1_vgax_header huge *)(vrs_cont->buffer + vrl_offs_list[counter]);

	// Get .vrl size by integer arithmetics (next vrl offset - current vrl offset)
	if(ids[counter+1]){
		vrl_cont->data_size = vrl_offs_list[counter+1] - vrl_offs_list[counter] - sizeof(struct vrl1_vgax_header);
	}
	// If we are retriving the last vrl, size is ids_list offset - current vrl offset, as next vrl offs is 0
	else{
		vrl_cont->data_size = vrs_cont->vrs_hdr->offset_table[VRS_HEADER_OFFSET_SPRITE_ID_LIST] - vrl_offs_list[counter] - sizeof(struct vrl1_vgax_header);
	}

	// Retrive line offsets form .vrs
	vrl_cont->line_offsets = vrs_cont->vrl_line_offsets[counter];

	return 0;
}
