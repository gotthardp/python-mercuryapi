#!/bin/bash

# Usage: ./install_ltkc.sh <path_to_root_dir> <install_dir(optional)>

#Script to extract LTKC code to build directory and apply required patches
ROOT_DIR=$1
SOURCE_DIR=${SOURCE_DIR:-"${ROOT_DIR}/vendor/llrp.org"}
FILE=LTKC_1_0_0_5.tar.gz
PATCH_DIR=${PATCH_DIR:-"${ROOT_DIR}/tm/modules/mos/MercuryOSLinux"}
XML_DIR=${XML_DIR:-"${ROOT_DIR}/tm/modules/tmmpd/common"}
if [ $# -gt 1 ]
then
  INSTALL_DIR=$2
else
  INSTALL_DIR=common
fi

if [ -d ${INSTALL_DIR}/LTK ]; then
        echo "${INSTALL_DIR}/LTK directory already exists"
        exit 0
fi

if [ ! -f ${SOURCE_DIR}/${FILE} ]; then
        echo "${SOURCE_DIR}/${FILE} not present"
        exit 1
fi

echo "Extracting ${FILE} to ${INSTALL_DIR}/ directory"

tar xzf ${SOURCE_DIR}/${FILE} -C ${INSTALL_DIR}/
rm ${INSTALL_DIR}/LTK/LTKC/Library/LLRP.org/llrporg_ltkc_genout.c
rm ${INSTALL_DIR}/LTK/LTKC/Library/LLRP.org/llrporg_ltkc.h
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_cross_compilation.patch
cp ${XML_DIR}/llrp-control-def.xml ${INSTALL_DIR}/LTK/Definitions/LLRP.org/
#This patch adds LLRP custom extensions xml file and modifies Makefiles to build these custom extensions
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltk_custom_extensions.patch
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_tcp_error_parsing.patch
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_custom_library_cross_compile.patch
#Apply patch that fixes typecast warnings (for Werror)
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_fix_typecast_warnings.patch
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_fix_xml_warnings.patch
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_max_custom_messages_size.patch
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_shared_lib.patch
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_add_custom_access_command_opspec.patch
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_add_loopSpec_ROSpec.patch
patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltk_shared_libs.patch
#Apply patch that provides specific error messages related to read function. This doesnt cover any additional functionality.
#patch -p0 -d ${INSTALL_DIR} < ${PATCH_DIR}/llrp_ltkc_read_specific_errors.patch

#This is to avoid multiple includes of out_ltkc_ header files.
echo '#ifndef __OUT_LTKC_WRAPPER_H' > ${INSTALL_DIR}/LTK/LTKC/Library/out_ltkc_wrapper.h
echo '#define __OUT_LTKC_WRAPPER_H' >> ${INSTALL_DIR}/LTK/LTKC/Library/out_ltkc_wrapper.h
echo '#include "out_ltkc.h"' >> ${INSTALL_DIR}/LTK/LTKC/Library/out_ltkc_wrapper.h
echo '#endif /* __OUT_LTKC_WRAPPER_H  */' >> ${INSTALL_DIR}/LTK/LTKC/Library/out_ltkc_wrapper.h

echo '#ifndef __OUT_TM_LTKC_WRAPPER_H' > ${INSTALL_DIR}/LTK/LTKC/Library/LLRP.org/out_tm_ltkc_wrapper.h
echo '#define __OUT_TM_LTKC_WRAPPER_H' >> ${INSTALL_DIR}/LTK/LTKC/Library/LLRP.org/out_tm_ltkc_wrapper.h
echo '#include "out_tm_ltkc.h"' >> ${INSTALL_DIR}/LTK/LTKC/Library/LLRP.org/out_tm_ltkc_wrapper.h
echo '#endif /* __OUT_TM_LTKC_WRAPPER_H  */' >> ${INSTALL_DIR}/LTK/LTKC/Library/LLRP.org/out_tm_ltkc_wrapper.h

echo "Done.."
