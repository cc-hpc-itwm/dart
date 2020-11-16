# 2020-11-16
## Added
  - Result Storage
    * Result Storage is used to store the results before they are retrieved by the python interface
	* Atm, there is only a RAM Storage
  - Python Interface
    * Added a is_result_available(jobid) function that checks (non-blocking) whether a new result is available
## Fixes
  - Removed orchestrator from ´FindGPISpace.cmake´ as orchestrator has been inlined in agent since GPSC 20.09
  - Fixed a bug where the ´python\*.so´ is not closed after usage