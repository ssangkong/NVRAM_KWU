#!/usr/bin/env python

import os
import sys
import subprocess
import time
from collections import OrderedDict

def usage():
	print 'usage: automation.py [application]'

def revert():
	lst = open('launch_files')
	files = lst.read().splitlines()
	lst.close()
	print files
	for f in files:
		os.system('mv ' + os.path.dirname(f) + '/___' + os.path.basename(f) + ' ' + f)
		#os.system('sudo mv /mnt/ssd' + os.path.dirname(f) + '/___' + os.path.basename(f) + ' /mnt/ssd' + f)

def io_profiler(device, timeout=5):
	subprocess.Popen(['sync'], stdout=subprocess.PIPE).wait()
        f = open('/proc/sys/vm/drop_caches', 'w')
        f.write('3')
        f.close()
	return subprocess.Popen(['blktrace', '-d', device, '-o', '-', '-w', str(timeout)], stdout=subprocess.PIPE)

def systemcall_profiler(cmdline):
	return subprocess.Popen(['strace', '-fe', 'trace=file', '-o', 'st'] + cmdline)

def extract_block_sequence(process_io):
	blkparse = subprocess.Popen(['blkparse', '-i', '-'], stdin=process_io.stdout, stdout=subprocess.PIPE)
	grep = subprocess.Popen(['grep', 'Q   R'], stdin=blkparse.stdout, stdout=subprocess.PIPE)
	r = grep.stdout.read()
	r = [l.split()[7:10:2] for l in r.splitlines()]
	return r

def intersection_block_sequence(block_sequence):
	try:
		f = open('block_sequence', 'r')
	except:
		return block_sequence
	saved = [i.split() for i in f.readlines()]
	f.close()
	intersection = []
	rest = []
	for start, size in block_sequence:
		for start_saved, size_saved in saved:
			if int(start_saved) <= int(start) < int(start_saved) + int(size_saved):
				if int(start) + int(size) <= int(start_saved) + int(size_saved):
					intersection.append([start, size])
					break
				else:
					rest.append([str(int(start_saved) + int(size_saved) + 1), str(int(start) + int(size) - int(start_saved) - int(size_saved))])
					size = str(int(start_saved) + int(size_saved) - int(start))
					intersection.append([start, size])
					break
			elif int(start) <= int(start_saved) < int(start) + int(size):
				if int(start_saved) + int(size) < int(start) + int(size):
					intersection.append([start_saved, size_saved])
					break
				else:
					rest.append([str(int(start) + int(size) + 1), str(int(start_saved) + int(size_saved) - int(start) - int(size))])
					size_saved = str(int(start) + int(size) - int(start_saved))
					intersection.append([start_saved, size_saved])
					break
			else:
				continue
		
	if rest != []:
		print 'rest'
		print rest
		return intersection + intersection_block_sequence(rest)
		
	return intersection

def save_block_sequence(block_sequence):
	f = open('block_sequence', 'w')
	for block in block_sequence:
		f.write(block[0])
		f.write('\t')
		f.write(block[1])
		f.write('\n')
	f.close()

def is_valid_file(f):
	if f.startswith('/dev'):
		return False
	elif f.startswith('/proc'):
		return False
	elif f.startswith('/run'):
		return False
	elif f.startswith('/sys'):
		return False
	elif not os.path.exists(f):
		return False
	elif os.path.isdir(f):
		return False
	else:
		return True

def extract_launch_files(process_sys):
	st = open('st', 'r')
	grep = subprocess.Popen(['grep', '-Ev', 'ENOENT|---|resumed'], stdin=st, stdout=subprocess.PIPE)
	awk = subprocess.Popen(['awk', '-F"', '{print $2}'], stdin=grep.stdout, stdout=subprocess.PIPE)
	files = awk.stdout.read().splitlines()
	files = OrderedDict((item,None) for item in files).keys() #deduplicate
	#remove pseudo files
	files = [f for f in files if is_valid_file(f)]
	return files

def save_launch_files(launch_files):
	f = open('launch_files', 'w')
	for launch_file in launch_files:
		f.write(launch_file)
		f.write('\n')
	f.close()

def mkfs(device='/dev/ram0'):
	mkfs = subprocess.Popen(['mkfs.ext2', device], stdout=subprocess.PIPE)
	mkfs.wait()

def mount(device, mountpoint):
	subprocess.Popen(['mkdir', mountpoint]).wait()
	mnt = subprocess.Popen(['mount', device, mountpoint])
	mnt.wait()
	subprocess.Popen(['chmod', '777', mountpoint]).wait()
	
