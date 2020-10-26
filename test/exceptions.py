import sys

class catch_stderr:
  def __init__(self):
    self.value = ''
  def write(self, txt):
    self.value += txt
        
catch_stderr = catch_stderr()
sys.stderr = catch_stderr

def module_not_found (params):
  import abcd
 
def division_by_zero (params):
  return 5/0
  
def file_not_found (params):
  f = open ('/sddq', 'r')
  