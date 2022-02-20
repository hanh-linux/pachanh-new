#!/bin/sh
root="${1}"
tmpdir="${2}"
infodir="$root/var/lib/pachanh/system/"

. $tmpdir/pre-install
for x in $depends; do
	if ! test -d "$infodir/$x"; then
		printf "Dependency $x not found!\n"
		exit 1
	fi
done

