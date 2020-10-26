import sys

class catch_stderr:
  def __init__(self):
    self.value = ''
  def write(self, txt):
    self.value += txt
        
catch_stderr = catch_stderr()
sys.stderr = catch_stderr
 
def syntax_error (params):
  for i in ['a', 'b', 'c']
    print (i)