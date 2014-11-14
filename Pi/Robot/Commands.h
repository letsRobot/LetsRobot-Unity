#ifndef Commands_h
#define Commands_h

#include "CommandFunctions.h"
#include <string>

void ExecuteShowHide(CommandFunctionParameters, bool showHide);
void Light(CommandFunctionParameters, int light, const std::string & color);
void Light(CommandFunctionParameters, int light, int r, int g, int b);
void SetLight(CommandFunctionParameters, int light, int r, int g, int b);

#endif
