proc FileToList {list file} {
	$list delete 0 end

	if [catch {open $file} stream] {
		puts "Unable to open help file."
		return
	}

	while {[gets $stream line] != -1} {
		$list insert end $line
	}
	
	close $stream
}

frame .top
pack .top -expand 1 -fill both

set t .top.lists
frame $t
pack $t -expand 1 -fill both

frame $t.name
frame $t.type
frame $t.size
pack $t.name $t.type $t.size -side left -fill y
pack $t.name -fill both -expand 1

proc selectValid {} {
	set t .top.lists
	set name [$t.name.list get active]

	exec valid_font $name
	FileToList $t.type.list /tmp/types
	FileToList $t.size.list /tmp/sizes
	
	# if only one type or medium type available, select it
	if { [$t.type.list size] == 1 } {
		$t.type.list selection set 1
	} else {
		set index [lsearch -exact [$t.type.list get 0 end] medium]
		if { $index != -1 } { $t.type.list selection set $index }
	}
}

scrollbar $t.name.scroll -command "$t.name.list yview"
listbox $t.name.list -yscroll "$t.name.scroll set" \
	-selectmode single -exportselection false
pack $t.name.scroll -side right -fill y
pack $t.name.list -fill both -expand 1
FileToList $t.name.list /tmp/names

listbox $t.type.list -selectmode single -exportselection false
pack $t.type.list -fill y -expand 1
FileToList $t.type.list /tmp/types

scrollbar $t.size.scroll -command "$t.size.list yview"
listbox $t.size.list -yscroll "$t.size.scroll set" \
	-selectmode single -exportselection false
pack $t.size.scroll -side right -fill y
pack $t.size.list -fill y -expand 1
FileToList $t.size.list /tmp/sizes

bind $t.name.list <Double-1> selectValid

button .top.select -command doit
pack .top.select -side bottom

proc doit {} {
	set t .top.lists
	
	set name [$t.name.list get active]
	set type [$t.type.list get active]
	set size [$t.size.list get active]
	
	socketsend "VFont -$name-$type-r-*-*-$size-*-*-*-*-*-*-*"
}
