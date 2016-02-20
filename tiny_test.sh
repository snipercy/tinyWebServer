#! /bin/bash
# Tests Assignment 08 solution.

# Set the port to run on. You should change this because
# there might be other students running on the same port
# as you to run tiny!
TINYPORT=2000

# make sure you execute make before running this test!

# run tiny web server
./tiny $TINYPORT web/test_index.txt &
# wait until indexing is complete
sleep 7
# run curl
curl -s "http://localhost:$TINYPORT/search?value=cats">test/stest1.html
curl -s "http://localhost:$TINYPORT/search?value=death" > test/stest2.html
curl -s "http://localhost:$TINYPORT/search?value=death+Virginia" > test/stest3.html
curl -s "http://localhost:$TINYPORT/search?value=the" > test/stest4.html
curl -s "http://localhost:$TINYPORT/search?value=apple" > test/stest5.html
curl -s "http://localhost:$TINYPORT/search?value=orange" > test/stest6.html
curl -s "http://localhost:$TINYPORT/search?value=banana+pear" > test/stest8.html
curl -s "http://localhost:$TINYPORT/search?value=zero" > test/stest9.html
curl -s "http://localhost:$TINYPORT/search?value=chicken" > test/stest10.html
curl -s "http://localhost:$TINYPORT/search?value=food" > test/stest11.html

# kill tiny
##killall tiny

# compare
if diff -b test/test1.html test/stest1.html; then
    echo "test/stest1 PASSED"
fi

if diff -b test/test2.html test/stest2.html; then
    echo "test/stest2 PASSED"
fi

if diff -b test/test3.html test/stest3.html; then
    echo "test/stest3 PASSED"
fi

if diff -b test/test4.html test/stest4.html; then
    echo "test/stest4 PASSED"
fi

if diff -b test/test5.html test/stest5.html; then
    echo "test/stest5 PASSED"
fi

if diff -b test/test6.html test/stest6.html; then
    echo "test/stest6 PASSED"
fi

if diff -b test/test8.html test/stest8.html; then
    echo "test/stest8 PASSED"
fi

if diff -b test/test9.html test/stest9.html; then
    echo "test/stest9 PASSED"
fi

if diff -b test/test10.html test/stest10.html; then
    echo "test/stest10 PASSED"
fi

if diff -b test/test11.html test/stest11.html; then
    echo "test/stest11 PASSED"
fi

rm test/stest*
