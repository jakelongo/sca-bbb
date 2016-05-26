# Author: Jake Longo
# Note: this is adapted from a version designed
# to be part of the DPA workflow so may not
# make complete sense

import logging
import socket
import binascii
import unittest
import random
import ctypes
import sys

import aesLib


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

def neon_open(args):
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

def neon_close(args):
  global targetObject

  if (targetObject == None):
    print "Target already closed!"
    return True

  targetObject.disconnect()
  targetObject = None

  return True

def neon_ct(args):
  global targetObject
  global returnVariable

  if (targetObject == None):
    print "Target not open!"
    return False

  # write register function
  targetObject.write('r')
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

def neon_pt(args):
  global targetObject

  if (targetObject == None):
    print "Target not open!"
    return False

  targetObject.write('m')

  # Send payload length
  targetObject.write(binascii.unhexlify("%08X" % (len(args)/2))[::-1])

  # set the string
  targetObject.write(binascii.unhexlify(args))

  # send EO transmission character
  targetObject.write('\n')

  #terminate
  targetObject.flush()

  return True


def neon_iv(args):
  global targetObject

  if (targetObject == None):
    print "Target not open!"
    return False

  targetObject.write('i')

  # Send payload length
  targetObject.write(binascii.unhexlify("%08X" % (len(args)/2))[::-1])

  # set the string
  targetObject.write(binascii.unhexlify(args))

  # send EO transmission character
  targetObject.write('\n')

  #terminate
  targetObject.flush()

  return True

def neon_key(args):
  global targetObject

  if (targetObject == None):
    print "Target not open!"
    return False

  targetObject.write('k')

  # Send payload length
  targetObject.write(binascii.unhexlify("%08X" % (len(args)/2))[::-1])

  # set the string
  targetObject.write(binascii.unhexlify(args))

  # send EO transmission character
  targetObject.write('\n')

  #terminate
  targetObject.flush()

  return True


def neon_encrypt(args):
  global targetObject

  if (targetObject == None):
    print "Target not open!"
    return False

  targetObject.write('e')
  targetObject.flush()

  return True

class test_neon(unittest.TestCase):


  def test_openAndClose(self):
    ret = neon_open(hostname + ' ' + hostport)
    ret = ret and neon_close('')
    self.assertTrue(ret)

  def test_neonkey(self):
    ret = neon_open(hostname + ' ' + hostport)
    ret = ret and neon_key('DEADBEEFDEADBEEFDEADBEEFDEADBEEF')
    ret = ret and neon_close('')
    self.assertTrue(ret)

  def test_neoniv(self):
    ret = neon_open(hostname + ' ' + hostport)
    ret = ret and neon_iv('DEADBEEFDEADBEEFDEADBEEFDEADBEEF')
    ret = ret and neon_close('')
    self.assertTrue(ret)

  def test_neonpt(self):
    ret = neon_open(hostname + ' ' + hostport)
    ret = ret and neon_pt('DEADBEEFDEADBEEFDEADBEEFDEADBEEF')
    ret = ret and neon_close('')
    self.assertTrue(ret)

  def test_neonread(self):
    global returnVariable
    ret = neon_open(hostname + ' ' + hostport)
    ret = ret and neon_ct('')
    ret = ret and neon_close('')
    self.assertTrue(ret)

  def test_neonexec(self):
    global returnVariable
    ret = neon_open(hostname + ' ' + hostport)
    ret = ret and neon_pt('DEADBEEFDEADBEEFDEADBEEFDEADBEEF')
    ret = ret and neon_key('DEADC0DEDEADC0DEDEADC0DEDEADC0DE')
    ret = ret and neon_iv('00000000000000000000000000000000')
    ret = ret and neon_encrypt('')
    ret = ret and neon_ct('')
    ret = ret and neon_close('')

    pt  = aesLib.str2state('DEADBEEFDEADBEEFDEADBEEFDEADBEEF')
    key = aesLib.str2state('DEADC0DEDEADC0DEDEADC0DEDEADC0DE')
    ct  = aesLib.AESencrypt(key, pt)

    ctStr = aesLib.state2str(ct)

    self.assertEqual(returnVariable, ctStr)

  def test_neonexecBuffer(self):
    global returnVariable
    ret = neon_open(hostname + ' ' + hostport)
    ret = ret and neon_pt('DEADBEEFDEADBEEFDEADBEEFDEADBEEF')
    ret = ret and neon_key('DEADC0DEDEADC0DEDEADC0DEDEADC0DE')
    ret = ret and neon_iv('00000000000000000000000000000000')
    ret = ret and neon_encrypt('')
    ret = ret and neon_ct('')
    ret = ret and neon_close('')

    pt  = aesLib.str2state('DEADBEEFDEADBEEFDEADBEEFDEADBEEF')
    key = aesLib.str2state('DEADC0DEDEADC0DEDEADC0DEDEADC0DE')
    ct  = aesLib.AESencrypt(key, pt)

    ctStr = aesLib.state2str(ct)

    self.assertEqual(returnVariable, ctStr)


if __name__ == '__main__':
  unittest.main(verbosity=2)


