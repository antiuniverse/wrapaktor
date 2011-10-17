// http://www.native-instruments.com/forum/showthread.php?t=76540

May 21, 2010 - initial release


    TABLE OF CONTENTS
    -----------------
    a. Overview
    b. Installation for Reaktor
    c. Installation for Kore Player (and maybe grown-up Kore)



a. OVERVIEW
====================

"Wrapaktor" is a Reaktor wrapper DLL. It masquerades as Reaktor,
but intercepts the plug-in initialization, and ensures that the
VST host process priority stays the same.



b. INSTALLATION FOR REAKTOR
====================

For the regular Reaktor 5 VST libraries, installation involves
the following four files from your VstPlugins directory:

   \Reaktor5.dll
   \Reaktor5 FX.dll
   \Reaktor5 FX 2x8.dll
   \Reaktor5 Surround.dll

You may want to back these files up before proceeding.


For each of these files, the process to set up the wrapper is
as follows:

  1) Rename the original file -- changing just the extension,
     from ".dll" to ".priowrap_dll"

     e.g. "Reaktor5.dll" is renamed to "Reaktor5.priowrap_dll"


  2) Copy Wrapaktor.dll in place of the original DLL.

     e.g. Create a copy of "Wrapaktor.dll"
          Rename that "Copy of Wrapaktor.dll" to "Reaktor5.dll"


The wrapper DLL will automatically load the Reaktor DLL that
shares its file name, but with the ".priowrap_dll" extension.



c. INSTALLATION FOR KORE PLAYER (and maybe grown-up Kore)
====================

Installation for Kore Player (and presumably the full version
of Kore) is mostly the same as the installation for Reaktor
outlined above.

Instead of wrapping the DLLs contained in your VstPlugins
directory, you want to wrap the files located in Kore's own
"PlugIns" directory. In my case (Kore Player on 64-bit Windows),
the path to the Kore DLLs was:

  C:\Program Files (x86)\Common Files\Native Instruments\Kore Player\PlugIns