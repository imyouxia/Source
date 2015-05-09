#!/usr/bin/env perl
use Getopt::Long;


sub usage();
sub genClassHeader();
sub genClassFile();
sub genTestHeader();
sub genTestFile();
sub genFileDescription($);

my $now = `date +\%Y-\%m-\%d\\ \%H:\%M:\%S`;
chomp $now;

my $defaultnamespace = "anet";
my $defaultrootdir = `pwd`;
chomp $defaultrootdir;
my $defaultsrcfiledir = "src/anet";
my $defaulttestfiledir = "test";
my($rootdir, $srcfiledir, $testfiledir, $classname, $namespace);
my($genClassHeader, $genClassCPP, $genTestHeader, $genTestCPP) = (0,0,0,0);
my($forceClassHeader, $forceClassCPP, $forceTestHeader, $forceTestCPP, $forceall) 
    = (0,0,0,0,0);
my ($includepath);
if ( !GetOptions ("srcfiledir=s" => \$srcfiledir, 
		  "testfiledir=s" => \$testfiledir,
		  "rootdir=s" => \$rootdir,
		  "classname=s" => \$classname,
		  "namespace=s" => \$namespace,
		  "genclassheader" => \$genClassHeader,
		  "genclasscpp" => \$genClassCPP,
		  "gentestheader" => \$genTestHeader,
		  "gentestcpp" => \$genTestCPP,
		  "forceclassheader" => \$forceClassHeader,
		  "forceclasscpp" => \$forceClassCPP,
		  "forcetestheader" => \$forceTestHeader,
		  "forcetestcpp" => \$forceTestCPP,
		  "forceall" => \$forceall,
		  "includepath" => \$includepath,
))
{
    usage();
}
    
usage() unless ($classname);
$rootdir = $defaultrootdir unless ($rootdir);
$srcfiledir = $defaultsrcfiledir unless ($srcfiledir);
$testfiledir = $defaulttestfiledir unless ($testfiledir);
$namespace = $defaultnamespace unless ($namespace);
($genClassHeader, $genClassCPP, $genTestHeader, $genTestCPP) = (1,1,1,1)
    unless ($genClassHeader || $genClassCPP || $genTestHeader || $genTestCPP);
($forceClassHeader, $forceClassCPP, $forceTestHeader, $forceTestCPP) = (1,1,1,1) 
    if ($forceall);
my $testclassname = "$classname"."TF";
my $srcdir = "$rootdir/$srcfiledir";
my $includedir = "$namespace" unless $includepath;
my $testdir = "$rootdir/$testfiledir";
my $classcpp = "$srcdir/".lc($classname).".cpp";
my $classheader = "$srcdir/".lc($classname).".h";
my $testcpp = "$testdir/".lc($testclassname).".cpp";
my $testheader = "$testdir/".lc($testclassname).".h";
my $classheaderguard=uc("${namespace}_${classname}_h_");
my $testheaderguard=uc("${namespace}_${testclassname}_h_");
print "rootdir: $rootdir\n";
print "srcdir: $srcdir\n";
print "classcpp: $classcpp\n";
print "classheader: $classheader\n";
print "testcpp: $testcpp\n";
print "testheader: $testheader\n";
print "classheaderguard: $classheaderguard\n";
print "testheaderguard: $testheaderguard\n";

genClassHeader();
genClassFile();
genTestHeader();
genTestFile();

sub usage() {
    print <<__EOF__;
$0 [--rootdir=/path/to/root/dir] [--srcfiledir=relevant/path/from/rootdir/to/src/file/dir] 
              [--testfiledir=relevant/path/from/rootdir/to/test/file/dir] [--namespace namespace]
	      [--includepath = default/include/path]
	      --classname=classname 
	      [--genclassheader] [--genclasscpp]
	      [--gentestheader] [--gentestcpp]
	      [--forceclassheader] [--forceclasscpp] 
	      [--forcetestheader] [--forcetestcpp] 
	      [--forceall]
__EOF__
    exit(1);
}

