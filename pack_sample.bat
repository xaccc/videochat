@echo off

rm -rf player_sample
xcopy /s /i /y player player_sample
rm -rf player_sample/jni
rm -rf player_sample/gen

rm -f player_sample.tar.bz2
tar cjf player_sample.tar.bz2 player_sample
rm -rf player_sample

@echo 点击任意键退出...
@pause