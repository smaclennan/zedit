#SAM catch {option readfile /home/sam/burp/Zblist.ad startup} set

frame .top -class Top
pack  .top -expand true -fill both

scrollbar .top.vscroll -command ".top.blist yview"
listbox .top.blist -yscroll ".top.vscroll set"
pack .top.blist -side left -fill both -expand 1
pack .top.vscroll -side left -fill y

bind .top.blist <Double-1> Switchto

if (0) {
# Just append
proc AddList {s} { .top.blist insert end $s }
} else {
# Sort
proc AddList {s} {
	set l [.top.blist get 0 end]	;# get the list
	set l [lsort [lappend l $s]]	;# append new and sort
	set n [lsearch -exact $l $s]	;# get the index of new
	.top.blist insert $n "$s"		;# insert it
}
}

proc DelList {s} {
	set n [lsearch -exact [.top.blist get 0 end] $s]
	if {$n != -1} { .top.blist delete $n }
}

proc Switchto {} {
	set s [.top.blist get active]
	socketsend "B$s"
}

proc Highlight {s} {
	.top.blist selection clear active
	set l [.top.blist get 0 end]	;# get the list
	set n [lsearch -exact $l $s]	;# get the index
	if {$n != -1} {
		.top.blist selection set $n
		.top.blist see $n
		.top.blist activate $n
	}
}
