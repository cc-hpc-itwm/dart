#This example count the words in each text file existing in a given folder.
#The execution is blocking, returning at the end a list of results that are
#stored at a given location, offering detaild information about the execution.
#The results are merged into one single result via a reduction operation.

import os
import sys

if (len (sys.argv)) < 2:
  raise Exception ("Please provide the folder containing the files to process!")

from dart import dart_context

dc = dart_context (sys.exec_prefix)

dc.start ('nodefile', 4)

parameters = dc.prepare_parameters (sys.argv[1])

list_results = dc.results.extract (dc.run ('count_words', parameters))
dc.results.store (list_results, '/var/tmp')

for result in list_results:
  dc.results.print (result)

dc.stop()
