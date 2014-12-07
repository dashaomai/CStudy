#include <stdio.h>
#include <stdlib.h>

struct peer_info {
	int	index;
	char	name[64];
};

union endin_info {
	short	number;
	char	bytes[2];
};

int main(int argc, char **argv) {
	struct peer_info *list;

	list = (struct peer_info*)calloc(5, sizeof(struct peer_info));

	fprintf(stdout, "The size of list is: %d bytes.\n", sizeof(list));
	fprintf(stdout, "The size of struct is: %d bytes.\n", sizeof(struct peer_info));
	fprintf(stdout, "The size of instance is: %d bytes.\n", sizeof(list[1]));

	fprintf(stdout, "The address of instance(s) is: %d\t%d\t%d.\n", (int)&list[0], (int)&list[1], (int)&list[2]);

	free(list);

	union endin_info ei;
	ei.number = 0xAABB;
	fprintf(stdout, "The content of bytes: 0x%x 0x%x.\n", ei.bytes[0], ei.bytes[1]);

	return 0;
}
