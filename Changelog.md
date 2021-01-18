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

