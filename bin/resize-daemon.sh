#!/bin/bash
#
# resize-daemon.sh
#
# Simple shell script which uses the "xclip" utility to monitor the clipboard
# contents, watching for a "magic packet" of the form:
#
# [BEGIN RESIZE]
# WIDTH
# HEIGHT
# [END RESIZE]
#
# where "WIDTH" and "HEIGHT" are the integer screen width and height
# respectively (in pixels). If such a packet is found within the clipboard, the
# screen is resized using the "xrandr" utility, and the previous clipboard
# contents are restored.
#

##
## The last clipboard contents read by the daemon, excluding the magic packet.
## If the magic packet is read, the clipboard will be restored to its previous
## state using this value.
##
PREVIOUS_CLIPBOARD=""

##
## Reads a single magic resize packet from STDIN, resizing the screen using
## xrandr if such a packet is read. The magic resize packet is simply 
## "[BEGIN RESIZE]", followed by the integer width, height, and "[END RESIZE]",
## each on their own separate lines. For example, to resize the screen to
## 1024x768, the following would packet would be used:
##
## [BEGIN RESIZE]
## 1024
## 768
## [END RESIZE]
##
## @return
##     0 (success) if the magic packet was found and successfully read, 1
##     (fail) otherwise.
read_resize_packet() {

    # Attempt to read magic resize packet
    read BEGIN
    read WIDTH
    read HEIGHT
    read END

    # If magic resize packet is valid, resize screen and return success
    if [ "$BEGIN" = "[BEGIN RESIZE]" -a "$END" = "[END RESIZE]" ]; then
        xrandr --fb "${WIDTH}x${HEIGHT}"
        return 0
    fi

    # No packet was read - fail
    return 1

}

# Poll the clipboard for the magic resize packet
while sleep 0.25s; do

    # Read the current clipboard contents into an environment variable
    CLIPBOARD="$(xclip -selection clipboard -o 2> /dev/null)"

    # Attempt to read the magic resize packet from the clipboard, restoring the
    # previous clipboard contents if it was read.
    if echo "$CLIPBOARD" | read_resize_packet; then
        echo "$PREVIOUS_CLIPBOARD" | xclip -selection clipboard -i

    # If the clipboard does NOT contain the magic packet, store its contents
    # internally for future use.
    else
        PREVIOUS_CLIPBOARD="$CLIPBOARD"
    fi

done

