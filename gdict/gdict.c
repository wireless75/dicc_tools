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

#define GN_OUT	"-g"
#define SK_OUT	"-s"
#define NC_OUT	"-nc"
#define NR_OUT	"-nr"
#define LE_OUT	"-le"

#ifdef _MSC_VER
	typedef unsigned __int64 u64t;
#else
	typedef unsigned long long u64t;
#endif

void exit_on_error(char *argv0, int exit_code)
{
	fprintf(stderr,
		"Syntax: %s [ " GN_OUT "n | " SK_OUT "n[K|M|G|T|P] " NC_OUT " " NR_OUT
		" " LE_OUT "] passphrase_length dictionary\n"
		GN_OUT "n[K|M|G|T|P]: generate n*10^r passphrases\n"
		SK_OUT "n[K|M|G|T|P]: skip first n*10^r passphrases\n"
		"(1 <= n <= 2^31, empty: r=0, K|M|G|T|P: r=3/6/9/12/15)\n"
		NC_OUT ": avoid output repeating consecutive codes\n"
		NR_OUT ": don't repeat codes (permutations)\n"
		LE_OUT ": little endian output (default is big endian)\n"
		"Examples:\n"
		"\t%s 8 AEIOU1234567890\n"
		"\t%s -s 12K 8 AEIOU1234567890\n",
		argv0, argv0, argv0);
	exit(exit_code);
}

u64t getval(int argc, char **argv, int arg_next)
{
	u64t r = 0;
	if (arg_next < argc) {
		r = (unsigned int)atoi(argv[arg_next]);
		size_t l = strlen(argv[arg_next]);
		if (l > 0) {
			switch (argv[arg_next][l - 1]) {
			case 'K': r *= 1000; break;
			case 'M': r *= 1000000; break;
			case 'G': r *= 1000000000; break;
			case 'T': r *= 1000000000000; break;
			case 'P': r *= 1000000000000000; break;
			}
		}
	}
	return r;
}

int main(int argc, char **argv)
{
	u64t gn = 0, sk = 0, gn_current = 0;
	int arg_next = 1;
	int no_consec = 0;
	int no_repeat = 0;
	int le_out = 0;
	if (argc < 3)
		exit_on_error(argv[0], 1);
	for (;; arg_next++) {
		if (!strcmp(argv[arg_next], GN_OUT)) {
			gn = getval(argc, argv, ++arg_next);
			continue;
		}
		if (!strcmp(argv[arg_next], SK_OUT)) {
			sk = getval(argc, argv, ++arg_next);
			continue;
		}
		if (!strcmp(argv[arg_next], NC_OUT)) {
			no_consec = 1;
			continue;
		}
		if (!strcmp(argv[arg_next], NR_OUT)) {
			no_repeat = 1;
			continue;
		}
		if (!strcmp(argv[arg_next], LE_OUT)) {
			le_out = 1;
			continue;
		}
		break;
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
	int i, j;
	for (i = 0; i < passphrase_length; c[i++] = 0);
	int inc0, incN, incD;
	if (le_out) {
		inc0 = 0;
		incN = passphrase_length;
		incD = 1;
	} else {
		inc0 = passphrase_length - 1;
		incN = -1;
		incD = -1;
	}
	for (;;) {
		/* Permutation and non-consecutive character optimization:
		*/
		if (no_repeat) {
			for (i = 0; i < passphrase_length; i++)
				for (j = 0; j < passphrase_length; j++)
					if (i != j && c[i] == c[j])
						goto skip_passphrase;
		}
		else
		if (no_consec) {
			for (i = 1; i <= passphrase_length; i++)
				if (c[i - 1] == c[i % passphrase_length])
					break;
			if (i <= passphrase_length)
				goto skip_passphrase;
		}
		/* Skip first n passphrases:
		 */
		if (sk) {
			sk--;
			goto skip_passphrase;
		}
		/* Write down next passphrase:
		*/
		for (i = 0; i < passphrase_length; i++)
			pf[i] = chars[c[i]];
		write(1, pf, passphrase_length + 1);
		/* Check for generation limit:
		 */
		if (gn && ++gn_current == gn)
			goto done;
skip_passphrase:
		/* Counter increment:
		 */
		for (i = inc0;;) {
			int ci_next = c[i] + 1;
			if (ci_next < num_chars) {
				c[i] = ci_next;
				break;
			}
			int i_next = i + incD;
			if (i_next == incN)
				goto done;
			c[i] = 0;
			i = i_next;
		}
	}
done:
	free(c);
	free(pf);
	return 0;
}


