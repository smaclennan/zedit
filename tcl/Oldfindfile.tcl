set dir [pwd]

# commandEntry.tcl

proc CommandEntry { name label width command args } {
	frame $name
	label $name.label -text $label -width $width -anchor w
	eval {entry $name.entry -relief sunken} $args
	pack $name.label -side left
	pack $name.entry -side right -fill x -expand true
	bind $name.entry <Return> $command
	return $name.entry
}

# Y scrollbars only
proc ScrolledListbox { parent args } {
	frame $parent
	# Create listbox attached to scrollbars, pass thru $args
	eval {listbox $parent.list \
		-yscrollcommand [list $parent.vscroll set]} $args
	scrollbar $parent.vscroll -orient vertical \
		-command [list $parent.list yview]
	# Arrange everything in the parent frame
	pack $parent.vscroll -side right -fill y
	pack $parent.list -side left -fill both -expand true
	return $parent.list
}

# filelist.tcl
proc Cfiles { dir } {

	if {$dir != "/"} { lappend list .. }
	foreach f [exec ls $dir] {
		if {[file isdirectory $dir/$f]} {
			lappend list "$f/"
		} elseif {[file extension $f] != ".o"} {
			lappend list $f
		}
	}
	return $list
}

proc UpdateDir {} {
	set dir [.filelist.dir.entry get]
	if {[file isdirectory $dir] == 0} {
		puts "$dir not a directory"
		return
	}
	.filelist.list.list delete 0 10000	;# delete entire list
	eval {.filelist.list.list insert 0} [Cfiles $dir]
}

proc OutFile {} {
	# get the directory name from the entry and remove any trailing /
	regsub {/$} [.filelist.dir.entry get] {} dir
	# get the file name from the listbox
	set fname [.filelist.list.list get active]

	if {$fname == ".."} {
		;# special case for up directory
		.filelist.dir.entry delete [string last / $dir] 10000
		UpdateDir
	} elseif {[file isdirectory $dir/$fname]} {
		.filelist.dir.entry delete 0 10000
		.filelist.dir.entry insert 0 $dir/$fname
		UpdateDir
	} else {
		socketsend "F$dir/$fname"
	}
}

proc FileList {dir parent} {
	set entry [CommandEntry $parent.dir Directory 9 UpdateDir]
	$entry insert 0 $dir
	ScrolledListbox $parent.list
	bind .filelist.list.list <Double-ButtonPress> OutFile
	eval {$parent.list.list insert 0} [Cfiles $dir]
	pack $parent.dir -side top -fill x
	pack $parent.list -side top -expand true -fill both

	bind $parent.list <Double-1> OutFile
}

# Read app-defaults file and silently ignore errors
# SAM global env
# SAM catch {option readfile $env(ZPATH)/Zfindfile.ad startup} err

# Create top level frame
frame .filelist -class Top
pack .filelist -expand true -fill both

FileList $dir .filelist
