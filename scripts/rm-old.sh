#!/bin/sh
root="${1}"
tmpdir="${2}"
infodir="$root/var/lib/pachanh/system/"

. "$tmpdir"/pre-install
for x in $(cat "$tmpdir"/oldfiles); do 
	rm -f "$root"/"$x" 
done
rm -rf "$tmpdir"
