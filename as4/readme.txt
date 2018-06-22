To use the PL/0 compiler collection:

Compile the lexical analyzer to the binary "lex" (gcc -o lex lex.c), the code generator to the binary "codegen", and the virtual machine to the binary "vm" in the same directory as the compiler driver script. 

Run the compiler driver script with a PL/0 program as its argument:

./pl0cd.sh input1.txt

Make sure you have permission to execute the script (chmod +x pl0cd.sh) and make sure the files "tokens" and "binary" don't exist in the current working directory--or that you don't mind overwriting them and that you have have permission to do so. 
