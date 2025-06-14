/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Gameboy_png;
    const int            Gameboy_pngSize = 1973770;

    extern const char*   DeEsser_Knob_svg;
    const int            DeEsser_Knob_svgSize = 2258;

    extern const char*   LowEq_Knob_svg;
    const int            LowEq_Knob_svgSize = 2258;

    extern const char*   Output_Knob_svg;
    const int            Output_Knob_svgSize = 2258;

    extern const char*   Satur_Knob_svg;
    const int            Satur_Knob_svgSize = 2258;

    extern const char*   ToneEq_Knob_svg;
    const int            ToneEq_Knob_svgSize = 2258;

    extern const char*   Comp_Knob_svg;
    const int            Comp_Knob_svgSize = 2258;

    extern const char*   EarlyGameBoy_ttf;
    const int            EarlyGameBoy_ttfSize = 10616;

    extern const char*   Hard_Comp_svg;
    const int            Hard_Comp_svgSize = 1374;

    extern const char*   Hard_Satur_svg;
    const int            Hard_Satur_svgSize = 1379;

    extern const char*   Soft_Comp_svg;
    const int            Soft_Comp_svgSize = 1377;

    extern const char*   Soft_Satur_svg;
    const int            Soft_Satur_svgSize = 1379;

    extern const char*   Start_Button_svg;
    const int            Start_Button_svgSize = 3403;

    extern const char*   Led_Rosso_svg;
    const int            Led_Rosso_svgSize = 1383;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 14;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
