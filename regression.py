#!/usr/bin/env python
import os, sys

tests=('49:0:16383:1 50:0:16383:2 49:0:16383:4 50:0:16383:8 49:0:16383:16 50:0:16383:32 49:0:16383:64 50:0:16383:128', '-r 8')

data=os.popen('ls -1 ../../testFiles').read()
dataFiles = data.split('\n')
dataFiles.remove('')
for test in tests:
    for file in dataFiles:
        print 'Processing data file', file
        buildNew = './SMARechunker -i ../../testFiles/'+file+' -o new '+test
        print buildNew
        status = os.system(buildNew)
        if status != 65280:
            buildOld = '/sma/bin/SMARechunker -i ../../testFiles/'+file+' -o old '+test
            print buildOld
            os.system(buildOld)
            fileString = os.popen('ls -1 old').read()
            files=fileString.split('\n')
            files.remove('')
            for file in files:
                o='old/'+file
                n='new/'+file
                command = 'md5sum '+n+' '+o
                print 'checking', file
                md5s = os.popen(command).read()
                md5sCheck = md5s.split()
                if md5sCheck[0] != md5sCheck[2]:
                    print 'md5 check failure on', file
                    sys.exit(-1)
        else:
            print 'Skipping', file, 'which appears to have no SWARM data'
        os.system('rm -r -f old new')
print '\nREGRESSION TESTS PASSED!'
