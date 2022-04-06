# -*- coding: utf-8 -*-

#!/usr/bin/python3
import os,re
import sys
prefix = '''digraph spdk {
    graph [
        rankdir = "LR"
        //splines=polyline
        //overlap=false
    ];
 
    node [
        fontsize = "16"
        shape = "ellipse"\r
    ];
 
    edge [
    ];
'''
 
middle_str = ''
edge_list = []
edge_string = ''
cur_indentation_level = 0
space4 = '    '
space8 = space4 + space4
space12 = space4 + space8
space16 = space4 + space12
node_database = {}
struct_file = sys.argv[1]
#print("%s" %struct_file)
node_database['created'] = []
#color_arrary = ['red', 'green', 'blue', 'black','blueviolet','brown', 'cadetblue','chocolate','crimson','cyan','darkgrey','deeppink','darkred']
color_arrary = ['red', 'green', 'blue', 'black','blueviolet','brown', 'cadetblue','chocolate','crimson','cyan','grey','deeppink','pink']
#with open(r'./struct.log', 'r') as file_input:
with open(struct_file, 'r') as file_input:
    tmpline = file_input.readline()
    while(tmpline):
        tmpline = re.sub(r'([^a-zA-Z0-9]const )', ' ', tmpline)
        #for match :struct device {
        if re.search(r'struct\s*([0-9a-zA-Z_\-]+)\s*\{', tmpline):
            m = re.search(r'struct\s*([0-9a-zA-Z_\-]+)\s*\{', tmpline)
            cur_indentation_level += 1
            if (cur_indentation_level == 1):
                node_name =  m.group(1)
                node_str = space4 + '\"' + node_name + '\" [\n' + space8 + 'label = \"<head> '+ node_name +'\l|\n' + space12 + '{|{\n'
                node_database['created'].append(node_name)
                try:
                    node_database[node_name]['node_str'] = node_str
                except:
                    node_database[node_name] = {}
                    node_database[node_name]['node_str'] = node_str
        #for match :struct device        *parent;
        elif re.search(r'struct\s*([0-9a-zA-Z_\-]+)\s*(\**)(\s*)([0-9a-zA-Z_\-]+)\s*;', tmpline) and cur_indentation_level > 0:
            m = re.search(r'struct\s*([0-9a-zA-Z_\-]+)\s*(\**)(\s*)([0-9a-zA-Z_\-]+)\s*;', tmpline)
            member_type = m.group(1)
            node_database[node_name]['node_str'] += space16 + '<'+ member_type + '> ' + m.group(2) +  m.group(3) +  m.group(4) + '\l|\n'
            try:
                node_database[member_type]['included_by'].append(node_name)
            except:
                try:
                    node_database[member_type]['included_by'] = []
                    node_database[member_type]['included_by'].append(node_name)
                except:
                    node_database[member_type] = {}
                    node_database[member_type]['included_by'] = []
                    node_database[member_type]['included_by'].append(node_name)
            #print('%s included by %s'%(member_type, node_database[member_type]['included_by']))
            if(member_type in node_database['created']):
                tmp_edge_str = space4 + node_name + ':' + member_type + ' -> ' + member_type + ':' + 'head'
                if not tmp_edge_str in edge_list:
                    edge_list.append(tmp_edge_str)
        #for match : void *driver_data;
        elif re.search(r'\s*[0-9a-zA-Z_\-]+\s*(\**[0-9a-zA-Z_\-]+)\s*;', tmpline) and cur_indentation_level > 0:
            m = re.search(r'\s*[0-9a-zA-Z_\-]+\s*(\**[0-9a-zA-Z_\-]+)\s*;', tmpline)
            node_database[node_name]['node_str'] += space16 + '<'+ m.group(1) + '> ' + m.group(1) +  '\l|\n'
        #for match:const char        *init_name;
        elif re.search(r'(.*)\s+(\**)(\s*)([0-9a-zA-Z_\-]+\s*);', tmpline) and cur_indentation_level > 0:
            m = re.search(r'(.*)\s+(\**)(\s*)([0-9a-zA-Z_\-]+\s*);', tmpline)
            node_database[node_name]['node_str'] += space16 + '<'+ m.group(2) + '> ' + m.group(2) + m.group(3) + m.group(4) +  '\l|\n'
        #for match:int *(*runtime_idle)(struct device *dev);
        elif re.search(r'\s*[0-9a-zA-Z_\-]+\s*\**\s*\(\s*(\**\s*[0-9a-zA-Z_\-]+)\s*\)\s*\([^\)]*\)\s*;', tmpline) and cur_indentation_level > 0:
            m = re.search(r'\s*[0-9a-zA-Z_\-]+\s*\**\s*\(\s*(\**\s*[0-9a-zA-Z_\-]+)\s*\)\s*\([^\)]*\)\s*;', tmpline)
            node_database[node_name]['node_str'] += space16 + '<'+ m.group(1) + '> (' + m.group(1) + ')\l|\n'
        #for match: };
        elif re.search(r'\s*\}\s*;', tmpline):
            if(cur_indentation_level >= 1):
                cur_indentation_level -= 1
                if (cur_indentation_level == 0):
                    node_database[node_name]['node_str'] += space12 + '}}\"\n'
                    node_database[node_name]['node_str'] += space8 + 'shape = \"record\"\n' + space4 + '];\n'
                    if 'included_by' in node_database[node_name]:
                        for parent_node in node_database[node_name]['included_by']:
                            if parent_node in node_database['created']:
                                tmp_edge_str = space4 + parent_node + ':' + node_name + ' -> ' + node_name + ':' + 'head'
                                if not tmp_edge_str in edge_list:
                                    edge_list.append(tmp_edge_str)
        tmpline = file_input.readline()
 
for tmpnode in node_database['created']:
    middle_str = middle_str + node_database[tmpnode]['node_str']
for i, tmpstr in enumerate(edge_list):
    edge_string += tmpstr + '[color="' + color_arrary[i%len(color_arrary)] + '"]\n'
 
print(prefix + middle_str + '\n' + edge_string + '}') 
