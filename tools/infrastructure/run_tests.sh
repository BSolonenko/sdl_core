#/bin/bash
run_test() {
    full_test_name="$1"
    cur_test="$2"
    test_count="$3"
    testname=${1##*/}
    fullpathname=${full_test_name%/*}
    cur_dir=$PWD
    echo "      Start $cur_test: $testname"

    cd $fullpathname
    test_output=`./$testname`
    res=`echo $?`
    cd $cur_dir

    testname=`echo $testname | sed -r ':l;/.{40}/q;s/.*/&./;bl'`
     cur_test_test_count=`echo $cur_test/$test_count | sed -r ':l;/.{5}/q;s/.*/& /;bl'`

    if [[ $res -eq 0 ]]; then
        test_result_text="   Passed"
        test_result=0
    else
        test_result_text="***Failed"
        test_result=1
        echo "\n $test_output \n"
    fi
    echo "$cur_test_test_count Test #$cur_test: $testname $test_result_text"
}

scan() {
    tests=$(find $1 -name '*_test');

    test_count=`echo "$tests" | wc -l`
    cur_test=$(( cur_tes+=1 ))

    for i in $tests; do
        run_test "$i" "$cur_test" "$test_count"
        failed_test=$(( failed_test+=$test_result ))
        cur_test=$(( cur_tes+=1 ))
    done

    echo "\n   $failed_test tests failed out of $test_count\n"
}

[ $# -eq 0 ] && dir=`pwd` || dir=$@

echo "Test project $dir"
scan "$dir"

exit $failed_test