/*
 * gdict.c
 *
 * Fixed-size word dictionary generator
 *
 * 20150106: Implementation
 *
*/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define NON_CONSEC "-nc"


void exit_on_error(char *argv0, int exit_code)
{
	fprintf(stderr,
		"Syntax: %s [" NON_CONSEC "] passphrase_length dictionary\n"
		NON_CONSEC ": avoid output repeating consecutive codes\n"
		"Example: %s 8 AEIOU1234567890\n", argv0, argv0);
	exit(exit_code);
}

int main(int argc, char **argv)
{
	int arg_next = 1;
	int non_consec = 0;
	if (argc < 3)
		exit_on_error(argv[0], 1);
	if (!strcmp(argv[arg_next], NON_CONSEC)) {
		non_consec = 1;
		arg_next++;
	}
	if (argc != arg_next + 2)
		exit_on_error(argv[0], 2);
	int passphrase_length = atoi(argv[arg_next]);
	if (passphrase_length <= 0)
		exit_on_error(argv[0], 3);
	char *chars = argv[arg_next + 1];
	const int num_chars = strlen(chars);
	char *pf = (char *)malloc(passphrase_length + 1);
	if (!pf)
		exit_on_error(argv[0], 4);
	int *c = (int *)malloc(passphrase_length * sizeof(int));
	if (!c)
		exit_on_error(argv[0], 5);
	pf[passphrase_length] = 10;
	int i = 0;
	for (; i < passphrase_length; c[i++] = 0);
	for (;;) {
		/* Non-consecutive character optimization:
		*/
		if (non_consec) {
			for (i = 1; i < passphrase_length; i++)
				if (c[i] == c[i - 1])
					break;
			if (i < passphrase_length)
				goto skip_passphrase;
		}
		/* Write down next passphrase:
		*/
		for (i = 0; i < passphrase_length; i++)
			pf[i] = chars[c[i]];
		write(1, pf, passphrase_length + 1);
skip_passphrase:
		/* Counter increment:
		 */
		for (i = 0; i < passphrase_length; i++) {
			int ci_next = c[i] + 1;
			if (ci_next < num_chars) {
				c[i] = ci_next;
				break;
			}
			if (i == (passphrase_length - 1))
				goto done;
			c[i] = 0;
		}
	}
done:
	free(c);
	free(pf);
	return 0;
}


