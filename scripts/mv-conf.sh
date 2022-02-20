#!/bin/sh
root="${1}"
tmpdir="${2}"

. $tmpdir/pre-install
for x in $config; do
	if ! test -f "$root/$x"; then
		mv $root/$x.newfile $root/$x || exit 1
	fi
done

