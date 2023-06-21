#!/usr/bin/env python2
import os
import threading
from time import sleep,ctime
import sys
import string

class ThreadFunc(object): 
    def __init__(self,func,args,name=''):
        self.name=name
	self.func=func
	self.args=args
    def __call__(self):
	apply(self.func,self.args)

    
def single_command(com_name,file_name):
    os.system('tcsh -c "%s %s"'%(com_name,file_name))

def main():
	
    if(len(sys.argv)<2):
       print "usage  : para_run.py <proc>   <command>       <filenames>"
       print "exmaple: para_run.py   4      mrc_shrink2_single.com    micrograph_1001.mrc micrograph_1003.mrc micrograph_1008.mrc micrograph_1009.mrc"
       print "exmaple: para_run.py   4      mrc_shrink2_single.com    micrograph_100*.mrc"
       print "exmaple: para_run.py   8      mrc_shrink2_single.com    micrograph_1???.mrc"
       print "The command can be any program/command/scripts, if it runs likes this 'command <one file>'. in the case of mrc_shrink2_single.com"
	       
       print "For example the case of mrc_shrink2_single.com. You would normally run it like 'mrc_shrink2_single.com micrograph_1001.mrc' or 'mrc_shrink2_single.com micrograph_1002.mrc' or 'mrc_shrink2_single.com micrograph_1003.mrc' ... "
       print "Now if you run 'para_run.py 8 mrc_shrink2_single.com micrograph_1???.mrc' on 1000 micrographs, they will be splited into 125 groups, eacho of them will run in parallel with 8 threads."

       print "This program is a multi-threads program, it calls a single command and does batch process."
       sys.exit()
       
       
    proc=int(sys.argv[1])
    command=sys.argv[2]
    allname=sys.argv[3:]
    lallname=len(allname)
    
    if(lallname<proc):
       print"      the image files must be more  than proc"
       sys.exit()

    threads=[]

    for a in range(0,lallname+1-proc,proc): 
	print a,'start'
	nprocs=range(a,a+proc)
        for i in nprocs:#creat all threads
	    t = threading.Thread(target=ThreadFunc(single_command,(command,allname[i]),single_command.__name__))
	    threads.append(t)
	    
        for i in nprocs:#start all threads
	    threads[i].start()
        for i in nprocs:#wait for completion
	    threads[i].join()
	print a,'end'
	
    if(a+proc<lallname):
        nprocs=range(a+proc,lallname)
        print a+proc,'start'
	for i in nprocs:#creat all threads
	    t = threading.Thread(target=ThreadFunc(single_command,(command,allname[i]),single_command.__name__))
	    threads.append(t)
        for i in nprocs:#start all threads
	    threads[i].start()
        for i in nprocs:#wait for completion
	    threads[i].join()
        print a+proc,'end'
    
#    os.system('tcsh -c "singlefpctffind32.com"')
    

if __name__=='__main__':
    main()
