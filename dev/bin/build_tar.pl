#!/usr/bin/perl
#copy stuff we want to go into the source tar ball

use strict;
use warnings;


use FindBin;
use File::Path;
use File::Copy;
use Date::Format;
use Getopt::Long qw(GetOptions);
use Cwd;

my $APP_NAME = 'logstalgia';

sub app_version {
    my $version = `cat $FindBin::Bin/../../src/settings.h | grep LOGSTALGIA_VERSION`;
    $version =~ /"([^"]+)"/ or die("could not determine version\n");
    $version = $1;
    return $version;
}

my $VERSION = app_version();

my @exclusions = (
    qr{^/autogen\.sh$},
    qr{^/cmd/},
    qr{^/config.status$},
    qr{^/config.log$},
    qr{^/logstalgia$},
    qr{^/dev/},
    qr{^/logs/},
    qr{/\.},
    qr{Makefile$},
    qr{\.o$},
    qr{^/todo.txt$},
    qr{^/build-stamp$},
    qr{^/autom4te},
    qr{^/src/core/README$},
    qr{^/src/core/ui/},
);

my @inclusions = (
    qr{^/logstalgia\.win32\.cbp$},
    qr{^/ChangeLog$},
    qr{^/THANKS$},
    qr{^/COPYING$},
    qr{^/INSTALL$},
    qr{^/README$},
    qr{/Makefile\.in$},
    qr{/Makefile\.am$},
    qr{^/aclocal\.m4$},
    qr{^/m4/.+\.m4$},
    qr{^/configure(?:\.ac)?$},
    qr{^/src/.+\.(?:cpp|h|cc|hh)$},
    qr{^/data/.+\.(?:png|tga|ttf|1)$},
    qr{^/data/fonts/README$},
    qr{^/data/example\.log$},
    qr{^/build-aux/(?:compile|config.(?:guess|sub)|depcomp|install-sh|missing)$},
);

my $tmp_path = "/var/tmp/$APP_NAME-$VERSION";

system("rm -r $tmp_path") if -d $tmp_path;
mkpath($tmp_path) or die("failed to make temp folder $tmp_path");

chdir("$FindBin::Bin/../../");

my @files = `find .`;

#check configure.ac has been updated
unless(`cat configure.ac` =~ /AC_INIT\(Logstalgia, $VERSION,/) {
    die("configure.ac does not mention current version number\n");
}

#check ChangeLog has been updated
unless(`cat ChangeLog` =~ /^$VERSION:/) {
    die("ChangeLog does not mention current version number\n");
}

#if Makefile exists, do distclean
if(-e 'Makefile') {
    if(system("make distclean") != 0) {
        die("make distclean failed: $!\n");
    }
}

#reconfigure
if(system("autoreconf -f -i -v") != 0) {
    die("autoreconf failed: $!\n");
}

foreach my $file (@files) {
    $file =~ s/[\r\n]+//;
    $file =~ s/^\.//;

    (my $relfile = $file) =~ s{^/}{};
    (my $dir = $file)    =~ s{[^/]+$}{/};

    next if $file =~ /^\s*$/;
    next if -d $relfile;

    next if grep { $file =~ $_ } @exclusions;

    unless(grep { $file =~ $_ } @inclusions) {
        warn "nothing known about $file\n";
        next;
    }

    mkpath("$tmp_path/$dir");
    if(system("cp", "$relfile", "$tmp_path/$relfile") != 0) {
        die("error copying $file to $tmp_path/$relfile: $? $!\n");
    }
}

my $current_dir = cwd;
chdir("/var/tmp/");

my $archive = "$APP_NAME-$VERSION.tar.gz";

if(system("tar -czf $archive $APP_NAME-$VERSION") !=0) {
    die("failed to make archive $archive");
}

unlink("$FindBin::Bin/../builds/$archive");

my $builds_dir = "$FindBin::Bin/../builds/";

system('mkdir', '-p', $builds_dir);

unlink("$builds_dir/$archive") if -e "$builds_dir/$archive";

move("$archive", $builds_dir);
system("rm -r $tmp_path");

print "Built $archive\n";
