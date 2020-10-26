import binascii
from collections import Counter
from functools import reduce
import json
import os
import pickle
import requests
import signal
import sys
from urllib.parse import urlparse
import uuid

class catch_stderr:
  def __init__(self):
    self.value = ''
  def write (self, txt):
    self.value += txt
catch_stderr = catch_stderr()

class catch_stdout:
  def __init__(self):
    self.value = ''
  def write (self, txt):
    self.value += txt
catch_stdout = catch_stdout()

from DART import runtime

class dart_context (runtime):
  def __init__(self, python_home, monitor_url = '/var/tmp', **kwargs):
    super (dart_context, self).__init__(python_home, kwargs)
    if 'name' in kwargs:
      job_name = kwargs['name'] + '_' + uuid.uuid4().hex
    else:
      job_name = uuid.uuid4().hex

    url = urlparse (monitor_url)
    if (url.scheme == 'http'):
      try:
        request = requests.get (monitor_url)
      except:
        print ('No monitoring database found at ' + monitor_url + '!')
        sys.exit (1);

    self.results = self.results (self.pop_result, self.collect_results, monitor_url, job_name)
    signal.signal (signal.SIGTERM, self.on_exit) # term
    signal.signal (signal.SIGABRT, self.on_exit) # abort
    signal.signal (signal.SIGINT, self.on_exit)  # keyboard interrupt
    signal.signal (signal.SIGSEGV, self.on_exit) # segfault

  def on_exit (self, signum, handler):
    try:
      self.stop()
      sys.exit (signum)
    except:
      os._exit (2)

  @staticmethod
  def catch_out():
    sys.stdout = catch_stdout

  @staticmethod
  def catch_err():
    sys.stderr = catch_stderr

  @staticmethod
  def pack (results):
    return binascii.b2a_base64 (pickle.dumps (results))

  @staticmethod
  def unpack (results):
    return pickle.loads (binascii.a2b_base64 (results))

  def prepare_parameters (self, base_url, path = '', config = {}):
    url = urlparse (base_url)
    parameters = [{'location' : 'local_cluster', 'parameters' : []}]

    if url.scheme:
      if url.scheme == 's3' or url.scheme == 's3a':
        bucket_name = url.netloc
        from boto3 import resource
        bucket = resource ('s3').Bucket (bucket_name)
        input_files = [object_summary.key for object_summary in bucket.objects.filter (Prefix=path)]
        for file in input_files:
          config['filename'] = file
          parameters[0]['parameters'].append (str(config))
      else:
        raise Exception ('Unknown protocol');
    else:
      input_files = os.listdir (base_url + '/' + path)
      for file in input_files:
        config['filename'] = os.path.join (base_url, os.path.join (path, file))
        parameters[0]['parameters'].append (str(config))

    return parameters

  def show_progress_and_store_results (self, handle, output_directory):
    n_tasks = self.get_total_number_of_tasks (handle)
    result = self.results.pop (handle)
    n_finished_tasks = 0
    progress_bar_len = 50

    print('', end='\n')
    while result:
      n_finished_tasks += 1
      finished_portion = n_finished_tasks/n_tasks
      progress_bar_signs = int (round (progress_bar_len * finished_portion))
      print ('\r[{0}] | {1:>3}% | {2:>6} of {3:>6}' .format
               ( '='*progress_bar_signs + ' '*(progress_bar_len - progress_bar_signs)
               , int (round (finished_portion*100))
               , n_finished_tasks
               , n_tasks
               )
            , end=''
            , flush=True
            )

      self.results.store ([result], output_directory)
      result = self.results.pop (handle)
    print('\n')

  def get_progress_status (self, handle):
    n_total_tasks = self.get_total_number_of_tasks (handle)
    n_completed_tasks = n_total_tasks - self.get_number_of_remaining_tasks (handle)
    return n_completed_tasks, n_total_tasks

  class results:
    def __init__(self, pop_result, collect_results, status_info_rep, job_name):
      self._pop_result = pop_result
      self._collect_results = collect_results
      self._status_info_rep = status_info_rep
      self._job_name = job_name

    def pop (self, job):
      try:
        result = self._pop_result (job)
        if result and 'result' in result:
          result['result'] = dart_context.unpack (result['result'])

        self.write_monitoring_info (job, result)

        return result
      except:
        return {}

    def clear_measurements (self):
      url = urlparse (self._status_info_rep)
      if (url.scheme == 'http'):
        try:
          requests.post ( self._status_info_rep + '/query'
                        , data = { 'user_name' : 'admin'
                                 , 'password' : 'Password1234'
                                 , 'db': 'taskdb'
                                 , 'q' : 'delete from "tasksperformed"'
                                 }
                        )
        except requests.exceptions.RequestException as e:
          print ('Exception occurred when attempting to clear the measurements table:\n' + e)
          sys.exit (1)

      else:
        abs_path_file = self._status_info_rep + '/monitoring_info.txt'
        if os.path.exists (abs_path_file):
          os.remove (abs_path_file)

    def write_monitoring_info (self, job, result):
      if 'result' in result:
        status = 'Succeeded'
        log = ""
      else:
        status = 'Failed'
        log = result['error'].replace('"','\\"')

      url = urlparse (self._status_info_rep)

      if (url.scheme != 'http'):
        with open (self._status_info_rep + '/monitoring_info.txt', 'a') as f:
          f.write ('{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8} \n'.format
                    ( self._job_name, result['task_id'], result['location'], result['host']
                    , result['worker'], result['start_time'], result['duration']
                    , status, log
                    )
                  )
      else:
        params = (('db', 'taskdb'),)
        data = ( 'tasksperformed,job={0},taskid={1},location={2}'
               + ',host={3},workerid={4},status={5}'
               + ' starttime={6},taskduration={7},log="{8}"'
               ).format ( self._job_name, result['task_id'], result['location']
                        , result['host'], result['worker'], status
                        , result['start_time'], result['duration'], log
                        )

        try:
          requests.post (self._status_info_rep + '/write', params=params, data=data)
        except requests.exceptions.RequestException as e:
          print ('Exception occurred when attempting to update the measurements table:\n' + e)
          sys.exit (1)

    def extract (self, results):
      extracted = results
      for result in extracted:
        if 'result' in result:
          result['result'] = dart_context.unpack (result['result'])

      return extracted

    def collect (self, job):
      results = self.extract (self._collect_results (job))
      for result in results:
        self.write_monitoring_info (job, result)

      return results

    @staticmethod 
    def format (result):
      return ( 'task_id: {0}, worker: {1}, host: {2}, location: {3}'
             + ', start_time: {4}, duration: {5}, '
             ).format (result['task_id'], result['worker'], result['host'],
             result['location'], result['start_time'], result['duration'])

    @staticmethod
    def store (list_results, output_directory):
      f = open (output_directory + '/results.txt' , 'a')
      for dict in list_results :
        f.write (dart_context.results.format (dict))
        if 'error' not in dict.keys():
          f.write ('result: {0}\n'.format (dict['result']))
        else:
          f.write ('error: {0}\n'.format (dict['error']))

        f.write ('\n')

    @staticmethod
    def print (dict):
      print (dart_context.results.format (dict))
      if 'error' not in dict:
        print ('result: {0}\n'.format (dict['result']))
      else:
        print ('error: {0}\n'.format (dict['error']))