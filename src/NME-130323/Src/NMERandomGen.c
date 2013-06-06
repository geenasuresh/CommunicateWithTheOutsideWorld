/**
 *	@file NMERandomGen.c
 *	@brief Generate random input for testing Nyctergatis Markup Engine.
 *	@author Yves Piguet.
 *	@copyright 2011, Yves Piguet.
 *
 *	@section nmerandomUsage nmerandom Usage
 *	This program filters standard input and writes the result to
 *	sandard output. It can be called as follows to convert file
 *	readme.nme to HTML file readme.html:
 *	@code
 *	./nmerandom options
 *	@endcode
 *	Here is the list of options it supports:
 *	- \c --ascii          ASCII output (default: random bytes)
 *	- \c --cr             CR (0x0D) for end-of-line (default: random)
 *	- \c --crlf           CRLF (0x0D 0x0A) for end-of-line (default: random)
 *	- \c --help           help message
 *	- \c --lf             LF (0x0A) for end-of-line (default: random)
 *	- \c --maxlinelength  max number of characters per line, or 0 for no limit
 *	- \c --seed           seed of the pseudorandom generator (default: 1)
 *	- \c --size           approximate size of output in bytes
 *
 *  To test nme in bash (Bourne Again Shell):
 *	@code
 *  do
 *  ./nmerandom --lf --ascii --size 10000 --seed $s | (./nme --test || echo $s)
 *  ./nmerandom --lf --ascii --size 10000 --seed $s | (./nme --checkhooks || echo $s)
 *  done
 *  @endcode
 */

/* License: new BSD license (see NME.h) */

/* To compile: gcc -o nmerandom NMERandomGen.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// Random generator (platform-independant, with uint32 seed or larger)
#define rnd() (seed = 1664525 * seed + 1013904223, (seed >> 24) & 0xff)

/// Application entry point
int main(int argc, char **argv)
{
	int i;
	int size = 1024;
	int maxLineLength = 60;
	enum { Random, CR, LF, CRLF } eol = Random;
	int ascii = 0;
	int count;	// number of characters generated
	int countForLine;	// number of characters generated
	unsigned long seed = 1;
	unsigned char c;
	
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "--size") && i + 1 < argc)
			size = strtol(argv[++i], NULL, 0);
		else if (!strcmp(argv[i], "--maxlinelength") && i + 1 < argc)
			maxLineLength = strtol(argv[++i], NULL, 0);
		else if (!strcmp(argv[i], "--ascii"))
			ascii = 1;
		else if (!strcmp(argv[i], "--cr"))
			eol = CR;
		else if (!strcmp(argv[i], "--lf"))
			eol = LF;
		else if (!strcmp(argv[i], "--crlf"))
			eol = CRLF;
		else if (!strcmp(argv[i], "--seed") && i + 1 < argc)
			seed = strtol(argv[++i], NULL, 0);
		else
		{
			if (strcmp(argv[i], "--help"))
				fprintf(stderr, "Unknown option %s\n", argv[i]);
			fprintf(stderr, "Usage: %s [options]\n"
					"Generate random input for testing Nyctergatis Markup Engine.\n"
					"--ascii           ASCII output (default: random bytes)\n"
					"--cr              CR (0x0D) for end-of-line (default: random)\n"
					"--crlf            CRLF (0x0D 0x0A) for end-of-line (default: random)\n"
					"--help            display this help message and exit\n"
					"--lf              LF (0x0A) for end-of-line (default: random)\n"
					"--maxlinelength   max number of characters per line, or 0 for no limit\n"
					"--seed            seed of the pseudorandom generator (default: 1)\n"
					"--size            approximate size of output in bytes\n",
				argv[0]);
			exit(0);
		}
	
	// skip first pseudorandom number to avoid zeros
	rnd();
	
	for (count = countForLine = 0; count < size; )
	{
		c = rnd();
		
		// eol: p = 2 %, or 20 % after an eol
		if (maxLineLength > 0 && countForLine >= maxLineLength
				|| c < 5 || countForLine == 0 && c < 51)
		{
			switch (eol)
			{
				case CR:
					printf("\x0d");
					count++;
					break;
				case LF:
					printf("\x0a");
					count++;
					break;
				case CRLF:
					printf("\x0d\x0a");
					count += 2;
					break;
				case Random:
				default:
					c = rnd();
					switch (c & 0xc0)
					{
						case 0:
							printf("\x0d");
							count++;
							break;
						case 0x80:
							printf("\x0a");
							count++;
							break;
						default:
							printf("\x0d\x0a");
							count += 2;
							break;
					}
					break;
			}
			countForLine = 0;
			continue;
		}
		
		// random number of special characters at beg. of line: p = 20 %
		c = rnd();
		if (countForLine == 0 && c < 51)
		{
			while (count < size)
			{
				c = rnd();
				switch ((c >> 5) & 7)
				{
					case 0:
						printf("*");
						break;
					case 1:
						printf("#");
						break;
					case 2:
						printf(":");
						break;
					case 3:
						printf(";");
						break;
					case 4:
						printf("|");
						break;
					default:
						printf("*");
						break;
				}
				count++;
				countForLine++;
				if (rnd() > 128)	// p = 50 % to break
					break;
			}
			continue;
		}
		
		// random style: p = 20 %
		c = rnd();
		switch (c >> 2)
		{
			case 0:
				printf("**");
				count += 2;
				countForLine += 2;
				continue;
			case 1:
				printf("//");
				count += 2;
				countForLine += 2;
				continue;
			case 2:
				printf("__");
				count += 2;
				countForLine += 2;
				continue;
			case 3:
				printf("##");
				count += 2;
				countForLine += 2;
				continue;
			case 4:
				printf(",,");
				count += 2;
				countForLine += 2;
				continue;
			case 5:
				printf("^^");
				count += 2;
				countForLine += 2;
				continue;
			case 6:
				printf("{{");
				count += 2;
				countForLine += 2;
				continue;
			case 7:
				printf("}}");
				count += 2;
				countForLine += 2;
				continue;
			case 8:
				printf("[[");
				count += 2;
				countForLine += 2;
				continue;
			case 9:
				printf("]]");
				count += 2;
				countForLine += 2;
				continue;
			case 10:
				printf("http://");
				count += 7;
				countForLine += 7;
				continue;
		}
		
		// random character
		for (;;)
		{
			c = rnd();
			if (!ascii || c >= 32 && c < 127)
			{
				printf("%c", c);
				count++;
				countForLine++;
				break;
			}
		}
	}
	
	return 0;
}
