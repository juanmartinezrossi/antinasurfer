function make_lnx(varargin)
% MAKE_LNX Executes the Real-Time Workshop Build procedure for a block diagram.
%
%       MAKE_LNX first invokes the Target Language Compiler to generate the
%       code and then invokes the language specific make procedure.
%
%       Calls rtw_<target_language>, tlc_<target_language>
%
%       Author: Dan Bhanderi, 2005

%  Note: This is a UDD class version of make_rtw.

%  initiate an object of RTW.makertw class
h = RTW.makertw;

%  call make_rtw method of RTW.makertw class
%  Note: varargin must be kept untouched.
h.make_rtw(varargin{:});
