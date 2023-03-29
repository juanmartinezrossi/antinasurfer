function lnx_callback(hDlg, hSrc, paramName)
%LNX_CALLBACK Callback function for Linux Soft Real-Time Target Options.
%
%       Author: Dan Bhanderi, 2005

checkVal = strcmp('on',slConfigUIGetVal(hDlg, hSrc, paramName));

% Disable/Enable
if checkVal == 1
  slConfigUISetEnabled(hDlg, hSrc, 'MatlabPath', true);
  slConfigUISetEnabled(hDlg, hSrc, 'TargetPath', true);
else
  slConfigUISetEnabled(hDlg, hSrc, 'MatlabPath', false);
  slConfigUISetEnabled(hDlg, hSrc, 'TargetPath', false);
end

%end lnx_callback.m
