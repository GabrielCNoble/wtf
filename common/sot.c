#include <stdio.h>



char tag[] = "[tag]";

int main()
{
	printf("%d\n", (sizeof(tag) + 3) & (~3));
}