sub genClassHeader() {
    return unless $genClassHeader;
    if (-e $classheader) {
	print STDERR "WARN!!! $classheader already exist!\n";
	if (!$forceClassHeader) {
	    print STDERR "add --forceclassheader to override it!\n";
	    return;
	}
	print STDERR "Will override it!\n"
    }
    print "Generating $classheader\n";
    my $rc = open FH, ">$classheader" ;
    if (!$rc) {
	print STDERR "Failed to open file $classheader to write:$!\n";
	return;
    }
    
    print FH genFileHeader(lc($classname).".h");
    print FH "#ifndef $classheaderguard\n";
    print FH "#define $classheaderguard\n";
    print FH "//*****add include headers here...\n";
    print FH "namespace $namespace {\n";
    print FH "class $classname\n";
    print FH "{\n";
    print FH "public:\n";
    print FH "    $classname();\n";
    print FH "};\n\n";
    print FH "}/*end namespace $namespace*/\n";
    print FH "#endif /*$classheaderguard*/\n";
}

sub genClassFile() {
    return unless $genClassCPP;
    if (-e $classcpp) {
	print STDERR "WARN!!! $classcpp already exist!\n";
	if (!$forceClassCPP) {
	    print STDERR "add --forceclasscpp to override it!\n";
	    return;
	}
	print STDERR "Will override it!\n"
    }
    print "Generating $classcpp\n";
    my $rc = open FH, ">$classcpp" ;
    if (!$rc) {
	print STDERR "Failed to open file $classcpp to write:$!\n";
	return;
    }
    print FH genFileHeader(lc($classname).".cpp");
    print FH "#include <$includedir/".lc($classname).".h>\n\n";
    print FH "namespace $namespace {\n";
    print FH "${classname}::${classname}() {\n";
    print FH "}\n\n";
    print FH "}/*end namespace $namespace*/\n";
}

sub genTestHeader() {
    return unless $genTestHeader;
    if (-e $testheader) {
	print STDERR "WARN!!! $testheader already exist!\n";
	if (!$forceTestHeader) {
	    print STDERR "add --forcetestheader to override it!\n";
	    return;
	}
	print STDERR "Will override it!\n"
    }
    print "Generating $testheader\n";

    my $rc = open FH, ">$testheader" ;
    if (!$rc) {
	print STDERR "Failed to open file $testheader to write:$!\n";
	return;
    }
    print FH genFileHeader(lc($testclassname).".h");
    print FH "#ifndef $testheaderguard\n";
    print FH "#define $testheaderguard\n";
    print FH "#include <cppunit/extensions/HelperMacros.h>\n\n";
    print FH "namespace $namespace {\n";
    print FH "class $testclassname : public CppUnit::TestFixture\n";
    print FH "{\n";
    print FH "    CPPUNIT_TEST_SUITE($testclassname);\n";
    print FH "    CPPUNIT_TEST(test$classname);\n";
    print FH "    CPPUNIT_TEST_SUITE_END();\n";
    print FH "public:\n";
    print FH "    void setUp();\n";
    print FH "    void tearDown();\n";
    print FH "    void test$classname();\n";
    print FH "};\n\n";
    print FH "}/*end namespace $namespace*/\n";
    print FH "#endif/* $testheaderguard*/\n";

}

sub genTestFile() {
    return unless $genTestCPP;
    if (-e $testcpp) {
	print STDERR "WARN!!! $testcpp already exist!\n";
	if (!$forceTestCPP) {
	    print STDERR "add --forcetestcpp to override it!\n";
	    return;
	}
	print STDERR "Will override it!\n";
    }
    print "Generating $testcpp\n";

    my $rc = open FH, ">$testcpp" ;
    if (!$rc) {
	print STDERR "Failed to open file $testcpp to write:$!\n";
	return;
    }

    print FH genFileHeader(lc($testclassname).".cpp");
    print FH "#include \"".lc($testclassname).".h\"\n";
    print FH "#include <$includedir/".lc($classname).".h>\n\n";
    print FH "namespace $namespace {\n";
    print FH "CPPUNIT_TEST_SUITE_REGISTRATION($testclassname);\n\n";
    print FH "void ${testclassname}::setUp() {}\n\n";
    print FH "void ${testclassname}::tearDown() {}\n\n";
    print FH "void ${testclassname}::test$testclassname() {\n\n";
    print FH "}\n\n";
    print FH "}/*end namespace $namespace*/\n";
}

sub genFileHeader($) {
    my $filename = shift;
    my $header =<<__EOF__;
/**
 * File name: $filename
 * Author: $ENV{USER}
 * Create time: $now
 * \$Id\$
 * 
 * Description: ***add description here***
 * 
 */

__EOF__
    return $header;
}
