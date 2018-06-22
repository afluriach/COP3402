# COP3402
COP3402: System Software (UCF, 2011)

This is the class project for System Software. It is a lexer/parser, compiler, and stack-based virtual machine for PL//0.
PL/0 is a simplified version of Pascal. However, for extra credit, I added support for passing parameters into procedures and having them support returning values.
The parser uses a recursive descent, LR/1 grammar; procedure parameters and return values are pushed onto the stack as is common for other stack-based VMs.