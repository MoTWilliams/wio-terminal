#ifndef COMMAND_H
#define COMMAND_H

typedef struct Button Button;

bool executing();
void executeButtonCommand(Button* b);
void receiveAndStartSerialCommand();
void mindSerialCommand();

#endif