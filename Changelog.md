# 2021-03-01
## Fixed
   - Fixed a bug while reading http packages
## Added
   - Added `unbundle.sh` script which checks if there are 'newer' system .so's installed (by checking GLIBC and GLIBCXX) strings. This is necessary as sometimes the .so's that are loaded by python require newer libc versions. This feature is experimental.

# 2021-02-11
## Added
   - Patch for gpispace to get capabilities
   - Added cmd-line argument to specify hostname
## Changed
   - Updated Readme
   - Workername is now required
   - Renamed some cmd-line parameters
   - The start_worker script now also expects a name
## API Changes
   - add_worker now needs a name

# 2021-02-11
## Added
   - Patch for gpispace to make communication portable
## Changed
   - armhf patch improved
   - better error capturing
## Fixes
   - worker/host fields were mixed up
# 2021-02-03
## Added
   - Experimental Dockerfile for building an armhf image
   - Dockerimages do not require qt no longer
   - Added some command line options for dart-server
   - Auto redirect stdout and stderr from Python to the log file
   - Script to start worker manually
## Removed
   - `name` entry in `worker.json`

## API Changes
   - `job.config` has been reworked
   - `result` now contains an additional field names `host`
## Fixes
   - Fixed `GET /server/` API Call
   - Error Handling for `add_worker` and `remove_worker` in `gspc_interface`
   - Python does not anymore gets cleaned up as this seems to make problems with numpy

# 2021-01-18
## Added
   - `get results` API call now accepts an additional parameter `worker_regex` that specifies a regular expression that the worker name of the results must match
   - experimental Dockerfile for building an image that is able to build dart for aarch64
      * we do not require a full build on arm but only the worker and therefore the image should become much leaner
## API Changes
   - `get results` accepts the optional `worker_regex` parameter
## Fixes
   - `dart.py` correct conversion of return value in `get_job_status`
# 2021-01-10
## Added
   - Config file that set up the `python_home`, `output_directory`, `module_prefix`, ... for the worker
## API Changes
   - Removed the requirement to specify `python_home`, `output_directory`, when adding a new job
## Other
   - Errors occuring while executing a function on the worker are now properly captured
# 2020-12-17
## Added
  - Initial commit of dart server

