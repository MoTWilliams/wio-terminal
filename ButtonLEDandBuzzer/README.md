# Sequence Generator -- Wio Terminal

A small embedded program that generates number sequences from button input, with LED and buzzer feedback. Built to keep logic simple and explicit by minimizing branching and using function pointers for sequence selection. 

Features debounced button input and real-time output via LED and Serial debug. Input handling, sequence logic, and control flow are kept separate fro easier reuse and extension.

## Build/Upload:
- Open in Arduino IDE
- Select Wio Terminal board
- Upload to device

## Controls:
**Button A**: Generate next odd number  
**Button B**: Generate next Fibonacci number  
**Button C**: Reset both sequences
