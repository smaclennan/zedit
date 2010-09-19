proc List { w name } {
	set t $w.$name
	frame $t
	pack  $t
	label $t.label -text $name
	pack  $t.label -fill x
	frame $t.f1
	pack  $t.f1 -expand 1 -fill both
    listbox $t.f1.list -yscrollcommand "$t.f1.scroll set"
    scrollbar $t.f1.scroll -orient vertical -command "$t.f1.list yview"
	pack $t.f1.list $t.f1.scroll -side left
	pack $t.f1.list -expand 1 -fill both
	pack $t.f1.scroll -fill y
	return $t
}

proc UpdateDir {} {
	global dir
	
	set new [.top.lists.folders.f1.list get active]
	if {$new == ".."} {
		;# special case for up directory
		.top.dentry delete [string last / $dir] 10000
		if { [string length $dir] == 0 } { set dir "/" }
	} else {
		set dir $dir/$new
		regsub {//} $dir {/} dir
	}
	Update
}
	
proc Update {} {
	global dir
	.top.lists.folders.f1.list delete 0 end
	.top.lists.files.f1.list delete 0 end
	if {$dir != "/"} { .top.lists.folders.f1.list insert end ".." }
	foreach f [exec ls $dir] {
		if {[file isdirectory $dir/$f]} {
			.top.lists.folders.f1.list insert end $f
		} else {
			.top.lists.files.f1.list insert end $f
		}
	}
}

proc OutFile {} {
	# send it to Zedit
	socketsend "F$dir/$file"
}

frame .top
pack  .top -expand 1 -fill both
label .top.dlabel -text "Enter path or folder name:"
entry .top.dentry -textvar dir
frame .top.lists
set l1 [List .top.lists folders]
set l2 [List .top.lists files]
pack $l1 $l2 -expand 1 -fill both -side left
label .top.flabel -text "Enter file name:"
entry .top.fentry -textvar file
pack  .top.dlabel .top.dentry .top.lists .top.flabel .top.fentry
pack  .top.dentry .top.fentry -fill x
pack  .top.lists -expand 1 -fill both

set b .top.buttons
frame $b
pack  $b
button $b.ok -text OK -width 6 -command OutFile
button $b.update -text Update -width 6 -command Update
button $b.cancel -text Cancel -width 6 -command { exit 0 }
button $b.help -text Help -width 6 -state disabled
pack $b.ok $b.update $b.cancel $b.help -side left

set dir [exec pwd]
Update

bind .top.dentry <Return> Update
bind .top.fentry <Return> OutFile
bind .top.lists.folders.f1.list <Double-ButtonPress> UpdateDir
bind .top.lists.files.f1.list   <Double-ButtonPress> \
	{set file [.top.lists.files.f1.list get active]}
