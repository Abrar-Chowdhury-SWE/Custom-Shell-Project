Abrar Chowdhury

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