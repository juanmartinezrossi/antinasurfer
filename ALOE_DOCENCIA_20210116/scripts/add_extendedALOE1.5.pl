#!/usr/bin/perl

$modulename = $ARGV[0];
$author = $ARGV[1];

if ($modulename eq "") {
	print "Please, enter the module name: ";
	$modulename = <STDIN>;
	chomp($modulename);
}

if ($modulename eq "") {
	print "Need a module name.\n";
	exit;
}

if ($author eq "") {
	print "Enter the author's name (<Enter> for machine's username): ";
	$author = <STDIN>;
	chomp($author);
}

if ($author eq "") {
	$author = getlogin();
}

$module_template = "module_template_extended.ALOE1.5";
$makefile = "Makefile.am";
$base_subdirs = "utils/lnx_make hw_api/lnx sw_daemons/lnx_make cmdman_console sw_api/lnx_make modules/typetools/lnx_make ";
$config = "configure.in";
$base_config = "AC_CONFIG_FILES([Makefile\n\tutils/lnx_make/Makefile\n\thw_api/lnx/Makefile\n\tsw_daemons/lnx_make/Makefile\n\tsw_api/lnx_make/Makefile\n\tcmdman_console/Makefile\n\tmodules/typetools/lnx_make/Makefile\n\t";
$base_config_cnt = 6;

$modules_dir = "modules/";
$make_append = "lnx_make/";


if (-d "modules/".$modulename) {
	print "Module " . $modulename . " already exist.\n";
	exit;
}
print "Creating directory structure for module " . $modulename . "...\n";
system("cp -fr scripts/". $module_template . " modules/");
system("mv modules/". $module_template . " modules/".$modulename);
system("mv modules/".$modulename."/src/modulename.c modules/".$modulename."/src/".$modulename.".c");
system("mv modules/".$modulename."/doc/modulename.params modules/".$modulename."/doc/".$modulename.".params");

open(M,"modules/".$modulename."/lnx_make/Makefile.am");
@lines = <M>;
close(M);

$output="";
foreach $line (@lines) {
	$line =~ s/modulename/$modulename/g;
	$output .= $line;
}
open(M,">","modules/".$modulename."/lnx_make/Makefile.am");
print M $output;
close(M);

open(M,"modules/".$modulename."/src/".$modulename.".c");
@lines = <M>;
close(M);

$output="";
foreach $line (@lines) {
	$line =~ s/_modulename_/$modulename/g;
	$output .= $line;
}
$data = localtime();
$output="";
foreach $line (@lines) {
	$line =~ s/_date_/$data/g;
	$output .= $line;
}
$output="";
foreach $line (@lines) {
	$line =~ s/_user_/$author/g;
	$output .= $line;
}
open(M,">","modules/".$modulename."/src/".$modulename.".c");
print M $output;
close(M);

$i=0;

@modules = <modules/*>;

print "Adding the module to " . $makefile . " and " . $config . " files.\n";
foreach $module (@modules) {
	if ($module =~ m/typetools/) {
            $j=$i;
	}
	$i++;
}

splice(@modules,$j,1);

open(M,$makefile);
@lines = <M>;
close(M);

$output="";

$c=0;
foreach $line (@lines) {
	if ($line =~ m/^SUBDIRS/) {
		$output .= "SUBDIRS=" . $base_subdirs;
		foreach $module (@modules) {
			$output .= $module . "/" . $make_append . " ";
		}	
		$output .= "\n";
	} else {
		$output .= $line;
	}
}

open(M,">",$makefile);
print M $output;
close(M);

open(M,$config);
@lines = <M>;
close(M);

$output="";
$c=0;
foreach $line (@lines) {
	if ($line =~ m/^AC_CONFIG_FILES/ || ($c>0)) {
		if ($c==0) {
			$output .= $base_config;
			foreach $module (@modules) {
				$output .= "\t" . $module . "/" . $make_append . "Makefile\n";
			}		
			$output .= "\t])\n";
		}
		$c++;
		if ($line =~ m/\]\)/) {
			$c=0;
		}
	} else {
		$output .= $line;
	}
}
open(M,">",$config);
print M $output;
close(M);

print "Done.\n";



