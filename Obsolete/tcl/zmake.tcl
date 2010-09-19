frame .top -class Top
pack  .top -fill both -expand true

# Create a text widget to log the output
frame .top.t
pack  .top.t -fill both -expand true

set normal [.top.t cget -background]

set log .top.t.log
scrollbar .top.t.hscroll -orient horiz -command "$log xview"
scrollbar .top.t.vscroll -orient vert  -command "$log yview"
pack .top.t.hscroll -side bottom -fill x
pack .top.t.vscroll -side right  -fill y

text $log -wrap none -width 80 -height 10 -state disabled \
	-xscrollcommand ".top.t.hscroll set" \
	-yscrollcommand ".top.t.vscroll set"
pack $log -fill both -expand true

# Create a frame for buttons and entry.
frame .top.b
pack  .top.b -side top -fill x
set make [button .top.b.make -text "Make" -command Make]
set next [button .top.b.next -text "Next Error" -command NextError \
	-state disabled]
checkbutton .top.b.warn -text "Warnings" -variable warnings
set stop [button .top.b.stop -text Stop -command Stop -state disabled]
set cmd [entry .top.b.cmd]
pack .top.b.make .top.b.next .top.b.warn .top.b.stop -side left
pack .top.b.cmd -fill x -side left -expand 1

bind $cmd <Return> Make

# SAM I don't know how to set this from the .Xdefaults
.top.b.warn select

# Run the program and arrange to read its input
proc Run {command} {
	global input log stop next sline make cmd normal

	# update command window with new command
	$cmd delete 0 end
	$cmd insert 0 $command

	.top.t configure -background $normal

	if [catch {open "|$command |& cat"} input] {
		$log insert end $input\n
		.top.t configure -background red
	} else {
		set sline 1
		$log config -state normal
		$log delete 0.0 end
		fileevent $input readable Log
#SAM		$log insert end $command\n
		$next config -state normal
		$stop config -state normal
		$make config -state disabled
	}
}
# Read and log output from the program
proc Log {} {
	global input log

	if [eof $input] {
		Stop
	} else {
		gets $input line
		$log insert end $line\n
		$log see end
	}
}

# Stop the program and fix up the button
proc Stop {} {
	global input stop log make
	if [catch {close $input}] {
		# command had non-zero exit status
		.top.t configure -background red
	}
	$stop config -state disabled
	$log  config -state disabled
	$make config -state normal
	puts -nonewline "\7"
}

# Find next error in log
proc NextError {} {
	global log sline warnings next

	set eline [expr $sline + 1]
	while (1) {
		set str [$log get $sline.0 $eline.0]
		if {$str == ""} { break }

		set out [parseoutput $warnings $str]
		if {$out != "0"} {
			$log config -state normal
			$log tag remove sel 1.0 end
			$log tag add sel $sline.0 $eline.0
			$log see sel.first
			$log config -state disabled
			
			socketsend "N$out"
			
			set sline $eline
			return
		}

		set sline $eline
		set eline [expr $eline + 1]
	}
	
	$next config -state disabled
}

# Send make to host
proc Make {} {
	global make cmd

	# since it may take awhile to get the Make to the target,
	# disable now...	
	$make config -state disabled
	socketsend M[$cmd get]
}
