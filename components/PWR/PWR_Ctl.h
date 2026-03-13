#ifndef __PWR_CTL_H__
#define __PWR_CTL_H__

#include "emMCP.h"

extern emMCP_tool_t relay;

void emMCP_SetRelayHandler(void *arg);
void emMCP_GetRelayHandler(void *arg);

#endif