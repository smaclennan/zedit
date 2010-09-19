frame .top -class Top
pack  .top -expand 1 -fill both

frame .top.string
frame .top.num
frame .top.flag
frame .top.buttons
pack .top.string .top.num .top.flag .top.buttons -expand 1 -fill x

frame .top.flag.flag1
frame .top.flag.flag2
pack .top.flag.flag1 .top.flag.flag2 -side left

button .top.buttons.save -text "Save Config" -command SaveConfig
pack .top.buttons.save

# Received via socket to add a variable to the list
proc VarAdd { type name value } {
	# add line based on type
	switch $type \
		F "AddFlag $name $value" \
		D "AddDecimal $name $value" \
		S {AddString $name "$value"} \
		E "DoneList"
}

global flag
set flag flag1

proc AddFlag {name value} {
	global flag

	# must check both cols
	if {[winfo exists .top.flag.flag1.a$name] == 1} {
		set w .top.flag.flag1.a$name
		if {$value == 0} { $w.button deselect } else { $w.button select }
		return
	} elseif {[winfo exists .top.flag.flag2.a$name] == 1} {
		set w .top.flag.flag2.a$name
		if {$value == 0} { $w.button deselect } else { $w.button select }
		return
	}

	set w .top.flag.$flag.a$name
	frame $w
	pack  $w -expand 1 -fill x
	entry $w.label -relief flat
	$w.label insert 0 $name
	checkbutton $w.button -variable $name -command "SetFlag $name"
	if {$value != 0} { $w.button select }
	pack $w.label -expand 1 -fill x -side left
	pack $w.button -side left

	bind $w.button <Help> "Help $name"
	bind $w.label  <Help> "Help $name"
	
	if {$flag == "flag1"} {
		set flag "flag2"
	} else {
		set flag "flag1"
	}
}

# Decimals and Strings are the same except for the base frame
proc EntryValue {w name value} {
	if {[winfo exists $w] == 1} {
		$w.entry delete 0 end
		$w.entry insert 0 $value
		return
	}

	frame $w
	pack $w -expand 1 -fill x
	label $w.label -text $name
	entry $w.entry
	$w.entry insert 0 $value
	pack $w.label -expand 1 -fill x -side left
	pack $w.entry -side left
	bind $w.entry <Return> "SetVariable $w.entry $name"

	bind $w.entry <Help> "Help $name"
	
	return $w.entry
}

proc AddDecimal {name value} {
	EntryValue .top.num.a$name $name $value
}

proc AddString {name value} {
	set w [EntryValue .top.string.a$name $name $value]
}

proc DoneList {} {
	global flag

	if {$flag == "flag2"} {
		label .top.flag.flag2.pad -bd 7
		pack .top.flag.flag2.pad
	}
}

proc SetVariable {w name} {
	set value [$w get]
	socketsend "V$name $value"
}

proc SetFlag {name} {
	socketsend "V$name"
}

proc Help {name} {
	global env

	if [catch {open $env(ZPATH)/varhelp.z} stream] {
		puts "Unable to open help file."
		return
	}

	while {[gets $stream line] != -1} {
		if {$line == ":$name"} {
			set w .help$name
			toplevel $w
			wm title $w "Help on $name"
			text $w.text -wrap word
			pack $w.text -expand 1 -fill both

			button $w.done -text Done -command "destroy $w"
			pack $w.done -fill x
	
			gets $stream line	;# skip empty line
			while {[gets $stream line] != -1} {
				if {[string range $line 0 0] == ":"} { break }
				$w.text insert end "$line\n"
			}
			close $stream
			return
		}
	}

	puts "Sorry. No help for $name."
	close $stream
}

proc SaveConfig {} {
	exec rm -f .config.z
	socketsend Vsave
}
