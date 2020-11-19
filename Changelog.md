# 2020-11-19
## Added
  - Result Storage
	* Python Storage can be used to pass a python object that handles the results
	    * To use a custom result storage, create a class that implements the following functions 
		    * add_job(self, job, expected_results)
			* remove_job(self, job)
			* get_number_of_expected_results(self, job)
			* get_number_of_received_results(self, job)
			* push(self, job, result)
			* get(self, job)
			* pop(self, job)
		* Create an instance of this class and pass that instance to the constructor of `dart_context`, i.e., `dart_context (sys.exec_prefix, result_storage=my_storage)`
        * See also the following example:
```python
class storage():
  _storage = {}
  
  def add_job(self, job, expected_results):
    print('Add a job : {}'.format(job))
    self._storage[job] = {}
    self._storage[job]["expected_results"] = expected_results
    self._storage[job]["received_results"] = 0
    self._storage[job]["results"] = []
  
  def remove_job(self, job):
    print('Remove job {}'.format(job))
    self._storage.pop(job)
  
  def get_number_of_expected_results(self, job):
    if not(job in self._storage):
      print("Wow expect 0")
      return 0
    v= self._storage[job]["expected_results"]
    print("Expected {}".format(v))
    return v
  
  def get_number_of_received_results(self, job):
    if not(job in self._storage):
      print("Wow receive 0")
      return 0
    v= self._storage[job]["received_results"]
    print("Received {}".format(v))
    return v
    
  def push(self, job, result):
    print('Push a result for job {}, {}'.format(job, result))
    print(self._storage[job])
    self._storage[job]["received_results"] += 1
    self._storage[job]["results"].append(result)
    print(self._storage[job])
    
  def get(self, job):
    print('Get a result for job {}'.format(job))
    print(self._storage[job])
    if not(job in self._storage):
      return None
    if len(self._storage[job]["results"]) == 0:
      return None
    return self._storage[job]["results"][0]
    
  def pop(self, job):
    print('Pop a result for job {}'.format(job))
    if not(job in self._storage):
      return
    print(self._storage[job])
    if len(self._storage[job]["results"]) == 0:
      return
    self._storage[job]["results"].pop(0)
    print(self._storage[job])
    
if (len (sys.argv)) < 2:
  raise Exception ("Please provide the folder containing the files to process!")

my_storage = storage()

dc = dart_context (sys.exec_prefix, result_storage=my_storage)
```
  - Python Interface
	* Added functionality to define a result storage, see above
## Fixes
  - Added an additional check in pop_result to ensure the result is valid

# 2020-11-16
## Added
  - Result Storage
    * Result Storage is used to store the results before they are retrieved by the python interface
	* Atm, there is only a RAM Storage
  - Python Interface
    * Added a is_result_available(jobid) function that checks (non-blocking) whether a new result is available
## Fixes
  - Removed orchestrator from ´FindGPISpace.cmake´ as orchestrator has been inlined in agent since GPSC 20.09