## TODO List

# Important
  - Save results on worker before sending them to the server
    
    If the worker loses connection to the server, this will fail and the worker will crash (gpispace does not exit gently). If we save the result on the worker, we can - after the worker restarts and gets scheduled a new job - check if the new job is equal to the saved job result and if so, send this result.

  - Add Authentification
    
    The server already calls an `is_authorized` function (see `src/dart-server/dart_server.cpp`). The missing ingredient is the key storage and key lifetimes. A simple solution would be to add the keys via cmd line arguments while starting the server.

  - Unit Tests
# Feature
  - Allow different install locations for dart on the workers
    
    This is actually a missing gpispace feature

  - Save job results on the server side on a persistent storage
    
    Simple Filesystem Storage or Database. The code infrastructure supports this: `src/dart-server/job_storage.hpp`

  - Unix Domain Sockets instead of "real" sockets
    
    This allows for a local mode that is contained inside the server

  - Server Topology

    In principle the code supports talking to different servers, however this is untested and unimplemented. For more details, see `src/dart-server/dart_server.cpp` and `broadcast` function.
# Misc
  - Better error messages when something failes on the server

  - Do not hardcode the location to the dart files
    
    See `src/workflow/load_worker_config.hpp`, we assume that the gpispace infrastructure is persistent. Probably use cmake to resolve this.
