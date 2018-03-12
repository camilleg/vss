#include "platform.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctype.h>

/*	Convert a string into an argv style of array
 *	This honors both single and double quotes.  The quote
 *	characters can be escaped with a preceding backslash.
 *	Inputs:		cmd		Optional string to use as argv[0].  Ignored if NULL.
 *				sz		Pointer to the string to convert
 *	Outputs:	argc	Pointer to a location for the argument count.
 *				argv	Pointer to a location to hold the argv pointer.
 *	Note: new/delete cause a link error, so I'm using malloc/free.
 */
static int ScanArgv(char *cmd, const char *sz, int *argc, char ***argv)
{
	char **wargv = NULL;
	int wargc;
	int len;
	const char *pSrc;
	char* pDst;
	const char *pSrc2;
	char *pDst2;
	char quote;

	if (!sz)
		return 0;

	// Count the number of fields, including the command if applicable
	wargc = cmd ? 1 : 0;
	for (pSrc=sz; *pSrc; )
		{
		while ((*pSrc) && isspace(*pSrc))
			pSrc++;
		if (*pSrc)
			{
			quote = (*pSrc == '"' || *pSrc == '\'') ? *pSrc++ : '\0';
			if (quote)
				{
				for (pSrc2=pSrc; (*pSrc2) && (*pSrc2 != quote); )
					{
					if (*pSrc2 == '\\')
						{
						switch (*(pSrc2+1))
							{
						case '\'':
						case '"':
							pSrc2++;
							break;
						default:
							break;
							}
						}
					pSrc2++;
					}
				}
			else
				{
				for (pSrc2=pSrc; (*pSrc2) && !isspace(*pSrc2); )
					pSrc2++;
				}
			wargc++;
			pSrc = pSrc2;
			}		
		}

	// Get a buffer large enough to hold the argv and the string

	len = strlen(sz) + 1;
	if (cmd)
		len += strlen(cmd) + 1;
	len += (sizeof(char *)) * wargc;
//	wargv = (char**) new char[len];
	wargv = (char**)malloc(len);
	if (!wargv)
		{
		fprintf(stderr, "vss error: out of memory.\n");
		return 0;
		}

	// Move the caller's string to the buffer
	pDst = ((char*)wargv) + ((sizeof(char*)) * wargc);
	strcpy(pDst, sz);

	// If the caller provided a command, move it in also
	wargc = 0;
	if (cmd)
		{
		pDst2 = pDst + strlen(pDst) + 1;
		strcpy(pDst2, cmd);
		*(wargv+(wargc++)) = pDst2;
		}

	// Parse the string, splitting into the component fields
	while (*pDst)
		{
		while (*pDst && isspace(*pDst))
			pDst++;
		if (*pDst)
			{
			quote = (*pDst == '"' || *pDst == '\'') ? *pDst++ : '\0';
			if (quote)
				for (pDst2=pDst; (*pDst2) && (*pDst2 != quote); )
					pDst2++;
			else
				for (pDst2=pDst; (*pDst2) && (! isspace(*pDst2)); )
					pDst2++;
			if (*pDst2)
				*pDst2++ = '\0';
			*(wargv+(wargc++)) = pDst;
			pDst = pDst2;
			}		
		}

	*argc = wargc;
	*argv = wargv;
	return 1;
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	int argc = 0;
	char **argv = NULL;
	if (!ScanArgv("vss", lpCmdLine, &argc, &argv))
		return -1;

	int w = VSS_main(argc, argv /*, hInstance*/);
//	delete [] argv;
	free(argv);
	return w;
}