def umount(mountpoint):
	umnt = subprocess.Popen(['umount', mountpoint])
	umnt.wait()

def get_filename_offset(block, device='/dev/mmcblk0p2'):
        try:
            fdisk = subprocess.Popen(["fdisk", "-l", device[:-1]], stdout=subprocess.PIPE)
        except:
            fdisk = subprocess.Popen(["fdisk", "-l", device[:-2]], stdout=subprocess.PIPE)
        fdisk.wait()
        grep = subprocess.Popen(["grep", device], stdin=fdisk.stdout, stdout=subprocess.PIPE)
        grep.wait()
        disk_offset = int(grep.stdout.read().split()[-5])
	block = (int(block)-disk_offset) / 8
	icheck = subprocess.Popen(['debugfs', '-R', 'icheck ' + str(block), device], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	icheck.wait()
	inum = icheck.stdout.readlines()[1].split()[1]
	print block, inum,
	ncheck = subprocess.Popen(['debugfs', '-R', 'ncheck ' + inum, device], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	ncheck.wait()
	try:
		filename = ncheck.stdout.readlines()[1].split()[1]
		print filename
		if not os.path.isdir(filename):
			filefrag = subprocess.Popen(['debugfs', '-R', 'filefrag -v ' + filename, device], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			filefrag.wait()
			contents = filefrag.stdout.readlines()[3:]
			for content in contents:
				content = content.split()
				offset = block - int(content[2])
				if 0 <= offset <= int(content[-1]):
					offset = offset + int(content[1])
					return filename, offset
		return None, None
	except:
		print 'err! ', block
		return None, None
	

def get_filenames_offsets_sizes(block_sequence, device='/dev/mmcblk0p2'):
	filenames_offsets_sizes = []
	for block, size in block_sequence:
		filename, offset = get_filename_offset(block, device)
		if( filename != None ):
			filenames_offsets_sizes.append([filename, offset, size])
	return filenames_offsets_sizes
		
def move_to_nvram(launch_files, mount_point):
	for f in launch_files:
		print f
		os.system('mkdir -p ' + mount_point + os.path.dirname(f))
		os.system('cp -pP ' + f + ' ' + os.path.dirname(f) + '/___' + os.path.basename(f))
		os.system('cp -p ' + f + ' ' + mount_point + f)
		os.system('ln -sf ' + mount_point + f + ' ' + f)

def generate_prefetcher(filenames_offsets_sizes, sourcefile='prefetcher.py'):
	f = open(sourcefile, 'w')
	f.write("#!/usr/bin/env python\n")
	f.write("import ctypes\n")
	f.write("import ctypes.util\n")
	f.write("POSIX_FADV_WILLNEED=3\n")
	f.write("libc = ctypes.CDLL(ctypes.util.find_library('c'))\n")
        f.write("def posix_fadvise(fd, offset, len, advice):\n")
	f.write('\tlibc.posix_fadvise(fd, ctypes.c_uint64(offset), ctypes.c_uint64(len), advice)\n')
	f.write('\n')
	for filename, offset, size in filenames_offsets_sizes:
		f.write('f = open("' + filename + '")\n')
		f.write('posix_fadvise(f.fileno(), ' + str(int(offset)*4096) + ',' + str(int(size)*512) + ',POSIX_FADV_WILLNEED)\n')
		f.write('f.close()\n')
	f.close()
        os.chmod(sourcefile, 0777)

if __name__ == '__main__':
	if len(sys.argv) < 2:
		usage()
		exit()

	if sys.argv[1] == '--revert':
		revert()
		exit()

	command = sys.argv[1:]
	process_io = io_profiler('/dev/mmcblk0p2')
	time.sleep(1)
	process_sys = systemcall_profiler(command)
	process_sys.wait()
	block_sequence = extract_block_sequence(process_io)
	print 'length of extracted blk sequence = %d'%len(block_sequence)
	block_sequence = intersection_block_sequence(block_sequence)
	print 'length of intersection blk sequence = %d'%len(block_sequence)
	save_block_sequence(block_sequence)
	launch_files = extract_launch_files(process_sys)
	print '# of launch files = %d'%len(launch_files)
	save_launch_files(launch_files)
	umount('/ramdisk')
	mkfs('/dev/ram0')
	mount('/dev/ram0', '/ramdisk')
	filenames_offsets_sizes = get_filenames_offsets_sizes(block_sequence, '/dev/mmcblk0p2')

	move_to_nvram(launch_files, '/ramdisk')
	generate_prefetcher(filenames_offsets_sizes)
