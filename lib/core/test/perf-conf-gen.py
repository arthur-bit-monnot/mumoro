# A small configuration generator for perfomance test 


import random

max_node =  330000

def rand_node():
  return random.randrange(0, max_node)

def rand_time():
  return random.randrange(0, 60*60*24)

def rand_day():
  return random.randrange(1, 10)

nb_test = 10

for i in range(nb_test-1):
  print "{0} {1} {2} {3}".format(rand_node(), rand_node(), rand_time(), rand_day())
