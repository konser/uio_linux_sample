#!/bin/sh

# dep: yum install graphviz 
# $1: struct file 
# $2: output graph
# signed-off-by Liming Wu <liming.wu@jaguarmicro.com>

if [[ $# -lt 2 ]]; then
	echo "ERROR: usage example: ./getgraph.sh struct_file output_graph" 
	exit 1
fi

INPUT=$1
GRAPH=$2
python analysis_struct.py $INPUT > tmpfile

dot -Tsvg tmpfile -o $GRAPH.svg
dot -Tpng tmpfile -o $GRAPH.png
