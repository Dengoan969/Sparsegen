source create_test.sh
make 
./sparse-gen A.dat B.dat
gzip -c A.dat > A.gz
gzip -c B.dat > B.gz
gzip -cd B.gz | ./sparse-gen C.dat
./sparse-gen A.dat D.dat -b 100
stat -c '%n -- %b blocks with %B bytes in block' *.dat *.gz > result.txt