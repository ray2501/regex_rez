# -*- tcl -*-
# Tcl package index file, version 1.1
#
if {[package vsatisfies [package provide Tcl] 9.0-]} {
    package ifneeded regex_rez 0.3 \
	    [list load [file join $dir libtcl9regex_rez0.3.so] [string totitle regex_rez]]
} else {
    package ifneeded regex_rez 0.3 \
	    [list load [file join $dir libregex_rez0.3.so] [string totitle regex_rez]]
}
