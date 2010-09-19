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

listbox $t.match
FileToList $t.match /tmp/match

listbox $t.expand
pack $t.match $t.expand -side left -fill both
pack $t.expand -expand 1
FileToList $t.expand /tmp/expand
