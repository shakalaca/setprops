# What is this ?
Android properties management tool by shakalaca@gmail.com

# You wrote this ?
I stole the codes from [AOSP](android.googlesource.com) except setprops.cpp and *.mk ;-)

# How to build ?
* Install and setup [Android NDK](https://developer.android.com/ndk/index.html)

You can then
* ```mkdir setprops; cd setprops; git clone https://github.com/shakalaca/setprops jni```
* `ndk-build`

or
* ```git clone https://github.com/shakalaca/setprops; cd setprops```
* ```ndk-build NDK_APPLICATION_MK=`pwd`/Application.mk NDK_PROJECT_PATH=`pwd` APP_BUILD_SCRIPT=Android.mk```

# How to use ?
#### Import (override) properties from file
- `setprops --import filename`
- ex: `setprops --import my.prop`

#### Change properties
- `setprops prop value`
- ex: `setprops ro.boot.verifiedbootstate green`

#### Remove properties (You have to do `ln -s setprops delprops` first)
- `delprops prop_1 prop_2`
- ex: `delprops ro.frp.pst ro.carrier`
