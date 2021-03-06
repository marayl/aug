#!/usr/bin/perl -w

# Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>
#
# This file is part of Aug written by Mark Aylett.
#
# Aug is released under the GPL with the additional exemption that compiling,
# linking, and/or using OpenSSL is allowed.
#
# Aug is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# Aug is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

use strict;

my $CONFIG = 'config.sh';

my ($LINUX, $LINUX_MINGW, $MINGW, $CYGWIN, $CYGWIN_MINGW, $OTHER)
    = (1 .. 6);

my %TOOLSET = (
               $LINUX => 'linux build',
               $LINUX_MINGW => 'linux build for mingw host',
               $MINGW => 'mingw build',
               $CYGWIN => 'cygwin build',
               $CYGWIN_MINGW => 'cygwin build for mingw host (gcc-3)',
               $OTHER => 'other (posix)'
               );

my ($SHARED_ONLY, $STATIC_ONLY, $BOTH) = (1 .. 3);

my %LIBTYPE = (
               $BOTH => 'both',
               $SHARED_ONLY => 'shared only',
               $STATIC_ONLY => 'static only'
               );

sub cygpath {
    my $s = shift;
    if (-x '/usr/bin/cygpath') {
        $s = `/usr/bin/cygpath -m $s`;
        chomp $s;
    }
    return $s;
}

sub trim {
    my $s = shift;
    $s =~ s/^\s+//;
    $s =~ s/\s+$//;
    return $s;
}

sub is {
    my $s = lc(shift);
    return ($s eq '1'
            || $s eq 'y'
            || $s eq 'yes'
            || $s eq 'true') ? 1 : 0;
}

sub valueask {
    my ($prompt, $default) = @_;
    print "$prompt [$default]: ";
    my $answer = <STDIN>;
    unless ($answer) {
        print "(quit)\n";
        exit 0;
    }
    chomp $answer;
    $answer = trim $answer;
    $answer = $default unless length $answer;
    return $answer;
}

sub listask {
    my ($prompt, $default, $pairs) = @_;
    print "\n";
    foreach my $name (sort(keys %$pairs)) {
        if ($name eq $default) {
            print "[$name]\t$pairs->{$name}\n";
        } else {
            print " $name\t$pairs->{$name}\n";
        }
    }
    print "\n";
    my $answer;
    do {
        $answer = valueask($prompt, $default);
        # Loop while bad answer.
    } while (!exists $pairs->{$answer});
    return $answer;
}

my (
    $win32,
    $prefix,
    $toolset,
    $maintainer,
    $gcc,
    $pedantic,
    $debug,
    $multicast,
    $smp,
    $threads,
    $cweb,
    $python,
    $ruby,
    $ssl,
    $libtype
    );

$win32 = (exists $ENV{OS} && $ENV{OS} =~ /windows/i);

$prefix = $win32 ? 'c:/usr/aug' : $ENV{HOME};

if ($win32) {

    if ($^O =~ /cygwin/i) {
        $toolset = $CYGWIN_MINGW;
    } else {
        $toolset = $MINGW;
    }

} else {
    $toolset = $LINUX;
}

# Turn off line-buffering.

$| = 1;

# Collect the input.

$prefix = valueask("prefix directory", $prefix);
$prefix =~ s{^~([^/]*)}
{
    $1 ? (getpwnam($1))[7]
        : ($ENV{HOME} || $ENV{LOGDIR} || (getpwuid($>))[7])
    }ex;
$prefix = cygpath($prefix);
$toolset = listask("compiler toolset", $toolset, \%TOOLSET);
$maintainer = valueask("maintainer mode", 'n');
if ($CYGWIN_MINGW == $toolset) {
    $gcc = 'y';
} else {
    $gcc = valueask("GCC compiler", 'y');
}
$pedantic = valueask("pedantic warnings", 'n');
$debug = valueask("debug build", 'n');
$multicast = valueask("multicast support", 'y');
$smp = valueask("smp support", 'y');
$threads = valueask("thread support", 'y');
$cweb = valueask("cweb support", 'n');
$python = valueask("python support", 'n');
$ruby = valueask("ruby support", 'n');
$ssl = valueask("ssl support", 'y');
$libtype = listask("library type", $BOTH, \%LIBTYPE);

my (
    $flags,
    $cflags,
    $cxxflags,
    $options
    );

