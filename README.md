inFORM_thesis

========================
#What does the program do
========================

This program plays back the movie clips for the self assembly scenario and the assembly primitives videos.

The program can also be used to establish a connection with 3dsMax using the EasyPin script. This is useful for quickly creating animations that are supposed to run on the inFORM. The single frames that are sent from 3dsMax to the inFORM can be exported as a movie file from this program.



======================
#How to use the program
======================

##TCP Mode

[1]: TCP mode";
[e] toggle export video flag
[l] toggle looping video";
[s] stop and reset video
[c] clear frame buffer array
[d] delete current frame from frame buffer array
[RIGHT] next frame
[LEFT] previous frame

Make sure the ip address and port numbers are set up correctly in inFORM_thesis and EasyPin before compiling. 

Click "start client" in EasyPin and confirm that a connection is established. Now you can follow the EasyPin instructions (INSERT LINK HERE)

If you enable export video mode [e] and playback the frame buffer's content a video named "fromMax" will be saved in your data/video/ folder. If you restart the program the exported video will be included in the video list dropdown menu. 


##Video playback (assembly mode)
    
[2] video playback/assembly mode;
[l] toggle looping video
[b] set the playhead to the beginning of the video
[space] toggle pause video
[DOWN] next video
[UP] previous video
[RIGHT] next frame
[LEFT] previous frame



#How to run the program

##Make sure you are using openFrameworks 0.8.0

You can download OF 0.8.0 here
https://github.com/openframeworks/openFrameworks/tree/0.8.0


#M#ake sure the CoreOF.xconfig file is updated

open /Users/bimster/Sites/OF/of_v0.8.0_osx_release/libs/openFrameworksCompiled/project/osx/CoreOF.xcconfig
add "$(LIB_FREEIMAGE)" and "$(LIB_FREETYPE)" to the OF_CORE_LIBS

go to /Users/bimster/Sites/OF/of_v0.8.0_osx_release/libs/openFrameworksCompiled/lib/osx/ and delete everything in that folder


##Make sure you clone the app into the proper folder structure

The content of inFORM_thesis should be inside of_v0.8.0_osx_release/apps/INFORM/inFORM_thesis/


##Make sure all dependencies are linked correctly

In Xcode in the left column expand the addons folder and check the inside of the addons folder if the text is in red. This means that either the addon does not exist in your openFrameworks/addons folder or it is not correctly linked.

This video tutorial explains how to include addons in your OF application
https://vimeo.com/34092591


##Make sure the ofMovieExporter libaries are linked correctly

In Xcode go into the project's target build settings. Scroll to "Search Paths" and open "Library Search Paths". Change the line "/Users/bimster/Sites/OF/of_v0.8.0_osx_release/addons/ofxMovieExporter/libs/libav/lib/osx" to point to the  library on your system.

##Make sure you clean the project before you compile

In Xcode go to Product -> Clean



=> the application should now compile and run succesfully