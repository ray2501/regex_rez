regex_rez
=====

Tcl bindings for [RE2](https://github.com/google/re2).


License
=====

This package is Licensed under MIT license.


UNIX BUILD
=====

Only test on openSUSE. Users need install RE2 development files first.

     sudo zypper install re2-devel

Building under most UNIX systems is easy, just run the configure script and
then run make. For more information about the build process, see the
tcl/unix/README file in the Tcl src dist. The following minimal example will
install the extension in the /opt/tcl directory.

    $ cd regex_rez
    $ export CC=g++
    $ ./configure --prefix=/opt/tcl
    $ make
    $ make install

If you need setup directory containing tcl configuration (tclConfig.sh),
below is an example:

    $ cd regex_rez
    $ export CC=g++
    $ ./configure --with-tcl=/opt/activetcl/lib
    $ make
    $ make install


Implement commands
=====

regex::re2 expr_string ?-utf8 boolean? ?-posix boolean? ?-longest-match boolean? ?-nocase boolean?  
REGEX_HANDLE fullmatch text  
REGEX_HANDLE partialmatch text  
REGEX_HANDLE replace text rewrite  
REGEX_HANDLE globalreplace text rewrite  
REGEX_HANDLE close  


Examples
=====

partial match example:

     package require regex_rez

     set re [regex::re2 create "b+"]
     set result [$re partialmatch "yabba dabba doo"]
     if {$result == 1} {
         puts "Match"
     } else {
         puts "Not match"
     }
     $re close