if (is $gcc) {
    my $sse2 = valueask("sse2 extensions", 'y');
    my $gprof = valueask("gprof build", 'n');
    $flags = '-march=i686';
    $flags .= (is $sse2) ? ' -msse2' : '';
    $flags .= (is $gprof) ? ' -pg' : '';
    $flags .= (is $debug) ? ' -ggdb' : ' -O3 -DNDEBUG';
    $flags .= ' -Wno-long-long -fno-strict-aliasing -Wno-unused-value';
    $cflags = "$flags";
    $cxxflags = "$flags -Wno-deprecated -Wno-unused-variable";
    if (is $pedantic) {
        $cflags = "-Wall -pedantic $cflags";
        $cxxflags = "-Wall -pedantic $cxxflags";
    }
} else {
    $flags = (is $debug) ? '-g' : '-O -DNDEBUG';
    $cflags = $flags;
    $cxxflags = $flags;
}

# Write script.

open(FILE, ">$CONFIG")
    or die "open() failed: $!\n";

print FILE "#!/bin/sh\n\n";

if ($win32) {
    print FILE <<EOD;
GRAPHVIZ_HOME=/toolset/graphviz
MIKTEX_HOME=/toolset/miktex/miktex
OPENSSL_HOME=/toolset/openssl
OPENSSL_CONF=\$OPENSSL_HOME/bin/openssl.cfg
PYTHON_HOME=/toolset/python
RUBY_HOME=/toolset/ruby
PATH=\$HOME/bin:/toolset/bin:\$GRAPHVIZ_HOME/bin:\$MIKTEX_HOME/bin:\$OPENSSL_HOME/bin:\$PYTHON_HOME:\$RUBY_HOME/bin:/usr/bin:/usr/bin; export PATH
export GRAPHVIZ_HOME MIKTEX_HOME OPENSSL_CONF OPENSSL_HOME PYTHON_HOME RUBY_HOME
EOD
}

my $s = cygpath($ENV{OPENSSL_HOME});

$options = "--prefix=$prefix";
$options .= " \\\n\t--enable-maintainer-mode"
    if is $maintainer;
$options .= " \\\n\t--disable-multicast"
    unless is $multicast;
$options .= " \\\n\t--disable-smp"
    unless is $smp;
$options .= " \\\n\t--disable-threads"
    unless is $threads;
$options .= " \\\n\t--with-cweb"
    if is $cweb;
$options .= " \\\n\t--with-python"
    if is $python;
$options .= " \\\n\t--with-ruby"
    if is $ruby;

if (is $ssl) {
    if (exists $ENV{OPENSSL_HOME}) {
        my $s = $ENV{OPENSSL_HOME};
        # Escape backslashes.
        $s =~ s/\\/\\\\/g;
        $s = cygpath($s);
        $options .= " \\\n\t--with-ssl=$s";
    }
} else {
    $options .= " \\\n\t--without-ssl"
}

if ($SHARED_ONLY == $libtype) {
    $options .= " \\\n\t--disable-static";
} elsif ($STATIC_ONLY == $libtype) {
    $options .= " \\\n\t--disable-shared";
}

if ($CYGWIN_MINGW == $toolset) {
    print FILE "CC='gcc-3 -mno-cygwin'; export CC\n";
    print FILE "CXX='g++-3 -mno-cygwin'; export CXX\n";
    $options .= " \\\n\t--disable-dependency-tracking";
} elsif ($MINGW == $toolset) {
    print FILE "MINGW_HOME=/toolset/mingw; export MINGW_HOME\n";
    print FILE "PATH=\$MINGW_HOME/bin:\$PATH; export PATH\n";
    $options .= " \\\n\t--disable-dependency-tracking";
    $options .= " \\\n\t--build=i686-pc-cygwin";
    $options .= " \\\n\t--host=mingw32";
} elsif (2 == $toolset) {
    $options .= " \\\n\t--build=i586-pc-linux-gnu";
    $options .= " \\\n\t--host=i586-mingw32msvc";
}

print FILE "CFLAGS='$cflags'; export CFLAGS\n";
print FILE "CXXFLAGS='$cxxflags'; export CXXFLAGS\n";
print FILE "rm -f config.cache && sh ./configure \\\n\t$options\n";

close FILE;
print "\nrun script as follows:\n\$ sh $CONFIG\n";
