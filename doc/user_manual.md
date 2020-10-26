## DART: Distributed Analytics Runtime 

DART is a paralleization framework that allows to:
  - easily integrate with Python applications
  - easily parallelize data analytics applications written in Python
  - efficiently exploit the existing HPC infrastructure, without caring about details    
    (e.g. scheduling, load balancing, memory management)
  - add or remove computing resources (workers) at runtime according to the application's needs 
  - to behave tolerantly to worker failures

Additionally, DART is suitable for the use in geo-distributed processing, 
being able to schedule and execute tasks at geographically distributed locations 
(in order to avoid unnecessary data transfers).

DART is built on top of GPI-Space, benefiting thus from an efficient C++ 
implementation that allows the parallel execution of workflow-driven
applications in distributed environments.

## DART Main Features

  - It can be imported as a module in the Python applications
  - Allows for easy integration into and parallelization of Python applications
  - Allows using arbitrary Python data structures for storing/transferring the results of distributed computations
  - Provides automatic transportation of backend errors to the users
  - Requires no installation of the Python applications/tasks
  - May scale up and down (elastic computing)
  - Fast C++ kernel (powered by GPI-Space)

## Configuration

Before using DART, one should either load the software module *dart*, if available,
(e.g. `module load /p/hpc/soft/etc/modules/soft/dart`) or define the environment
variable `DART_HOME`, which should contain the path to a valid DART installation.
Additionally, before carrying experiments with DART the `soft/anaconda3` module shoul

## Creating a dart_context object

Before using the DART API in a Python application, one should first import and create
a dart_context, as below: 

    from dart import dart_context
    dc = dart_context (pyhome, monurl)

where the first argument is the path to the _Python_ installation used and the second
is either a folder where logging information about the tasks is to be stored or the
url of the *influxdb* daemon used by the *Grafana* monitoring tool.
Additionally, several other parameters can be specified as keyworded arguments:
  - the path to the certificates folder (e.g. certificates_dir='/scratch/test/certs')
  - the logging host and port (.e.g. log_host=`host`, log_port=45091)
  - the GUI monitoring host and port og GPI-Space (e.g gui_host=`host`, gui_port=45090)

####  Examples:

  - Creating a simple DART context using the Grafana monitoring tool:
      `dc = dart_context (sys.exec_prefix, 'http://lts001hpc.itwm.fhg.de:8086')`
      
  - Creating a named context with grafana monitoring:      
      `dc = dart_context (sys.exec_prefix, 'http://lts001hpc.itwm.fhg.de:8086', name='count_words')`
  
  - Creating a simple DART context with monitoring information stored into a file:
      `dc = dart_context (sys.exec_prefix, '/var/tmp')`

  - Creating a named context with monitoring information stored into a file:      
      `dc = dart_context (sys.exec_prefix, '/var/tmp', name='count_words')`

## Starting DART

DART can be started either in a homogeneous setup, in which the same number of workers of the same type are started on the machines enumerated in a machine file, or in a heterogeneous setup in which the type and the number of workers to be started on different hosts are described in a worker description file.

Example 1: starting 10 workers on each of the machines emumerated in the file `/dart_workspace/machinefile`, where on each line a host name is specified, can be achieved with the following call

	dc.start ('/dart_workspace/machinefile', 10) 
	
Example 2: starting 5 workers of type "A" at the location "Frankfurt" on the hosts "host1.baz.de"
and  "host2.baz.de" and 10 workers of type "B" at the location "Dublin" on the host "host3.baz.ie"
can be achieved with the call `dc.start ('/dart_workspace/machinefile.json')`, where the worker description file  "machinefile.json" has the following structure: 

          {  "worker" : { "capabilities": ["A", "Frankfurt"]   
                        , "num_per_node": "5"
                        , "shm_size": "0"
                        , "hosts" : ["host1.baz.de", "host2.baz.de"]
                        },
             "worker" : { "capabilities": ["B", "Dublin"]
                        , "num_per_node": "10"
                        , "shm_size": "0"
                        , "hosts" :  ["host3.baz.ie"]
                        }
          }

## Specifying Task Parameters

For each location, a list of parameter sets should be defined. Each set of parameters in this list represent arguments that should be passed to the method executed by a task on an a worker. In the most general case this should be a list of dictionaries as below:
  
     parameters = [ {'location' : 'location0', 'parameters': [p00, p01, …]}
                  , {'location' : 'location1', 'parameters': [p10, p11, …]}
                  , ...
                  , {'location' : 'locationn', 'parameters': [pn0, pn1, …]}
                  ]

