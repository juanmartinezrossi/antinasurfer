function [lnxPath] = lnx_install_dir
%LNX_INSTALL_DIR Returns the path to lnx.tlc
%
%       Author: Dan Bhanderi, 2004

[lnxPath, name, ext, versn] = fileparts(which('aloe.tlc'));

lnxPath = rtw_alt_pathname(lnxPath);

%end lnx_install_dir.m
