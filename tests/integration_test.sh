#!/bin/bash

# working directory is set to /tests/ so we need PATH_TO_BIN to access server binary through ../build/
PATH_TO_BIN="../build/bin/server"
url="http://localhost:8080"

echo "running integration test"

function start_server {
    $PATH_TO_BIN ../new_format_config &
    server_pid=$!
    echo $server_pid
}

function stop_server {
    kill -9 $server_pid
}

function output_test_result {
    ret=$?
    if [ $ret -eq 0 ]; then
        echo "SUCCESS $1";
    else
        echo "FAILED $1";
        stop_server;
        exit 1;
    fi
}

start_server
sleep 1

# TEST 1: Tests static request handler for 400 Bad Request
echo "test" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response actual_response
output_test_result 1
rm actual_response


# TEST 2: Tests echo request handler
echo "$(cat ./request_1)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response2 actual_response
output_test_result 2
rm actual_response

# TEST 3: Tests .txt request for StaticRequestHandler
echo "$(cat ./http_requests/text_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response3 actual_response
output_test_result 3
rm actual_response

# TEST 4: Tests .jpg request for StaticRequestHandler
echo "$(cat ./http_requests/jpg_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response4 actual_response
output_test_result 4
rm actual_response

# TEST 5: Tests .zip request for StaticRequestHandler
echo "$(cat ./http_requests/zip_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response5 actual_response
output_test_result 5
rm actual_response

# TEST 6: Tests .html request for StaticRequestHandler
echo "$(cat ./http_requests/html_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response6 actual_response
output_test_result 6
rm actual_response

# TEST 7: Tests .png request for StaticRequestHandler
echo "$(cat ./http_requests/png_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response7 actual_response
output_test_result 7
rm actual_response

# TEST 8: Tests PUT request for StaticRequestHandler
echo "$(cat ./http_requests/put_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response8 actual_response
output_test_result 8
rm actual_response

# TEST 9: Tests .html request through CURL for StaticRequestHandler
curl http://localhost:8080/static1/text/sample.html > actual_response
sleep 1
diff curl_expected_res/expected_curl1 actual_response
output_test_result 9
rm actual_response

# TEST 10: Tests .jpg request through CURL for StaticRequestHandler
curl http://localhost:8080/static1/images/orange-sun-small.jpg > actual_response
sleep 1
diff curl_expected_res/expected_curl2 actual_response
output_test_result 10
rm actual_response

# TEST 11: Tests .txt request through CURL for StaticRequestHandler
curl http://localhost:8080/static1/text/file1.txt > actual_response
sleep 1
diff curl_expected_res/expected_curl3 actual_response
output_test_result 11
rm actual_response

# TEST 12: Tests .png request through CURL for StaticRequestHandler
curl http://localhost:8080/static1/images/hehe-kitty.png > actual_response
sleep 1
diff curl_expected_res/expected_curl4 actual_response
output_test_result 12
rm actual_response

# TEST 13: Tests echo request through CURL for EchoRequestHandler
curl http://localhost:8080/echo > actual_response
sleep 1
diff curl_expected_res/expected_curl6 actual_response
output_test_result 13
rm actual_response

# TEST 14: Tests error request with base path through CURL for ErrorRequestHandler
curl http://localhost:8080/ > actual_response
sleep 1
diff curl_expected_res/expected_curl7 actual_response
output_test_result 14
rm actual_response

# TEST 15: Tests error request with incorrect path through CURL for ErrorRequestHandler
curl http://localhost:8080/blahblahhhhh > actual_response
sleep 1
diff curl_expected_res/expected_curl8 actual_response
output_test_result 15
rm actual_response

# TEST 16: Tests error request for ErrorRequestHandler
echo "$(cat ./http_requests/error_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response9 actual_response
output_test_result 16
rm actual_response

# Before CRUD tests, delete testing directory if it exists 
# to ensure consistent results
rm -rf ../api_dir/integration_test

# TEST 17: Create entity for ApiRequestHandler
echo "$(cat ./http_requests/crud_create_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response10 actual_response
output_test_result 17
rm actual_response

# TEST 18: Read entity for ApiRequestHandler
echo "$(cat ./http_requests/crud_read_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response11 actual_response
output_test_result 18
rm actual_response

# TEST 19: Update entity for ApiRequestHandler
echo "$(cat ./http_requests/crud_update_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response12 actual_response
output_test_result 19
rm actual_response

# TEST 20: Make sure PUT updated properly for ApiRequestHandler
echo "$(cat ./http_requests/crud_read_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response13 actual_response
output_test_result 20
rm actual_response

# TEST 21: Make sure LIST shows IDs properly for ApiRequestHandler
echo "$(cat ./http_requests/crud_list_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response16 actual_response
output_test_result 21
rm actual_response

# TEST 22: Delete entity for ApiRequestHandler
echo "$(cat ./http_requests/crud_delete_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response14 actual_response
output_test_result 22
rm actual_response

# TEST 23: Make sure DELETE updated properly for ApiRequestHandler
echo "$(cat ./http_requests/crud_read_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response15 actual_response
output_test_result 23
rm actual_response

# After crud tests, remove integration_test dir
rm -rf ../api_dir/integration_test

# TEST 24: Tests multithreading
# Different format than our previous integration tests because it requires checking a few things in one test.
curl localhost:8080/sleep/3 > ./sleep.txt &
curl -s -I localhost:8080 > ./temp.txt
sleep 1 
sleeplines=`wc -l ./sleep.txt | awk '{print $1}'` # should be 0 here because it has not returned yet 
templines=`wc -l ./temp.txt | awk '{print $1}'` # Should have returned already, should be bigger than 1
if [ $sleeplines -eq 0 ] && [ $templines -gt 0 ] ; then 
    sleep 2 # block should end 
    sleeplines2=`wc -l ./sleep.txt | awk '{print $1}'`
    if [ $sleeplines2 -gt 0 ] ; then 
        echo "SUCCESS 24" 
    else 
        echo "FAILED 24 - Block Handler not returning on time"
        stop_server
        exit 1
    fi 
else 
    echo "FAILED 24 - Not multithreaded" 
    stop_server 
    exit 1 
fi 
rm ./sleep.txt ./temp.txt 

# TEST 25: tests health request for HealthRequestHandler
echo "$(cat ./http_requests/health_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response17 actual_response
output_test_result 25
rm actual_response

# TEST 26: tests GET /board request which should return the form
echo "$(cat ./http_requests/board_form_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response18 actual_response
output_test_result 26
rm actual_response

# TEST 27: tests create board request
echo "$(cat ./http_requests/board_createboard_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response19 actual_response
output_test_result 27
rm actual_response

# TEST 28: tests create note request
echo "$(cat ./http_requests/board_createnote_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response20 actual_response
output_test_result 28
rm actual_response

# TEST 29: tests get boards request
echo "$(cat ./http_requests/board_getboards_req)" | nc localhost 8080 > actual_response &
nc_pid=$!
sleep 1
kill -9 $nc_pid
diff integr_test_nc_expected_res/expected_response21 actual_response
output_test_result 29
rm actual_response

# After board tests, remove board dir
rm -rf ../board

stop_server
exit 0
