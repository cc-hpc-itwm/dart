import os
import sys
import socket
import time
sys.path.insert (0, '../python/')

import dart

# Get current directory of the current file
current_dir = os.path.dirname(os.path.realpath(__file__))

if len(sys.argv) < 2:
  print("Usage {0} server_address [capabilities]")
  sys.exit(0)

client = dart.client(sys.argv[1], '000') # Change client key ?

info = client.get_server_information()

agent_host = info["servers"][0]["agent"]["host"];
agent_port = info["servers"][0]["agent"]["port"];
agent_pid = "0"; # atm this is not needed, actually the whole name of the agent is ignored

worker_id = time.time()

master = "agent-{0} {1} {2}-0%{0}%{1}".format(agent_host, agent_port, agent_pid)

capabilities = ""
for i in range(2, len(sys.argv)):
  if capabilities != "":
    capabilities = capabilities + " "
  capabilities = capabilities + sys.argv[i]
 
name = "{0}-{1} 0 0-{2}".format(capabilities.replace(" ", "_"), socket.gethostname(), worker_id)

print("Trying to connect to master '{0}' as '{1}' with capabilities '{2}'".format(master, name, capabilities))

# These variables should be fixed
kernel = current_dir + "/../libexec/bundle/gpispace/libexec/gspc/drts-kernel"
backlog_length = "1"
library_dir = current_dir + "/../workflow/"

# The command to start the worker
cmd = kernel + " 1 --master \"" + master + "\" --library-search-path \"" + library_dir + "\" --capability \"" + capabilities + "\" -n \"" + name + "\" --backlog-length \"" + backlog_length + "\""

os.system("{0} > /var/tmp/{1}.txt 2>&1 &".format(cmd, name))