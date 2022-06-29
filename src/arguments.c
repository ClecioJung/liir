/*
 *------------------------------------------------------------------------------
 * LIBRARIES
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arguments.h"

/*
 *------------------------------------------------------------------------------
 * DEFINITIONS
 *------------------------------------------------------------------------------
 */

#define TRUE ((int)1)
#define FALSE ((int)0)

/*
 *------------------------------------------------------------------------------
 * USER TYPES
 *------------------------------------------------------------------------------
 */

/* Table used to concentrate all the information related to the comand line arguments */
struct ArgCmd
{
    const char *cmd;
    size_t cmdLength;
    const char *alias;
    size_t aliasLength;
    int parameter;
    ArgFunction function;
    const char *usage;
};

/*
 *------------------------------------------------------------------------------
 * GLOBAL VARIABLES
 *------------------------------------------------------------------------------
 */

static struct ArgCmd *argList = NULL;
static unsigned int argNum = 0;
static ArgFunction usageFunction = NULL, defaultFunction = NULL;
static const char *software = NULL;

/*
 *------------------------------------------------------------------------------
 * FUNCTIONS
 *------------------------------------------------------------------------------
 */

int initArguments(const ArgFunction usageFunc, const ArgFunction defaultFunc)
{
    atexit(endArguments);
    usageFunction = usageFunc;
    defaultFunction = defaultFunc;
    return addArgument("--help", "-h", argumentsUsage, "Display this help message.");
}

void endArguments(void)
{
    free((struct ArgCmd *)argList);
    argList = NULL;
    argNum = 0;
    usageFunction = NULL;
    defaultFunction = NULL;
    software = NULL;
}

int addArgument(const char *const cmd, const char *const alias, ArgFunction function, const char *const usage)
{
    struct ArgCmd *newList;
    char *param;
    if (!cmd || !function)
    {
        return EXIT_FAILURE;
    }
    newList = (struct ArgCmd *)realloc((void *)argList, (argNum + 1) * sizeof(struct ArgCmd));
    if (!newList)
    {
        return EXIT_FAILURE;
    }
    argList = newList;
    argList[argNum].cmd = cmd;
    argList[argNum].cmdLength = strlen(cmd);
    argList[argNum].alias = alias;
    argList[argNum].aliasLength = alias ? strlen(alias) : 0;
    argList[argNum].parameter = FALSE;
    argList[argNum].function = function;
    argList[argNum].usage = usage;
    param = strchr(cmd, '=');
    if (param)
    {
        argList[argNum].parameter = TRUE;
        argList[argNum].cmdLength = param - cmd;
        if (alias)
        {
            param = strchr(alias, '=');
            if (param)
            {
                argList[argNum].aliasLength = param - alias;
            }
        }
    }
    argNum++;
    return EXIT_SUCCESS;
}

static int isCommand(const char *const arg, const struct ArgCmd *const command)
{
    const size_t length = strlen(arg);
    if (command->parameter)
    {
        if (!strncmp(arg, command->cmd, command->cmdLength))
        {
            return TRUE;
        }
        else if (command->alias && !strncmp(arg, command->alias, command->aliasLength))
        {
            return TRUE;
        }
    }
    else
    {
        if (length == command->cmdLength || length == command->aliasLength)
        {
            if (!strcmp(arg, command->cmd) || (command->alias && !strcmp(arg, command->alias)))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static int parseArgument(const char *const arg)
{
    unsigned int argCmd;
    char buf[100];
    for (argCmd = 0; argCmd < argNum; argCmd++)
    {
        if (isCommand(arg, &argList[argCmd]))
        {
            return argList[argCmd].function(arg);
        }
    }
    /* no argument found, try default action */
    if (defaultFunction)
    {
        return defaultFunction(arg);
    }
    sprintf(buf, "Unknown argument: %s", arg);
    argumentsUsage(buf);
    return EXIT_FAILURE;
}

int parseArguments(const int argc, const char *const argv[])
{
    int argIdx;
    software = argv[0];
    for (argIdx = 1; argIdx < argc; argIdx++)
    {
        if (parseArgument(argv[argIdx]))
        {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

static int max(const int x, const int y)
{
    return (x > y ? x : y);
}

void showListOfArguments(void)
{
    unsigned int argCmd;
    int cmdMaxLength = 0;
    int aliasMaxLength = 0;
    int whiteSpaces;
    for (argCmd = 0; argCmd < argNum; argCmd++)
    {
        cmdMaxLength = max(cmdMaxLength, strlen(argList[argCmd].cmd));
        if (argList[argCmd].alias)
        {
            aliasMaxLength = max(aliasMaxLength, strlen(argList[argCmd].alias));
        }
    }
    printf("[Options]:\n");
    for (argCmd = 0; argCmd < argNum; argCmd++)
    {
        printf("\t%-*s", cmdMaxLength, argList[argCmd].cmd);
        if (argList[argCmd].alias)
        {
            printf(" or %-*s ", aliasMaxLength, argList[argCmd].alias);
        }
        else
        {
            whiteSpaces = aliasMaxLength + 6;
            while (--whiteSpaces)
            {
                printf(" ");
            }
        }
        if (argList[argCmd].usage)
        {
            printf(": %s", argList[argCmd].usage);
        }
        printf("\n");
    }
}

int argumentsUsage(const char *const msg)
{
    if (msg && strlen(msg))
    {
        /* if is not an help command, show message */
        if (strcmp(msg, argList[0].cmd) && strcmp(msg, argList[0].alias))
        {
            printf("%s\n", msg);
        }
    }
    if (usageFunction)
    {
        if (usageFunction(software))
        {
            return EXIT_FAILURE;
        }
    }
    showListOfArguments();
    return EXIT_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * END
 *------------------------------------------------------------------------------
 */