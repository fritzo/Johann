#LogFile.py
#version: 2003_04_20

from time import time
from sys import exit

class LogFile:
  def __init__(self,title = 'Computation'):
    self.title = title
    self.stack = ''
    self.indentSymbol = '  '
    self.log = ''
    self.startTime = time()
    self.printLevel = 256

  def setTitle(self,title):
    self.title = title
    
  def setPrintLevel(self,printLevel):
    self.printLevel = printLevel

  def write(self,label=None):
    if label is not None:
      message = ('['+str(time()-self.startTime)[:6]+']'
        +self.stack+str(label))
      if len(self.stack) <= (len(self.indentSymbol)*self.printLevel):
        print message
      self.log += message+'\n'

  def indent(self):
    self.stack += self.indentSymbol

  def outdent(self,indent='  '):
    self.stack = self.stack[:-len(self.indentSymbol)]

  def beginSection(self,label=None):
    if label is not None:
      self.write(str(label))
    self.indent()

  def endSection(self,label=None):
    self.outdent()
    if label is not None:
      self.write(str(label))

  def save(self,fileName = None,extraInfo=None):
    if fileName is None:
      fileName = self.title+'.log'
    file = open(fileName,'wc')
    file.write(
      '===================[ Begin '+self.title+' Log ]===================\n')
    file.write(self.log)
    file.write(
      '====================[ End '+self.title+' Log ]====================\n\n')
    if extraInfo is not None:
      file.write(extraInfo)
    file.close()

  def loggedExit(self,errorMessage):
    self.beginSection('<--- error encountered here:')
    self.stack = ''
    self.write(' '+errorMessage)
    self.write('[ logged exit ]--------------------')
    self.save('error.log')
    exit(errorMessage)
    
  def __del__(self):
    if len(self.log)>0:
      self.save()

global logFile
logFile = LogFile()
