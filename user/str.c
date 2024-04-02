#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/string.h"

# define LEN 16

int main(void) {
	char buf[LEN];
	
	// strncpy
	printf("strncpy:\n");
	strncpy(buf, "Hello", LEN);
	printf("%s\n", buf);

	// strncmp
	printf("\nstrncmp:\n");
	
	if (!strncmp(buf, "Hello", LEN))
		printf("%s matches Hello\n", buf);
		
	if (strncmp(buf, "World", LEN))
		printf("%s does not match World\n", buf);

	// strncat
	printf("\nstrncat:\n");
	strncat(buf, " World12345678910", LEN - strlen(buf));
	printf("%s\n", buf);

	// strstr
	printf("\nstrstr:\n");
	char *substr = strstr(buf, "");
	printf("%s\n", substr);

	substr = strstr(buf, "Hell");
	printf("%s\n", substr);

	substr = strstr(buf, "123");
	printf("%s\n", substr);	

	substr = strstr(buf, "Yell");
	printf("%s\n", substr);	

	printf("\nstrspn:\n");

	// strspn
	int len = strspn(buf, "Hel");
	printf("len: %d, should be 4\n", len);

	len = strspn(buf, "hel");
	printf("len: %d, should be 0\n", len);

	printf("\nstrcspn:\n");

	// strcspn
	len = strcspn(buf, "Hel");
	printf("len: %d, should be 0\n", len);

	len = strcspn(buf, "hel");
	printf("len: %d, should be 1\n", len);

	len = strcspn(buf, " ");
	printf("len: %d, should be 5\n", len);

	// strtok
	printf("\nstrtok:\n");
    char strtokstr[] = "first second third";
    char *strtoktoken = strtok(strtokstr, " ");
    while (strtoktoken != 0) {
    	printf("%s\n", strtoktoken);
    	strtoktoken = strtok(0, " ");
    }
    // char *strtoktoken = strtok(strtokstr, " ");
    // printf("%s\n", strtoktoken);

	// strtok_r
	printf("\nstrtok_r:\n");
	char str[] = "First,sentence! Second sentence? Third-sentence.";
    char outer_delims[] = "!?.";
    char inner_delims[] = " ,-";
    char* token;
    char* outer_ptr = 0;
    char* inner_ptr = 0;
    
    token = strtok_r(str, outer_delims, &outer_ptr);
    while (token != 0) {
        printf("Outer Token: %s\n", token);
        char* inner_token = strtok_r(token, inner_delims, &inner_ptr);
 
        while (inner_token != 0) {
            printf("Inner Token: %s\n", inner_token);
            inner_token = strtok_r(0, inner_delims, &inner_ptr);
        }
 
        token = strtok_r(0, outer_delims, &outer_ptr);
    }

    // strsep
    printf("\nstrsep:\n");
    char strsepstr[] = "I love string functions";
    char strsepdelim[] = " ";
    char *strsepptr = strsepstr;
    char *strseptoken = strsep(&strsepptr, strsepdelim);
    
    while (strseptoken != 0) {
        printf("%s\n", strseptoken);
    	strseptoken = strsep(&strsepptr, strsepdelim);
    }

	// scanf tests
    printf("\nscanf:\n");
	int num1, num2;
	printf("Input two integers(space separated, can be negative): ");
	int result = scanf("%d %d", &num1, &num2);
	printf("%d %d %d\n", result, num1, num2);

	char *uninitialised = 0;
	char *initialised0 = malloc(128);
	char *initialised1 = malloc(128);
	memset(initialised0, 0, 128);
	memset(initialised1, 0, 128);
	
	printf("Input three strings(space separated): ");
	result = scanf("%s %s %s", initialised0, uninitialised,initialised1);
	printf("%d %s %s %s\n", result, initialised0, uninitialised, initialised1);

	free(initialised0);
	free(initialised1);
	return 0;
}
