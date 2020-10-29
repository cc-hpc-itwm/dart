#This example count the words in each text file existing in a given folder.
#The execution is non-blocking. After the submission of parameters, the user is
#given a handle that can be used for collecting all the results.
#A progress bar is displyed showing the percentage of tasks left.
#The results, containing detailed information about execution, are stored
#on the disk at a location given by the user (e.g. /var/tmp).

import os
import sys

if (len (sys.argv)) < 2:
  raise Exception ("Please provide the folder containing the files to process!")

from dart import dart_context

dc = dart_context (sys.exec_prefix)

dc.start ('nodefile', 4)

parameters = dc.prepare_parameters (sys.argv[1])

handle = dc.async_run ('count_words', parameters)

dc.show_progress_and_store_results (handle, '/var/tmp')

dc.stop()
