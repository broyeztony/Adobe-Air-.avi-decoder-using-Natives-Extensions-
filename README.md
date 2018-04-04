# Adobe-Air-.avi-decoder-using-Natives-Extensions-
.avi decoder for the Adobe AIR platform using the Native Extensions api and C/DirectShow for the video decoding.

This is a Proof-of-Concept using the Adobe AIR Natives Extensions feature (https://www.adobe.com/devnet/air/native-extensions-for-air.html)

The Adobe AIR runtime is not natively able to render .avi files. This PoC shows that it possible to do it using 
the Natives Extensions.

With Natives Extensions, it is possible to extend the capabilities of the AIR platform by creating a bridge between the AIR runtime 
and libraries developped in other languages.

In this project, i used C++ along with the Direct Show library to parse a .AVI video file, grab the images from the file, 
decode the pixels in each image and write them in real-time to a in-memory Texture (FREObject) that is being shared between
the 2 worlds:  C++/OpenGL write to the pixels, Adobe AIR runtime gets notifications (invalidate state) when the texture's pixels have been 
updated, reads from them and renders the texture using BitmapData (https://help.adobe.com/fr_FR/FlashPlatform/reference/actionscript/3/flash/display/BitmapData.html)




