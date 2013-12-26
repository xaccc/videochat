rm -Rf player_sample
cp -Rf player player_sample
rm -Rf player_sample/jni
rm -Rf player_sample/gen
rm -Rf player_sample/bin
rm -Rf player_sample/obj
rm -f player_sample.tar.bz2
tar cjf player_sample.tar.bz2 player_sample
rm -Rf player_sample
