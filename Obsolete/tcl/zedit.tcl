# Text chapter
proc ScrolledText { f width height } {
	frame $f
	# The setgrid setting allows the window to be resized.
	text $f.text -width $width -height $height \
		-setgrid true -wrap none -borderwidth 1\
		-yscrollcommand [list $f.yscroll set]
	scrollbar $f.yscroll -orient vertical \
		-command [list $f.text yview]
	pack $f.yscroll -side right -fill y
	# The fill and expand are needed when resizing.
	pack $f.text -side left -fill both -expand true
	pack $f -side top -fill both -expand true
	return $f.text
}

frame .top -class Top
pack  .top -expand true -fill both

# menubar
label .top.menubar -relief raised -text "File  Edit  Format  Options" \
	-borderwidth 2 -height 2 \
	-font "-dt-interface user-medium-r-normal-m*-*-*-*-*-m-*"

# text/scrollbar/status are in one frame
frame .top.bulk

# paw in seperate frame
label .top.paw -relief raised

pack .top.menubar .top.bulk .top.paw -side top -expand true -fill x

# create text/scrollbar/status line
set t .top.bulk

ScrolledText $t.textscroll 80 24
