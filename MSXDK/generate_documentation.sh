#!/bin/sh

if test ! -d NaturalDocs; then
	echo "NaturalDocs directory not found, creating now ..."
	mkdir NaturalDocs || exit 1
fi
if test ! -f NaturalDocs/NaturalDocs; then
	NDZIP=NaturalDocs-1.16.zip
	echo "The NaturalDocs directory does not contain the Natural Docs package."
	echo "Now trying to extract the package from $NDZIP ..."

	ISMINGW=`uname | grep MINGW`
	if test ! -f $NDZIP; then
		echo -n "$NDZIP not found. Do you want me to try and download it for you? (Y/n)"
		read download_nd
		if test -z $download_nd; then
			download_nd="Y"
		fi
		if test $download_nd = "Y" -o $download_nd = "y"; then
			
			HOST='heanet.dl.sourceforge.net'
			USER='anonymous'
			PASSWD='passwd'
			REMOTEDIR='naturaldocs'
			
			echo "Trying to download $NDZIP now ..."			
			if test $ISMINGW; then
				$COMSPEC <<END_MINGWSCRIPT
				ftp -n $HOST
				quote USER $USER
				quote PASS $PASSWD
				binary
				cd $REMOTEDIR
				get $NDZIP
				quit
END_MINGWSCRIPT
			else
				ftp -n $HOST <<END_SCRIPT
				quote USER $USER
				quote PASS $PASSWD
				binary
				cd $REMOTEDIR
				get $NDZIP
				quit
END_SCRIPT
			fi
		else
			echo "Please download $NDZIP from http://www.naturaldocs.org and put it in the MSXDK directory."
			exit 1
		fi
	fi
	if test ! -f $NDZIP; then
		echo "Failed to download $NDZIP, please download it manually from http://www.naturaldocs.org and put it in the MSXDK directory."
		exit 1
	fi
	
	# The Natural Docs zip exists, now try to unzip it.
	if test $ISMINGW; then
		echo "$NDZIP found, now trying to unzip it ..."
		UNZIP=`which unzip.exe`
		UNZIPCUT=`echo "$UNZIP" | sed s/\ //g`
		if test ! "$UNZIP" -o "$UNZIP" != "$UNZIPCUT"; then
			# No unzip (in the usable path). A path becomes unusable for executables (under mingw/msys) when 
			# it has spaces in it.
			if test ! -f infozip/unzip.exe; then
				# Also no downloaded unzip.exe found.
				echo "No (usable) unzip found, going to try to get/use infozip's unzip now ..."
				if test ! -d infozip; then
					echo "infozip directory not found, creating now ..."
					mkdir infozip || exit 1
				fi
				INFOZIP="unz550xN.exe"
				echo -n "$INFOZIP not found. Do you want me to try and download it for you? (Y/n)"
				read download_unzip
				if test -z $download_unzip; then
					download_unzip="Y"
				fi
				if test $download_unzip = "Y" -o $download_unzip = "y"; then
					HOST_UNZIP='ftp.info-zip.org'
					USER_UNZIP='anonymous'
					PASSWD_UNZIP='passwd'
					REMOTEDIR_UNZIP='/pub/infozip/WIN32'

					echo "Trying to download $INFOZIP now ..."
					$COMSPEC <<END_MINGWUNZIP
					ftp -n $HOST_UNZIP
					quote USER $USER_UNZIP
					quote PASS $PASSWD_UNZIP
					binary
					cd $REMOTEDIR_UNZIP
					lcd infozip
					get $INFOZIP
					quit
END_MINGWUNZIP
					if test ! -f infozip/$INFOZIP; then
						echo "Failed to download $INFOZIP Please download and install  unzip from http://www.info-zip.org"
						echo "Alternatively just unpack the $NDZIP contents into the NaturalDocs directory of MSXDK."
						exit 1
					fi
					cd infozip && $INFOZIP && cd .. || exit 1
				else
					echo "Please download and install unzip from http://www.info-zip.org"
					echo "Alternatively just unpack the $NDZIP contents into the NaturalDocs directory of MSXDK."
					exit 1
				fi
			fi
		fi
		UNZIP="infozip/unzip.exe"
	else
		UNZIP=`which unzip`
		if test ! $UNZIP; then
			echo "unzip is not installed (for this user). Please download and install  unzip from http://www.info-zip.org"
			echo "Alternatively just unpack the $NDZIP contents into the NaturalDocs directory of MSXDK."
			exit 1
		fi
	fi
	$UNZIP $NDZIP -d NaturalDocs || exit 1
fi	
if test ! -f NaturalDocs/patchedForMSXDK; then
	echo "The NaturalDocs package was not yet patched, now trying to apply the MSXDK patch ..."
	patch -p0 < patchNaturalDocs || exit 1
fi

#XXX add real documentation generating commands here

exit 0
