import logging
import socket
import binascii
import unittest
import random
import ctypes
import sys

targetObject   = None
returnVariable = None
hostname       = '10.70.25.143'
hostport       = '8081'

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
  targetObject.write(binascii.unhexlify("%08X" % (len(args)))[::-1])

  # set the string
  targetObject.write(args)

  # send EO transmission character
  targetObject.write('\n')

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

# the receiver is agnostic of word sizes so this converts to the
# correct memory format to be imported by the assembly calls
def vec2payload(vec):
  wordLen = 64/len(vec)
  byteLen = wordLen/8
  charLen = byteLen*2
  vecstrs = [(format(x, 'X')).zfill(charLen) for x in vec]
  payload = ''.join([''.join(reversed([x[i:i+2] for i in range(0, len(x), 2)])) for x in vecstrs])
  return payload

def addSigned(a,b):
  ia = ctypes.c_int32(a)
  ib = ctypes.c_int32(b)
  ic = ctypes.c_int32(ia.value+ib.value)
  return (ctypes.c_uint32(ic.value)).value

def bits2signed(a,bits):
  return (((a)+ 2**(bits-1)) % 2**bits - 2**(bits-1)) & (2**bits-1)

def add_vector(bits):
  a = [random.getrandbits(bits) for i in range(64/bits)]
  b = [random.getrandbits(bits) for i in range(64/bits)]
  c = [bits2signed(x+y,bits) for x, y in zip(a, b)]

  strs = [vec2str(x)     for x in [a, b, c]]
  payl = [vec2payload(x) for x in [a, b, c]]

  return (strs, payl)

def sub_vector(bits):
  a = [random.getrandbits(bits) for i in range(64/bits)]
  b = [random.getrandbits(bits) for i in range(64/bits)]
  c = [bits2signed(x-y,bits) for x, y in zip(a, b)]

  strs = [vec2str(x)     for x in [a, b, c]]
  payl = [vec2payload(x) for x in [a, b, c]]

  return (strs, payl)



vmuls = ['i8', 'i16', 'i32']
vadds = ['i8', 'i16', 'i32', 'i64']
vsubs = ['i8', 'i16', 'i32', 'i64']
veors = ['u64']
vands = ['u64']

class test_neon(unittest.TestCase):


  def test_openAndClose(self):
    ret = neonOpen(hostname + ' ' + hostport)
    ret = ret and neonClose('')
    self.assertTrue(ret)

  def test_neonwrite(self):
    ret = neonOpen(hostname + ' ' + hostport)
    ret = ret and neonSet('1 AABBCCDDEEFF0011')
    ret = ret and neonClose('')
    self.assertTrue(ret)

  def test_neonread(self):
    global returnVariable
    ret = neonOpen(hostname + ' ' + hostport)
    ret = ret and neonSet('3 DEADBEEFDEADC0DE')
    ret = ret and neonGet('3')
    ret = ret and neonClose('')
    self.assertEqual(returnVariable, 'DEADBEEFDEADC0DE')

  def test_neonsetop(self):
    global returnVariable
    ret = neonOpen(hostname + ' ' + hostport)
    ret = ret and neonOp('vmuli32')
    ret = ret and neonClose('')
    self.assertTrue(ret)

  def test_neonexec(self):
    global returnVariable
    (vecstrs, vecpay) = add_vector(32)

    ret = neonOpen(hostname + ' ' + hostport)

    ret = ret and neonSet('3 ' + vecpay[0])
    ret = ret and neonSet('4 ' + vecpay[1])
    ret = ret and neonOp('vaddi32')
    ret = ret and neonExec('')
    ret = ret and neonGet('2')
    ret = ret and neonClose('')

    self.assertEqual(returnVariable, vecpay[2])

  def test_fillmem(self):
    global returnVariable
    ret = neonOpen(hostname + ' ' + hostport)

    # fill the memory
    for i in xrange(8):
      ret = ret and neonSet(str(i) + ' DEADC0DE' + (format(i, 'X')).zfill(8))

    # read the memory
    for i in xrange(8):
      ret = ret and neonGet(str(i))
      self.assertEqual(returnVariable, 'DEADC0DE' + (format(i, 'X')).zfill(8))

    ret = ret and neonClose('')
    self.assertTrue(ret)


  def test_neon_vadd(self):
    global returnVariable
    ret = neonOpen(hostname + ' ' + hostport)

    for testIdx in xrange(50):

      width = random.randint(0, len(vadds)-1)
      neonOp('vadd' + vadds[width])

      (vecstrs, vecpay) = add_vector(int((vadds[width])[1:]))

      neonSet('3 ' + vecpay[0])
      neonSet('4 ' + vecpay[1])

      neonExec('')
      neonGet('2')

      self.assertEqual(returnVariable, vecpay[2])

    neonClose('')

  def test_neon_vsub(self):
    global returnVariable
    ret = neonOpen(hostname + ' ' + hostport)

    for testIdx in xrange(50):

      width = random.randint(0, len(vsubs)-1)
      neonOp('vsub' + vsubs[width])

      (vecstrs, vecpay) = sub_vector(int((vsubs[width])[1:]))

      neonSet('3 ' + vecpay[0])
      neonSet('4 ' + vecpay[1])

      neonExec('')
      neonGet('2')

      self.assertEqual(returnVariable, vecpay[2])

    neonClose('')

  def test_neon_vmul(self):
    global returnVariable
    ret = neonOpen(hostname + ' ' + hostport)

    for testIdx in xrange(50):

      width = random.randint(0, len(vsubs)-1)
      neonOp('vsub' + vsubs[width])

      (vecstrs, vecpay) = sub_vector(int((vsubs[width])[1:]))

      neonSet('3 ' + vecpay[0])
      neonSet('4 ' + vecpay[1])

      neonExec('')
      neonGet('2')

      self.assertEqual(returnVariable, vecpay[2])

    neonClose('')

if __name__ == '__main__':
  unittest.main(verbosity=2)
  # vecs = add_vector(32)
  # vecs = sub_vector(16)
  # print vecs


