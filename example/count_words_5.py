#This example count the words in each text file existing in a given folder.
#The execution is non-blocking. After submission of the parameters, the user is
#given a handle that can be used for collecting all the results.
#For each result monitoring informatio is pushed into an influx database
#available at the url given as send parameter dart_context constructor

import os
import sys

if (len (sys.argv)) < 2:
  raise Exception ("Please provide the folder containing the files to process!")

from dart import dart_context

#Note: assume using the module soft/dartmonitor and port tunelling is done
dc = dart_context (sys.exec_prefix, 'http://localhost:8086', name='count_words')

dc.start ('nodefile', 4)

parameters = dc.prepare_parameters (sys.argv[1])

handle = dc.async_run ('count_words', parameters)

while True:
  result = dc.results.pop (handle)
  if not result:
    break
  dc.results.print (result)
  dc.results.store ([result], '/var/tmp')

dc.stop()
