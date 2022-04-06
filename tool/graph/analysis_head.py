# -*- coding: utf-8 -*-
# 用于分析c语言头分拣的包含关系

#!/usr/bin/python3
import os,re
prefix = '''digraph spdk {
    graph [
        rankdir = "LR"
        //splines=polyline
        overlap=false
    ];
 
    node [
        fontsize = "16"
        shape = "ellipse"\r
    ];
 
    edge [
    ];
'''
 
def get_head_file_list(path_file):
    head_file_list = []
    with open(path_file, 'r') as file_input:
        tmpline = file_input.readline()
        while (tmpline):
            #to match #include < XXX/YYY.h >
            m = re.search(r'#include\s*[<\"]\s*(.*/)([0-9a-zA-Z_\-]*)\.[Hh]\s*[>\"]', tmpline)
            if m:
                head_file_list.append(re.sub(r'\-', '_', m.group(2)))
 
            #to match #include < XXX.h >
            elif re.search(r'#include\s*[<\"]\s*([0-9a-zA-Z_\-]*)\.[Hh]\s*[>\"]', tmpline):
                m = re.search(r'#include\s*[<\"]\s*([0-9a-zA-Z_\-]*)\.[Hh]\s*[>\"]', tmpline)
                head_file_list.append(re.sub(r'\-', '_', m.group(1)))
            tmpline = file_input.readline()
    return head_file_list
 
def build_node_from_file(file_path, file_name, edges, included_by):
    i = 0
    space4 = '    '
    space8 = space4 + space4
    space12 = space4 + space8
    space16 = space4 + space12
    file_name_wo_h = re.search(r'([0-9a-zA-Z_\-]*)\.h', file_name).group(1)
    file_name_wo_h = re.sub(r'\-', '_',file_name_wo_h)
    #print(file_name_wo_h)
    node_str = space4 + '\"' + file_name_wo_h + '\" [\n' + space8 + 'label = \"<head> '+ file_name_wo_h +'.h\l|\n' + space12 + '{|{\n'
    headfilelist = ["aaa", "bbb"] #fake file list
    headfilelist2 = get_head_file_list(os.path.join(file_path, file_name))
    #print('headfilelist2:')
    #print(headfilelist2)
    for headfile in headfilelist2:
        i += 1
        try:
            included_by[headfile].append(file_name)
        except:
            included_by[headfile] = []
            included_by[headfile].append(file_name)
        node_str = node_str + space16 + '<'+ headfile + '> ' + headfile +  '.h\l|\n'
        tmp_edge_str = space4 + file_name_wo_h + ':' + headfile + ' -> ' + headfile + ':' + 'head' #+ '[color="' + color_arrary[i%len(color_arrary)] + '"]\n'
        try:
            if not tmp_edge_str in edges[headfile]:
                edges[headfile].append(tmp_edge_str)
        except:
            edges[headfile] = []
            edges[headfile].append(tmp_edge_str)
    node_str = node_str + space12 + '}}\"\n'
    node_str = node_str + space8 + 'shape = \"record\"\n' + space4 + '];\n'
    #print(included_by)
    return {'node_str':node_str,'edges':edges}
 
edges = {}
included_by = {}
node_created = []
middle_str = ''
edge_string = ''
color_arrary = ['red', 'green', 'blue', 'black','blueviolet','brown', 'cadetblue','chocolate','crimson','cyan','grey','deeppink','pink']
 
for maindir, subdir, file_name_list in os.walk(r'G:\git_repository\linux-stable\linux-4.18\drivers\net\wireless\broadcom'):#('G:\git_repository\linux-stable\linux-4.18\drivers\usb'):
    for tmpfile in file_name_list:
        if re.match(r'.*\.h', tmpfile):
            result = build_node_from_file(maindir, tmpfile, edges, included_by)
            node_created.append(re.search(r'([0-9a-zA-Z_\-]*)\.h', tmpfile).group(1))
            middle_str = middle_str + '\n' + result['node_str']
            edges = result['edges']
##print(filelist2)
for tmpfile in edges:
    if tmpfile in node_created:
        for i,tmpstr in enumerate(edges[tmpfile]):
            edge_string += tmpstr + '[color="' + color_arrary[i%len(color_arrary)] + '"]\n'
print(prefix + middle_str + '\n' + edge_string + '}')
