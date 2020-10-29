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

  time.sleep (0.5)
  return dc.pack (wordCounter)
          
def test (params):

  exec (params, globals(), globals())
  result = x * y
  
  return dc.pack (result)
