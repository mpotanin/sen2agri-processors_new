import argparse
import os
import sys
import re
import shutil
import time
import datetime
import array
import subprocess
import errno

def wait_for(max_proc, proc_in_work):
    while len(proc_in_work) >= max_proc :
        for proc in proc_in_work:
            if (proc.poll()!=None) : 
                proc_in_work.remove(proc)
                return
        time.sleep(1)
    return
                

parser = argparse.ArgumentParser(description='Converts rasters of L2A from jpeg2000 to GeoTiff')

parser.add_argument('-i', required=True, metavar='input folder', help='Input folder with L2A products')
parser.add_argument('-o', required=False, metavar='output folder', help='Output folder if specified all files are copied to')
parser.add_argument('-t', required=False, metavar='tiles filter', help='List of tiles comma separated')
parser.add_argument('-sd', required=False, type=int, default=0, metavar='start yyyymmdd',help='Start date yyyymmdd')
parser.add_argument('-ed', required=False, type=int, default=30000000, metavar='end yyyymmdd', help='End date yyyymmdd')
parser.add_argument('-proc', required=False, type=int, metavar='processes num', help='num of parallel processes',default=1)

if (len(sys.argv)==1):
    parser.print_usage()
    exit(0)

args = parser.parse_args()

tiles_filt = None
if args.t is not None:
    tiles_filt = args.t.upper().split(',')
    
max_proc = args.proc
proc_in_work = list()

for d in os.listdir(args.i):
    if re.match("S2[A-D]_MSIL2A_.+_T([A-Z0-9]+)_.+", d, re.IGNORECASE):
        img_date = int(d[11:19])
        if (img_date <args.sd) or (img_date > args.ed): continue
            
        if tiles_filt is not None:
            if tiles_filt.count(d[39:44]) == 0: continue
                        
        print(d)
        attempt = 0
        attempt_max = 2
        raster_count = 0
        while (attempt < attempt_max):
            raster_count = 0
            for (dirpath, dirnames, filenames) in os.walk(os.path.join(args.i,d)):
                new_dirpath = dirpath if args.o is None else dirpath.replace(args.i,args.o)
                
                #create dirs and copy files if args.o defined
                if args.o is not None:
                    for d2 in dirnames :
                        if not os.path.exists(os.path.join(new_dirpath,d2)):
                            os.makedirs(os.path.join(new_dirpath,d2))
                    for f in filenames:
                        if ((not f.endswith('.jp2')) and (not f.endswith('.tif'))): 
                            try:
                                shutil.copy(os.path.join(dirpath,f), 
                                        os.path.join(new_dirpath,f))
                            except IOError as err:
                                time.sleep(1)
                                os.system('cp ' + os.path.join(dirpath,f) + ' ' + os.path.join(new_dirpath,f))
                                
                                                  
                #convert jp2 -> geotiff
                for f in filenames:
                    if f.endswith('.jp2'):
                        raster_count+=1
                        wait_for(max_proc,proc_in_work)
                        command = ('gdal_translate -of GTiff ' + os.path.join(dirpath,f) + ' ' + 
                                  os.path.join(new_dirpath,f).replace('.jp2','.tif'))
                        proc_in_work.append(subprocess.Popen(command,shell=True,stdout=subprocess.DEVNULL))
                wait_for(1,proc_in_work)
           
            if raster_count == 40 :
                print ('DONE')
                break
            else:
                attempt+=1
                
                
        if (attempt == attempt_max):
            print ('ERROR: found only ' + str(raster_count) + '/40' + ' rasters')
                
                          
        



