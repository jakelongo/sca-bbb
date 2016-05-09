import socket
import binascii
import unittest
import random

targetObject   = None
returnVariable = None

class neonInterface(object):

  def __init__(self):
    self.port      = 5000
    self.session   = None
    self.sockFd    = None

  def connect(self, hostname):
    if (self.session != None):
      self.session.close()

    self.session = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.session.connect((hostname, self.port))

    self.sockFd = self.session.makefile()
    self.sockFd.flush()

  def disconnect(self):
    self.sockFd.write('x')

    self.sockFd.close()
    self.session.close()

    self.sockFd   = None
    self.session  = None

  def write(self, data):
    if (self.sockFd != None):
      self.sockFd.write(data)

  def read(self, len):
    if (self.sockFd != None):
      ret = self.sockFd.read(len)
      return ret

  def flush(self):
    if (self.sockFd != None):
      self.sockFd.flush()

def neonOpen(args):
  global targetObject

  if (targetObject != None):
    print "Target already open!"
    return True

  targetObject = neonInterface()
  splitArgs    = args.split(' ')

  # Check if we've assigned a port
  # as well as ip address
  if (len(splitArgs) == 2):
    targetObject.port = int(splitArgs[1])

  try:
    targetObject.connect(splitArgs[0])

  # If it failed to open, ret false
  except Exception, e:
    targetObject = None
    return False

  # opened, let's do this!
  return True

def neonClose(args):
  global targetObject

  if (targetObject == None):
    print "Target already closed!"
    return True

  targetObject.disconnect()
  targetObject = None

  return True

def neonSet(args):
  # Expecting:
  #    arg[0] to be memory bank (0-2)
  #    arg[1] to be register content
  global targetObject

  if (targetObject == None):
    print "Target not open!"
    return False

  splitArgs = args.split(' ')

  # write register function
  targetObject.write('w')

  # Set register bank
  targetObject.write(chr(int(splitArgs[0])))

  # Send payload length
  targetObject.write(binascii.unhexlify("%08X" % (len(splitArgs[1])/2))[::-1])

  # send payload data
  targetObject.write(binascii.unhexlify(splitArgs[1]))

  # send EO transmission character
  targetObject.write('\n')

  # Flush the socket
  targetObject.flush()

  return True

def neonGet(args):
  # Expecting:
  #     arg[0] to be memory bank (0-2)
  #     arg[1] to be destination variable
  global targetObject
  global returnVariable

  if (targetObject == None):
    print "Target not open!"
    return False

  splitArgs = args.split(' ')

  # write register function
  targetObject.write('r')

  # Set register bank
  targetObject.write(chr(int(splitArgs[0])))
  targetObject.flush()

  # get payload length
  dataLen = targetObject.read(4)[::-1]
  dataLen = int(binascii.hexlify(dataLen), 16)

  # send payload data
  dataPayload = binascii.hexlify(targetObject.read(dataLen))

  # send EO transmission character
  targetObject.read(1)

  # Flush the socket
  targetObject.flush()

  returnVariable = dataPayload.upper()
  return True

def neonOp(args):
  global targetObject

  if (targetObject == None):
    print "Target not open!"
    return False

  targetObject.write('o')

  # Send payload length
  targetObject.write(binascii.unhexlify("%08X" % (len(args) + 1))[::-1])

  # set the string
  targetObject.write(args + r'\0')

  #terminate
  targetObject.flush()

  return True

def neonExec(args):
  global targetObject

  if (targetObject == None):
    print "Target not open!"
    return False

  targetObject.write('e')
  targetObject.flush()

  return True

def vec2str(vec):
  wordLen = 64/len(vec)
  vecstrs = [(format(x, 'X')).zfill(wordLen/4) for x in vec]
  return ''.join(vecstrs)

def add_vector(bits):
  a = [random.getrandbits(bits) for i in range(64/bits)]
  b = [random.getrandbits(bits) for i in range(64/bits)]
  c = [(x + y) & ((1<<bits)-1) for x, y in zip(a, b)]
  return [vec2str(x) for x in [a, b, c]]


class test_neon(unittest.TestCase):

  def test_openAndClose(self):
    ret = neonOpen('10.70.25.143 8081')
    ret = ret and neonClose('')
    self.assertTrue(ret)

  def test_neonwrite(self):
    ret = neonOpen('10.70.25.143 8081')
    ret = ret and neonSet('1 AABBCCDDEEFF0011')
    ret = ret and neonClose('')
    self.assertTrue(ret)

  def test_neonread(self):
    global returnVariable
    ret = neonOpen('10.70.25.143 8081')
    ret = ret and neonSet('3 DEADBEEFDEADC0DE')
    ret = ret and neonGet('3')
    ret = ret and neonClose('')
    self.assertEqual(returnVariable, 'DEADBEEFDEADC0DE')

  def test_neonexec(self):
    global returnVariable
    ret = neonOpen('10.70.25.143 8081')
    ret = ret and neonSet('3 DEADBEEFDEADC0DE')
    ret = ret and neonGet('3')
    ret = ret and neonClose('')
    self.assertEqual(returnVariable, 'DEADBEEFDEADC0DE')

if __name__ == '__main__':
  # unittest.main(verbosity=2)
  a = add_vector(32)
  print a


