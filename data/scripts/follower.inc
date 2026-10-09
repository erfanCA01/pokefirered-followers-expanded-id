EventScript_Follower::
	lock
	callnative ScrFunc_bufferlivemonnickname
	.byte 0
	callnative ScrFunc_playfirstmoncry
	callnative ScrFunc_getfolloweraction
EventScript_FollowerEnd::
	release
	end

EventScript_FollowerGeneric::
	faceplayer
	waitfieldeffect FLDEFF_QUESTION_MARK_ICON
	message 0x0
	waitmessage
	waitbuttonpress
	return
