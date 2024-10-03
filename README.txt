Prompt format When running in interactive mode, mysh will print a prompt to indicate that it
is ready to read input. The prompt is normally the string “mysh> ” (note the trailing space).

Usage Batch mode:
$ cat myscript.sh
echo hello
$ ./mysh myscript.sh
hello
$
Batch mode with no specified file:
$ cat myscript.sh | ./mysh
hello
$

Interactive mode:
$ ./mysh
Welcome to my shell!
mysh> cd subdir
mysh> echo hello
hello
mysh> cd subsubdir
mysh> pwd
/current/path/subdir/subsubdir
mysh> cd directory_that_does_not_exist
cd: No such file or directory
mysh> cd ../..
mysh> exit
mysh: exiting
$




Test Plan:

We create a sh file that includes these commands:

echo my
echo bash
mkdir TestDir
cd TestDir
pwd
cd ..
pwd
"Hello" > outfile.txt
sort < infile.txt
ls *.txt
ls | sort
exit
mkdir bla
echo "Do Not Print"

We test that our implemented commands work properly and that the program exits correctly. 
We also test that the shell is able to interact with the file system and that we can traverse through the folders made by the shell

For <, >, and |, we do 
echo "Hello" > outfile.txt,
sort > infile.txt,
ls | sort

test commands:

make
./mysh myscript.sh
./mysh