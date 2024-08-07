#!/usr/bin/env python3.6
import sys
import requests
from parse_log import parse_log

print("Sys args: ", sys.argv)
if __name__ == '__main__':
  client = requests.session()

  # Get CSRF token
  url = 'http://localhost:8000/elog/boardtest/create/'
  client.get(url)
  csrftoken = client.cookies['csrftoken']

  print("Potentially useless input argument" + sys.argv[1])

  #Sets board id and datetime
  #Note (7/16/24 from Anders): This seems to be working fine, but the log does not appear in the odmb database :( 
  test_board_id = 'odmb_test#1'
  if(len(sys.argv)>1):
    test_board_id = sys.argv[1]
  print("board tested: " + test_board_id)

  test_date_time = '2024-04-26+21:23:51'
  if(len(sys.argv)>3):
    test_date_time = sys.argv[3]
    data_time = test_date_time.split("_")
    data_time[1] = data_time[1].replace("-",":")
    test_date_time = data_time[0] + "+" + data_time[1] 
  print("Date/time: " + test_date_time)


  # Post data
  data = {'csrfmiddlewaretoken':csrftoken,
          'boardtest-board':test_board_id, 'boardtest-date':test_date_time, 'boardtest-location':'1', 'testfilter-prom test': '1', 'testfilter-vme basic test': '1', 'testfilter-fpga clock test': '1', 'testfilter-sysmon test': '1', 'testfilter-ccb test': '1', 'testfilter-otmb test': '1', 'testfilter-lvmb test': '1', 'testfilter-dcfeb jtag test': '1', 'testfilter-dcfeb fast signal test': '1', 'submit_test': 'Submit'
         }
  data = {**data, **parse_log(sys.argv[2])}
  print(data)
  req = client.post(url, data=data)
  #print(req)
  #print(req.status_code)
  #print(req.reason)
  #print(req.text)
