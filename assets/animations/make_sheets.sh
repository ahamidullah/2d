#!/bin/bash
ncols=4
for sheet in *
do
	sheet_fpath="$sheet"/"$sheet".png
	anim_fpath="$sheet"/"$sheet".anim

	if [ ! -d "$sheet" ]; then continue; fi
	echo "processing sheet $sheet"
	echo ["$sheet"] > "$anim_fpath"
	echo $(pwd)/"$sheet_fpath" >> "$anim_fpath"

	anim_i=0
	for anim in "$sheet"/*
	do
		if [[ ! -d "$anim"  || ! "$(ls -A $anim)" ]]; then continue; fi
		echo "	processing anim $anim"
		anim_name=$(basename "$anim")
		echo ["$anim_name"] >> "$anim_fpath"

		IFS=$'\r\n' delays=($(awk '/"[0-9]+"/{gsub(/"/,"",$1); print $1}' "$anim"/"$anim_name".csv))
		unset IFS

		echo $(ls -l "$anim"/*.png | wc -l) >> "$anim_fpath"

		for sprite in "$anim"/*
		do
			if [[ ! -e "$sprite" || ${sprite: -4} != ".png" ]]; then continue; fi
			echo "		processing sprite $sprite"
			spritew=$(identify -format "%[w]" "$sprite")
			spriteh=$(identify -format "%[h]" "$sprite")
			ncols=2
			x=$(expr "$anim_i" / "$ncols" \* "$spritew")
			y=$(expr $(expr "$anim_i" + "$ncols" - 1) / "$ncols" \* "$spriteh")

			printf '%d,%d,%d,%d,%d\n' "${delays[anim_i]}" "${x}" ${y} ${spritew} ${spriteh} >> "$anim_fpath"
			sprites[anim_i]="$sprite"
			let anim_i+=1
		done

	done

	IFS=$'\n' sprites=($(sort -n -t _ -k 2 <<<"${sprites[*]}"))
	sprites=($(sort -d -t _ -k 1,1 -s <<<"${sprites[*]}"))
	unset IFS

	nrows=$(expr $(expr "$anim_i" + "$ncols" - 1) / "$ncols")
	echo "making sheet $(basename "$sheet_fpath")"
	montage-im6 -geometry +"$ncols"+"$nrows" ${sprites[*]} -format png24 "$sheet_fpath"
done

