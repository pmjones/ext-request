# copy userland source
rm -rf $1/src
cp -r ./userland/src $1

# copy userland tests
rm -rf $1/tests
cp -r ./tests $1
cp -r ./userland/tests/* $1/tests/

# copy test runner
cp ./run-tests.php $1
