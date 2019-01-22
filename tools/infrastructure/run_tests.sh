#!/usr/bin/env sh

# Copyright (c) 2019, Ford Motor Company
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following
# disclaimer in the documentation and/or other materials provided with the
# distribution.
#
# Neither the name of the Ford Motor Company nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Using:
# use the -V flag for print the test output if the test fails
VERBOSE=$1

run_test() {
    full_test_name="$1"
    test_name=${1##*/}
    full_path_name=${full_test_name%/*}
    cur_dir=$PWD

    cd $full_path_name
    test_output=`./$test_name 2>&1`
    test_result=`echo $?`
    cd $cur_dir

    if [[ $test_result -ne 0 ]]; then
        test_result=1
        if [[ $2 == "-V" ]]; then
            echo "\n $test_output \n"
        fi
    fi

    echo $test_result
}

print_test_result(){
    test_name=`echo ${1##*/} | sed -r ':l;/.{40}/q;s/.*/&./;bl'`
    cur_test_test_count=`echo $2/$3 | sed -r ':l;/.{5}/q;s/.*/& /;bl'`
    test_result=$4

    if [[ $test_result -eq 0 ]]; then
        test_result_text="   Passed"
    else
        test_result_text="***Failed"
    fi

    echo "$cur_test_test_count Test #$2: $test_name $test_result_text"
}

run_all_tests() {
    echo "Test project $PWD"
    tests=$(find $PWD -name '*_test')
    test_count=`echo "$tests" | wc -l`
    cur_test=1
    failed_test_count=0

    for i in $tests; do
        echo "      Start $cur_test: ${i##*/}"
        test_result=`run_test "$i" "$1"`
        print_test_result "$i" "$cur_test" "$test_count" "$test_result"
        if [[ $test_result -ne 0 ]]; then
            failed_tests="$failed_tests\n      $test_name"
            (( failed_test_count+=1 ))
        fi
        (( cur_test+=1 ))
    done

    echo "\n   $failed_test_count tests failed out of $test_count"
    echo "$failed_tests \n"
}

run_all_tests "$VERBOSE"

exit $failed_test_count
