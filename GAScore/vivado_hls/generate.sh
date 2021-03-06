#!/usr/bin/env bash

if [[ "$#" != 1 ]]; then
    echo "Syntax: script moduleName"
    exit 1
fi

old_path=$PWD
file=$1

basePath=$SHOAL_PATH/GAScore/vivado_hls
projectPath=$basePath/projects/$SHOAL_HLS_VERSION
solutionPath=$projectPath/$file/$SHOAL_PART_FAMILY
ipPath=$solutionPath/impl/ip
repoPath=$SHOAL_PATH/GAScore/repo/$SHOAL_VIVADO_VERSION/$SHOAL_PART_FAMILY/$file

prefixedName=${file}_${file}
finalName=$file

mkdir -p $projectPath
cd $projectPath
vivado_hls -f $basePath/generate.tcl $file
vivado_return=$?
if [[ $vivado_return != 0 ]]; then
    exit $vivado_return
fi
# cp $SHOAL_PATH/GAScore/src/$file.cpp $solutionPath
cat $solutionPath/syn/report/${finalName}_csynth.rpt
mkdir -p $repoPath
rm -rf $repoPath/*
sed -i "s/\b$prefixedName\b/${file}/g" $ipPath/run_ippack.tcl
sed -i "s/set DisplayName \"${file^}_${finalName}\"/set DisplayName \"${finalName}\"/g" \
    $ipPath/run_ippack.tcl
sed -i "s/\b$prefixedName\b/${finalName}/g" \
    $ipPath/hdl/vhdl/${prefixedName}.vhd
sed -i "s/\b$prefixedName\b/${finalName}/g" \
    $ipPath/hdl/verilog/${prefixedName}.v
sed -i "s/\b$prefixedName\b/${finalName}/g" \
    $ipPath/bd/bd.tcl
mv $ipPath/hdl/vhdl/${prefixedName}.vhd \
    $ipPath/hdl/vhdl/${finalName}.vhd
mv $ipPath/hdl/verilog/${prefixedName}.v \
    $ipPath/hdl/verilog/${finalName}.v
cd $ipPath 
./pack.sh
zipFile=xilinx_com_hls_${file}_1_0.zip
cp $ipPath/$zipFile $repoPath
cd $repoPath
unzip -o $zipFile -d .
rm $zipFile
cd $old_path

# call IP specific script if it exists
if [[ -f "$basePath/$file.sh" ]]; then
    source $basePath/$file.sh
fi
