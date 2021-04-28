# Readme
## Description
Dart can be used to distribute computional loads on workers that are in different geographic locations. It uses a REST API to communicate with the client which defines these computional loads. A computional load is a python script that should get executed on one or multiple workers.
##### Table of Contents
1) [Building DART](#build)
2) [Running a first example](#example1)
3) [The REST interface](#rest)
4) [gpispace patches](#patches)

<a name="#build"/>
## Building DART
We use docker images to create more or less portable binaries. Moreover, the images will setup all required libraries such as gpispace, GPI-2, ...

1) Clone this repository an cd in it

2) Create a build environment image with `docker build -t dart_buildenv_x86_64 -f docker/Dockerfile-x86_64 docker`
   * This is going to need a long time, however this step is only required once
  
3) Build dart inside the container
   * Start a container with some name, e.g., `build`
   * Copy dart to the container, i.e., `docker cp . build:/build/dart/`
   * Build dart, install it, and zip it, i.e., `docker exec build bash /build/dart/docker/build_dart.sh`
   * Copy the builded archive out of the container, i.e., `docker cp build:dart.tar.gz dart_x86_64.tar.gz`

<a name="#example1"/>
## How to use DART

1) Start `./bin/dart-server.exe`, the server is by default started at `127.0.0.1:7777` (you can check all cmd-line options using `--help`).
   * Make sure that you have a ssh service started!

2) In the `example` directory, start `python3 add_workers.py` to add workers to the servers.
   * The workers are started on the local machine
3) In the `example` directory, start `python3 count_words.py` to run the count_words example.
   * Note that you should change the filenames from `/home/luca/test/...` to some files on your drive whose words should get counted.
   * Note that the workers from step 2. will execute the `count_words` function from the `count_words_worker` script as specified by the `client.add_job` call.



<a name="#rest"/>
## The REST interface

All REST API calls require a JSON request body and a valid authentication key. For the exact structure of the requests, please refer to `./python/dart.py`.

| Resource | Function | Description |
| -------- | -------  | ----------- |
| /server/ | DELETE   | Stop the dart servers. |
| /server/ | GET      | Return information about the server. |
| |
| /worker/ | POST | Add workers to gpispace (on remote hosts) |
| /worker/ | DELETE | Delete workers from gpispace (on remote hosts) |
| /worker/ | GET | Ret# Readme
## Description
Dart can be used to distribute computional loads on workers that are in different geographic locations. It uses a REST API to communicate with the client which defines these computional loads. A computional load is a python script that should get executed on one or multiple workers.
##### Table of Contents
1) [Building DART](#build)
2) [Running a first example](#example1)
3) [The REST interface](#rest)
4) [gpispace patches](#patches)

<a name="#build"/>
## Building DART
We use docker images to create more or less portable binaries. Moreover, the images will setup all required libraries such as gpispace, GPI-2, ...

1) Clone this repository an cd in it

2) Create a build environment image with `docker build -t dart_buildenv_x86_64 -f docker/Dockerfile-x86_64 docker`
   * This is going to need a long time, however this step is only required once
  
3) Build dart inside the container
   * Start a container with some name, e.g., `build`
   * Copy dart to the container, i.e., `docker cp . build:/build/dart/`
   * Build dart, install it, and zip it, i.e., `docker exec build bash /build/dart/docker/build_dart.sh`
   * Copy the builded archive out of the container, i.e., `docker cp build:dart.tar.gz dart_x86_64.tar.gz`

<a name="#example1"/>
## How to use DART

1) Start `./bin/dart-server.exe`, the server is by default started at `127.0.0.1:7777` (you can check all cmd-line options using `--help`).
   * Make sure that you have a ssh service started!

2) In the `example` directory, start `python3 add_workers.py` to add workers to the servers.
   * The workers are started on the local machine
3) In the `example` directory, start `python3 count_words.py` to run the count_words example.
   * Note that you should change the filenames from `/home/luca/test/...` to some files on your drive whose words should get counted.
   * Note that the workers from step 2. will execute the `count_words` function from the `count_words_worker` script as specified by the `client.add_job` call.
   * By default the logs get written to `/var/tmp/`, see also `./worker/worker.json`


<a name="#rest"/>
## The REST interface

All REST API calls require a JSON request body and a valid authentication key. For the exact structure of the requests, please refer to `./python/dart.py`.

| Resource | Function | Description |
| -------- | -------  | ----------- |
| /server/ | DELETE   | Stop the dart servers. |
| /server/ | GET      | Return information about the server. |
| |
| /worker/ | POST | Add workers to gpispace (on remote hosts) |
| /worker/ | DELETE | Delete workers from gpispace (on remote hosts) |
| /worker/ | GET | Return all currently connected workers |
| |
| /job/ | POST | Define a new job |
| /job/{job_id}/ | GET | Return information about the job with `job_id` | 
| /job/{job_id}/ | DELETE | Stop the job with `job_id` |  
| /job/{job_id}/status/ | GET | Return the state of the job with `job_id` | 
| /job/{job_id}/tasks/ | POST | Start tasks for the job with `job_id` | 
| /job/{job_id}/results/ | GET | Return results for the job with `job_id` |
| /job/{job_id}/results/{result_id}/ | DELETE | Delete the result with `result_id` of job with `job_id` |


<a name="#patches"/>
## gpispace patches
We require a patched version of gpispace 20.12 for dart. There are two patches that need to be applied, see `./docker/`

* `gspc-20.12-capabilities.patch`: Adds an event to fetch the capabilities that the server has atm. As we store the worker name as a capability, this allows us to check the currently connected workers.
* `gspc-20.12-portable-archive.patch`: Replaces the default non-portable archive that gpispace uses for communication by a portable one. This is necessary for communication between systems with different architecture (for instance x86_64 <-> armhf)
