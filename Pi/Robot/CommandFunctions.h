#ifndef CommandFunctions_h
#define CommandFunctions_h

#define AppendLineNumber2(identifier, lineNumber) identifier##lineNumber
#define AppendLineNumber1(identifier, lineNumber) AppendLineNumber2(identifier, lineNumber)
#define AppendLineNumber(identifier) AppendLineNumber1(identifier, __LINE__)

#include "Robot.h"
#include "IrcThread.h"
#include "ActualCommandParameters.h"

namespace CommandFunctions
{
   #define CommandFunctionParameters const ActualCommandParameters & parameters, Robot & robot, IrcThread & irc, bool & showCommands, bool & showChat
   #define CommandFunctionActualParameters parameters, robot, irc, showCommands, showChat
   typedef void (*CommandFunctionPointer) (CommandFunctionParameters);

   struct Node
   {
      const CommandFunctionPointer function;
      const char * const command;
      Node * next;

      Node(CommandFunctionPointer function, const char * command)
         : function(function),
           command(command),
           next(0)
      { }
   };

   Node *& First();
   Node *& Last();

   struct NodeAdder
   {
      NodeAdder(Node * node)
      {
         auto & first = First();
         auto & last = Last();

         if(!first)
            first = node;
         else
            last->next = node;

         last = node;
      }
   };
}

#define Command(command)                                                            \
namespace CommandFunctions                                                          \
{                                                                                   \
   void AppendLineNumber(ExecuteCommand) (CommandFunctionParameters);               \
   Node AppendLineNumber(node)(AppendLineNumber(ExecuteCommand), command);          \
   NodeAdder AppendLineNumber(nodeAdder)(&AppendLineNumber(node));                  \
}                                                                                   \
void CommandFunctions::AppendLineNumber(ExecuteCommand) (CommandFunctionParameters)

#endif
