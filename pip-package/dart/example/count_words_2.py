#This example count the words in each text file existing in a given folder.
#The execution is non-blocking. After submission of the parameters, the user is
#given a handle that can be used for waiting and collecting all the results.
#The results, containing detailed information about execution, are stored
#on the disk at a location given by the user.

import os
import sys

if (len (sys.argv)) < 2:
  raise Exception ("Please provide the folder containing the files to process!")

sys.path.insert (0, os.environ['DART_HOME'] + '/lib')
from dart import dart_context

dc = dart_context (sys.exec_prefix)

dc.start ('nodefile', 4)

parameters = dc.prepare_parameters (sys.argv[1])

handle = dc.async_run ('count_words', parameters)

list_results = dc.results.collect (handle)
dc.results.store (list_results, '/var/tmp')

for result in list_results:
  dc.results.print (result)

dc.stop()
