# -*- coding: utf-8 -*-
#!/usr/bin/python2

import re
import pandas as pd
import matplotlib.pyplot as plt
import sys
#reload(sys)
#sys.setdefaultencoding('utf8')

#maps_filename="maps_aie_thd_165.txt"
maps_filename="maps.txt"
#maps_filename="maps_init.txt"
maps_list=[]
maps_file = open(maps_filename, 'rb')
maps_columns = ["start", "end", "size(KB)", "filename", 'permission']
maps_process_end='80000000'

pre_end=0
for line in maps_file:
    #00008000-0000b000 r-xp 00000000 b3:01 1023       /root/pidmax
    #0000b000-0000c000 r--p 00002000 b3:01 1023       /root/pidmax
    #0000c000-0000d000 rw-p 00003000 b3:01 1023       /root/pidmax
    maps_line_fmt = '(?P<start>.{8})-(?P<end>.{8}) (?P<permission>.{4}) (?P<size>.{8}) (?P<major>.{2}):(?P<minor>.{2}) (?P<handle>[0-9]*) *(?P<filename>.*)'
    m = re.match(maps_line_fmt, line)
    if(not m):
        continue
    start = m.group('start')
    end = m.group('end')
    permission = m.group('permission')
    #size = m.group('size')
    #major = m.group('major')
    #minor = m.group('minor')
    #handle = m.group('handle')
    filename = m.group('filename')
    
    start_int = int(start, 16)
    end_int = int(end, 16)

    if(pre_end != start_int):
        maps_list.append([ "{:0>8x}".format(pre_end), start, (start_int - pre_end)/1024, 'NOT USED', 'unknown'])
    #print start+','+end+','+permission+','+filename
    #---p r--p rw-p r-xp rwxp rw-s
    if permission == '---p':
        permission = 'guard/thread stack'
    elif (permission == 'r--p'):
        permission = 'readonly var'
    elif (permission == 'rw-p'):
        permission = 'read/write var'
    elif (permission == 'r-xp'):
        permission = 'code'
    elif (permission == 'rwxp'):
        permission = 'heap/stack'
    elif (permission == 'rw-s'):
        permission = 'sharememory'
    else:
        permission = 'unkown'
    maps_list.append([start, end, (end_int - start_int)/1024, filename, permission])
    pre_end = end_int
maps_file.close()
maps_list.append([end, maps_process_end, (int(maps_process_end, 16) - end_int)/1024, 'NOT USED', 'unknown'])

maps_pd = pd.DataFrame(columns=maps_columns, data=maps_list)
maps_pd.to_csv("maps.csv", encoding='utf-8')
#print 'Total memory =', maps_pd['size(KB)'].sum()/1024,'(MB)'
print('Total memory = %s MB' % maps_pd['size(KB)'].sum()/1024)

rectangle_width = 800
maps_height_base = 40
maps_height_diff = 160
maps_size_min = maps_pd['size(KB)'].min()
maps_size_max = 16384
rectangle_x = 50
rectangle_y = 50

fig = plt.figure()

ax = fig.add_subplot(111)
for index, maps in maps_pd.iterrows():
    rectangle_height = (float)(min(maps_size_max, maps['size(KB)']) - maps_size_min)*maps_height_diff/maps_size_max + maps_height_base
    if maps['filename'] == 'NOT USED':
        color = 'red'
        text_color = 'white'
    else:
        color_value = (int)((float)(min(maps_size_max, maps['size(KB)']) - maps_size_min)/maps_size_max*0xffffff)
        color = '#%06x'%(color_value)
        text_color = '#%06x'%(0xffffff - color_value)
    if maps['size(KB)'] >= 1024:
        maps_label = "(%.2fMB)%s(%s)"%((float)(maps["size(KB)"])/1024, maps["filename"], maps['permission'])
    else:
        maps_label = "(%dKB)%s(%s)"%(maps["size(KB)"], maps["filename"], maps['permission'])
    #print rectangle_x, rectangle_y, rectangle_width, rectangle_height, maps['size(KB)'], color
    plt.bar(rectangle_x, rectangle_height, width=rectangle_width, bottom=rectangle_y, align='edge',facecolor=color, linewidth=1, edgecolor='red')
    plt.text(rectangle_x+10, rectangle_y+rectangle_height/2, maps_label, horizontalalignment='left', verticalalignment='center', color=text_color, fontsize=12)

    rectangle_y += rectangle_height

plt.xlim(0, rectangle_width+100)
plt.ylim(0, rectangle_y+100)
plt.axis('off')
    
#plt.show()
fig.set_size_inches(rectangle_width/100, rectangle_y/100)
fig.savefig('maps.svg', dpi=120, bbox_inches='tight', format='svg')
