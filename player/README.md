Video Chat Player

需要几点注意的地方：

在AndroidMainifest.xml中必须定义OpenGL ES 2
<uses-feature android:glEsVersion="0x00020000" /> 

必须具备以下权限：
<uses-permission android:name="android.permission.INTERNET" />
<uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS" />

