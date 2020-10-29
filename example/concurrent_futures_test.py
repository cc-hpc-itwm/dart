import argparse
import os
import sys

class FullPaths(argparse.Action):
  def __call__(self, parser, namespace, values, option_string=None):
    setattr(namespace, self.dest, os.path.abspath(os.path.expanduser(values)))

def is_dir(dirname):
  if not os.path.isdir(dirname):
    msg = "{0} is not a directory".format(dirname)
    raise argparse.ArgumentTypeError(msg)
  else:
    return dirname
        
def is_file(filename):
  if not os.path.isfile(filename):
    msg = "{0} is not a file".format(filename)
    raise argparse.ArgumentTypeError(msg)
  else:
    return filename        

parser = argparse.ArgumentParser (description='Command line arguments for starting the GPI-Space runtime')
parser.add_argument('--anaconda_home', help='anaconda home', type=is_dir, action=FullPaths, required=True)
parser.add_argument('--worker_description_file', help='worker description file', type=is_file, action=FullPaths, required=True)
parser.add_argument('--certificates_directory', help='certificates directory', type=is_dir, action=FullPaths)
parser.add_argument('--input_folder', help='input folder containing the files to analyze', type=is_dir, action=FullPaths)
parser.add_argument('--method', help='python method to execute on worker side', type=str, required=True)

args = parser.parse_args()

print ("Application installation directory: {0}".format (args.anaconda_home))
print ("Worker description file: {0}".format (args.worker_description_file))
print ("Certificates directory: {0}".format (args.certificates_directory))
print ("Method: {0}".format (args.method))

from dart import dart_context 

dc = dart_context (args.anaconda_home, args.certificates_directory)

dc.start (args.worker_description_file)

from concurrent.futures import ProcessPoolExecutor, as_completed
future_list = []
               
parameter_list = [ 'x=1\ny=5'
                 , 'x=3\ny=7'
                 , 'x=2\ny=9'
                 , 'x=10\ny=6'
                 , 'x=12\ny=2'
                 , 'x=20\ny=3'
                 ]

def wait (handle):
  return dc.results.collect (handle)
                           
with ProcessPoolExecutor (max_workers=5) as executor:
  for s in parameter_list:
    handle = dc.async_run ("test", [{'location':'local_cluster', 'parameters': [s]}])
    future_list.append (executor.submit (wait, handle))

  for f in as_completed (future_list):
    list_results = f.result()
    dc.results.store (list_results, '/var/tmp') 
 
dc.stop()      
