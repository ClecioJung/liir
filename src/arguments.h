#ifndef __ARGUMENTS
#define __ARGUMENTS

/*
 *------------------------------------------------------------------------------
 * USER TYPES
 *------------------------------------------------------------------------------
 */

typedef int (*ArgFunction)(const char *const arg);

/*
 *------------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 *------------------------------------------------------------------------------
 */

int initArguments(const ArgFunction usageFunc, const ArgFunction defaultFunc);
void endArguments(void);
int addArgument(const char *const cmd, const char *const alias, ArgFunction function, const char *const usage);
int parseArguments(const int argc, const char *const argv[]);
int argumentsUsage(const char *const msg);

/*
 *------------------------------------------------------------------------------
 * END
 *------------------------------------------------------------------------------
 */
#endif /* __ARGUMENTS */
