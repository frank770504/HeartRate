#adb root and remount the device
device_root()
{
        adb wait-for-device
        adb root
        adb wait-for-device
        adb remount
        adb wait-for-device
        #echo "adb root and remount ok!!!"
}

_regulator="8226_l19 \
            8226_lvs1 \
            8226_l28"

checkc_regulator()
{
	echo "${FUNCNAME} ----------"
	for _reg in $_regulator
	do
		echo "check regulator: $_reg"
		_enable=$( adb shell "cat d/regulator/$_reg/enable" | tr -d '\r')
		if [ $_enable == "1" ]; then
			echo "$_reg is enable"
		else
			echo "$_reg is disable"
		fi
	done
}

op_regulator()
{
#$1 regulator
#$2 command
	echo "${FUNCNAME} ----------"
	_reg=$1
	case $2 in
		"en")
			adb shell "echo 1 >> d/regulator/$_reg/enable"
		;;
		"dis")
			adb shell "echo 0 >> d/regulator/$_reg/enable"
		;;
		*)
		;;
	esac

}

_PUSH_PATH="system/bin"

#push binary
push_bin()
{
#$1 the binary name
#$2 binary path
#$3 the push path

        adb push $2/$1 $3
        adb shell "chmod 777 $3/$1"
}


get_into_passthr()
{
	echo "${FUNCNAME} ----------"
	adb shell "rmmod em718x" #TODO what if SENtral init is failled
	adb shell "insmod system/lib/modules/em718x.ko passthrough=1"
}



get_module()
{
	cp $_pSmb/system/bin/$1 .
}


###################################################################################
#                                    main                                         #
###################################################################################
device_root

#_ModuleName=HRModule
#_ModuleName=i2c-ctrl
_ModuleName=HRMTest

if [ -z $1 ]; then
get_module $_ModuleName
push_bin $_ModuleName "." $_PUSH_PATH
adb push libpaw8001motion.so system/lib
get_into_passthr
#op_regulator 8226_l28 dis
#checkc_regulator

#op_regulator 8226_l28 en
checkc_regulator

adb shell "system/bin/$_ModuleName"

else
	case $1 in
		"pass")
		get_into_passthr
		;;
		"dishr")
			op_regulator 8226_l28 dis
		;;
		"enhr")
			op_regulator 8226_l28 en
		;;
		"m")
		op_regulator 8226_l28 en
		checkc_regulator
		adb shell "system/bin/$_ModuleName"
		;;
		*)
		;;
	esac

fi
