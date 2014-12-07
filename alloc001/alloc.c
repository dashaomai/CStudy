#include <stdio.h>
#include <stdlib.h>

struct peer_info {
	int	index;
	char	name[64];
};

int main(int argc, char **argv) {
	struct peer_info *list;

	list = (struct peer_info*)calloc(5, sizeof(struct peer_info));

	fprintf(stdout, "The size of list is: %d bytes.\n", sizeof(list));
	fprintf(stdout, "The size of struct is: %d bytes.\n", sizeof(struct peer_info));
	fprintf(stdout, "The size of instance is: %d bytes.\n", sizeof(list[1]));

	fprintf(stdout, "The address of instance(s) is: 0x%x\t0x%x\t0x%x.\n", &list[0], &list[1], &list[2]);

	free(list);

	return 0;
}