In the case when the parameters are files located at a given absolute path in the filesystem or relative 
to an url, one can use one of the predefined functions `prepare_parameters` as in the examples below:
   
      dc.prepare_parameters (‘/var/tmp‘)
      dc.prepare_parameters ('s3://my_buckets‘, fdm_releases, config)

where in the first example the files are store on a shared filesystem at the location `"/var/tmp"`
and in the second example the files are stored at the location `"s3://my_buckets/fdm_releases"`.
Additionally, one can also specify as an argument a dictionary (config) that contains user information necessary to execute tasks.

## Running Tasks

The task submission to DART can be realized in two ways: blocking and non-blocking.
In the first case the method run must be invoked. This method can be used to either execute methods from a module that is already installed in DART, in which case the module name can be omitted, as in the example below:

    results = dc.run (method, parameters, out_dir)

This call waits until all the tasks are computed (i.e. the method is executed) and returns the list of all task results which are stored in the specified output directory.

When the method is provided by the user itself in a specified module, one should also specify this module (with the absolute path), as in the example below:

    run (module, method, parameters, out_dir)

Alternatively, one may use non-blocking execution methods. These methods have similar signatures as the blocking methods, but with the exception that they don't wait for the tasks to finish, returning immediately a handle, that can be later used for requesting the results.  When the method is already installed in a module in DART the signature is as follows;      

    handle = dc.async_run (method, parameters, output_dir)
 
When the method is defined in a module provided by the user, this should be explicitly specified in the call (with absolute path), as follows:
            
    handle = dc.async_run (module, method, parameters, output_dir)

## Retrieving Results
For retrieving all the results of a bunch of tasks associated with a given handle, the  member function collect (from the results class) can be used, as in the example below:

    results = dc.results.collect  (handle)

For retrieving the the next available result, the results class method pop must be used, as in the example below:

    result = dc.results.pop (handle)  
    
## Storing Results
For storing a list of results in a given directory the results class `"store"` method must be used, as in the example below:

    dc.results.store ([result_0, result_1, ..., result_n], '/var/tmp')
    
## Printing Results
 A result can be printed to the standard output by invoking the class method print, as in the example below:

    dc.results.print (result)

## Status Reporting

The API method get_progress_status returns the number of tasks executed (x) and the total number of tasks submitted (y) corresponding to a given handle:

    x, y = dc.get_progress_status (handle)

Alternatively, by using the API method show_progress_and_store_results, a progress bar nis displayed, showing
the percentage of tasks already computed. The arguments have the same meaning as above:

    dc.show_progress_and_store_results   (handle, parameters, output_directory)

## Packing and Unpacking Results

 Assuming that dc is a dart_context Python object, the API provides two methods for serializing pack/unpack data must be:

    dc.pack (results)
    dc.unpack (results)

## Adding and Removing Workers

DART allows to add new workers at runtime by invoking the API function `add_workers` with a list of hosts where to spawn workers and a positive number specifying how many process workers per host to start. Example:
  
    dc.add_workers (['node115.beehive.itwm.fhg.de'], 5)

Additionally, the API provides a method that can be used for removing all workers started on a bunch of hosts,
as below:

    dc.remove_workers (['node115.beehive.itwm.fhg.de'])

## Logging and Monitoring

DART can produce monitoring information that ca be graphically represented offering to the user valuable insights about the workload distribution across workers and machines and detailed information about the errors occurred in either in the system or in the user Python code. For this purpose, DART was adapted to push information about task executions into a time series database (influx), which is used for creating meaningful views corresponding to different filters by the Grafana monitoring tool.
In order to be able to use Grafana, one should specify at the context creation time the url of the influxdb server as in example below:
        
 dc = dart_context (sys.exec_prefix, 'http://lts001hpc.itwm.fhg.de:8086')

Additionally, DART automatically produces log files for each worker, which are stored in the output folder
specified at the submission time. The logs contain information w.r.t. the errors occurred or user produced information.
In order to be able to use the Grafana monitoring tool with DART, before starting the experiments, the dartmonitor shoul be started using the command line `dartmonitor-start`. For this purpose the module `/p/hpc/soft/etc/modules/soft/dartmonitor.` For stopping the *dartmonitor* the following command line must be used: *dartmonitor-stop*.

## Clearing the Measurements Table
The table storing the measurements and implicitly the history of past runs can be cleaned with the command `clear_measurements`:

    dc.results.clear_measurements()

## Shutdown DART manually
DART takes care to stop all related processes at the end. However, if left-over related processes are still running at after the application finished (mostly because of an abnormal termination), the following pre-installed script should be used for terminating all processes:
  
    $DART_HOME/bin/dart_stop.sh <absolute_path_to_the_nodefile_used_at_startup>
    
## Building DART
Compiling DART requires a GPI-Space installation linked against the boost libraries, including boost::python.
(use the module /p/hpc/soft/etc/modules/lib/boost/1.61.0_py3 for this purpose).
Before compilling DART the `configure` script available at the top level in the source
tree must be run with the parameters:

 1. the path to the DART source tree 
 2. the path where to install DART
 3. the path to the GPI-Space installation used
 
## Use Cases and Examples

A couple of examples illustrating the use of the DART API are installed in the folder `$DART_HOME/example`.
The corresponding methods used in this examples are defined and installed in the module `$DART_HOME/lib/dart_task.py`. The examples count the words in a list of files that are stored at a given location in the filesystem and illustrate the use of different DART API methods.

#### Example:

    #This example counts the words in each text file existing in a given folder given as argument.
    #The execution is non-blocking. After submission of the parameters, the user is
    #given a handle that can be used for collecting all the results.
    #For each result monitoring informatio is pushed into an influx database
    #available at the url given as send parameter dart_context constructor
    
    import os
    import sys
    
    if (len (sys.argv)) < 2:
      raise Exception ("Please provide the folder containing the files to process!")
    
    sys.path.insert (0, os.environ['DART_HOME'] + '/lib')
    from dart import dart_context
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

#### Word-counting method to call in parallel, for different data samples:

    import sys
    import time
    
    sys.path.insert (0, '..\lib')
    from dart import dart_context as dc, catch_stdout, catch_stderr
    
    dc.catch_out()
    dc.catch_err()
      
    def count_words (_params):
      config = eval (_params)
      wordCounter = {}       
        
      with open (config['filename'], "r+") as file:
        for word in file.read().split():
          if word not in wordCounter:
            wordCounter[word] = 1
		  else:
            wordCounter[word] += 1
            
      return dc.pack (wordCounter)
          









