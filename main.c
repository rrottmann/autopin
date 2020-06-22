#include <stdio.h>	 // printf
#include <stdlib.h>	 // arc4random
#include <sys/stat.h>    // stat
#include <stdbool.h> 	 // bool type
#include <stdint.h> 	 // uint8_t type
#include <string.h> 	 // strcpy, strcat
#include <ctype.h> 	 // tolower
#include <unistd.h> 	 // getpid
#include "base64.h"	 // b64_encode
#include "hmac-sha256.h" // hmac-sha256

#define BUF_SIZE 80
#define FILENAME "/etc/pinentry_salt"

bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

void arc4random_buf(void *buf, size_t nbytes);

int chmod(const char *pathname, mode_t mode);

int tolower(int argument);

char * load_salt(char *filename) {
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	static char str[BUF_SIZE];
	//char *strptr = malloc(fsize + 1);
	char *strptr = str;
	fread(strptr, 1, fsize, f);
	fclose(f);
	strptr[fsize + 1] = 0;
	return strptr;
}

char * create_salt(char *filename) {
	/* read random bytes */
	static char str[32];
	size_t nbytes = 32;
	char *strptr;
	strptr = str;
	arc4random_buf(strptr, nbytes);
	/* b64_encode random bytes */
	char *in = str;
	int in_len = sizeof(str);
	static char b64str[BUF_SIZE];
	char *out = b64str;
	int ret = -1;
	ret = b64_encode(in, in_len, out);
	/* write salt file */
	FILE *f = fopen(FILENAME, "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	const char *b64strptr = b64str;
	fprintf(f, "%s", b64str);
	fclose(f);
	return b64str;
}

char * machine_id(void) {
	char *cmd = "sh -c '(cat /etc/machine-id;sha256sum /etc/ssh/ssh_host*;head -1 /proc/meminfo;grep -i model /proc/cpuinfo)|sha256sum| cut -d\\  -f1'";
	static char buf[BUF_SIZE];
	FILE *fp;
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}
	fgets(buf, sizeof(buf), fp);
	pclose(fp);
	return buf;

		
}

int main(void) {
	/* generate or load salt */
	static char salt[BUF_SIZE] = "";
	static char *saltptr;
	saltptr = salt;
	/* check whether salt file exists */
	if (file_exists(FILENAME)) {
		/* using salt file */
		saltptr = load_salt(FILENAME);
		//printf("Salt loaded from %s: %s\n", FILENAME, saltptr);
	} else {
		/* creating salt file */
		saltptr = create_salt(FILENAME);
		chmod(FILENAME, S_IRUSR);
		//printf("Salt created in %s: %s\n", FILENAME, saltptr);
	}
	/* the variable pepper gets injected at compile time */
	//printf("Pepper embedded at compile time: %s\n", pepper);
	//printf("Salt from current system: %s\n", saltptr);
	/* create a machine id */
	static char mid[BUF_SIZE];
	char *midptr = mid;
	midptr = machine_id();
	//printf("Machine ID unique to this system: %s", midptr);
	/* concat pepper and machine id */
	static char *hmac_key = NULL;
	hmac_key = malloc(strlen(pepper) + strlen(mid) + 1);
	strcpy(hmac_key, pepper);
	strcat(hmac_key, midptr);
	//printf("HMAC key to generate pin: %s", hmac_key);
	/* create hmac_sha256 */
	static char output[BUF_SIZE];
	uint8_t *out = output;
	uint8_t *data = saltptr;
	size_t data_len = sizeof(saltptr);
	uint8_t *key = hmac_key; 
	size_t key_len = sizeof(hmac_key);
	hmac_sha256(out, data, data_len, key, key_len);
	//printf("HMAC binary representation: %s\n", output);
	/* b64_encode hmac_sha256 */
	char *inptr = output;
	int in_len = sizeof(output);
	char b64buf[BUF_SIZE];
	char *b64bufptr = b64buf;
	int ret = -1;
	ret = b64_encode(inptr, in_len, b64bufptr);
	//printf("HMAC base64 reprensentation: %s\n", b64buf);
	/* create pin */
	char pin[17] = "";
	char *pinptr = pin;
	strncpy(pin, b64buf, 8);
	/* we react to gpg pinentry commands */
	static char cmd[BUF_SIZE] = "";
	static char *cmdptr = cmd;
	printf("%s", pin);
		
	return 0;
}
